#include "engine/kedro.hpp"

#include <QApplication>
#include <QProcess>
#include <QStandardPaths>

#include <QtNodes/DirectedAcyclicGraphModel>

#include <QtUtility/file/file.hpp>

#include <quazip/JlCompress.h>

#include "data/constants.hpp"
#include "data/custom_graph.hpp"
#include "data/settings.hpp"
#include "data/tab_components.hpp"
#include "ui/models/fdf_block_model.hpp"
#include "ui/models/io_models.hpp"
#include "ui/models/processor_models.hpp"

#include "engine/kedro.hpp"
#include <iostream>

#ifdef Q_OS_WIN
#define IS_WINDOWS true
#else
#define IS_WINDOWS false
#endif

using Settings = data::Settings;

namespace {

using FdfType = FdfBlockModel::FdfType;
const std::unordered_set<FdfType> EXCLUDED_TYPES = {FdfType::Data, FdfType::Output};

QString singleQuote(const QString &string)
{
    return '\'' + string + '\'';
}

QString quote(const QString &string)
{
    return '\"' + string + '\"';
}

QStringList getPortList(const FdfBlockModel &block, const PortType &type)
{
    QStringList result;
    for (PortIndex i = 0; i < block.nPorts(type); ++i) {
        // if an in port is not connected it is null, the 'if' can be removed after the validity check handles it
        if (auto port = block.portData(type, i))
            result.append(quote(port->type().name.replace(' ', '_')));
    }
    return result;
}

QDir getKedroUmbrellaDir()
{ // Run a Python command to find kedro-umbrella's location
    QProcess process;
    process.start("python",
                  QStringList() << "-c"
                                << "import kedro_umbrella, os; "
                                   "print(os.path.dirname(kedro_umbrella.__file__))");
    process.waitForFinished();

    QString kedroUmbrellaPath = process.readAllStandardOutput().trimmed();
    if (kedroUmbrellaPath.isEmpty()) {
        qCritical() << "Failed to locate kedro-umbrella package";
        return QDir();
    }
    qInfo() << "Using kedro_umbrella path :" << kedroUmbrellaPath;
    return QDir(kedroUmbrellaPath);
}

QString toString(const FdfBlockModel &block)
{
    QString result = block.typeAsString() + '(';
    if (!block.functionName().isEmpty())
        result += QString("func=%1,").arg(block.functionName());
    result += QString("name=%1").arg(quote(block.caption().replace(' ', '_')));
    QStringList inputs = getPortList(block, PortType::In);
    if (block.hasParameters())
        inputs << quote(QString("params:%1").arg(block.caption().replace(' ', '_')));
    if (inputs.size() == 1)
        result += QString(",inputs=%1").arg(inputs.at(0));
    else if (inputs.size() > 1)
        result += QString(",inputs=[%1]").arg(inputs.join(','));
    QStringList outputs = getPortList(block, PortType::Out);
    if (outputs.size() == 1)
        result += QString(",outputs=%1").arg(outputs.at(0));
    else if (outputs.size() > 1)
        result += QString(",outputs=[%1]").arg(outputs.join(','));
    result += ')';
    return result;
}

int timeoutMinutes()
{
    return Settings::instance().value("engine timeout (minutes)").toInt();
}

} // namespace

Kedro::Kedro()
    : m_WINDOWS(IS_WINDOWS)
    , m_setup(false)
    , m_KEDRO_UMBRELLA_DIR(getKedroUmbrellaDir())
    , m_execution(std::make_unique<ExecutionBundle>())
    , m_DEFAULT_TEMPLATE(m_KEDRO_UMBRELLA_DIR.absoluteFilePath("template/builder-spring/"))
{
    if (!m_runtimeCache.isValid())
        qCritical() << "Temporary dir failed to setup";
    // init execution process
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("COLUMNS", "200");
    env.insert("LINES", "25");
    m_execution->process.setProcessEnvironment(env); // this is for kedro logger to print better
#if IS_WINDOWS
    m_execution->process.setProgram("python");
    m_execution->process.setArguments({"-m", "kedro", "run"});
#else
    m_execution->process.setProgram("kedro");
    m_execution->process.setArguments({"run"});
#endif
    m_execution->timer.setSingleShot(true);

    verifySetup();

    connect(&m_execution->process, &QProcess::finished, this, &Kedro::onExecutionFinished);
    connect(&m_execution->timer, &QTimer::timeout, this, &Kedro::onTimeOut);
}

Kedro::~Kedro()
{
    disconnect(&m_execution->process, &QProcess::finished, this, &Kedro::onExecutionFinished);
}

bool Kedro::execute(std::shared_ptr<TabComponents> tab)
{
    if (m_execution->inProgress) {
        qInfo() << "There is already an execution in progress, please wait.";
        return false;
    }
    emit started();
    m_execution->inProgress = true;
    m_execution->timer.start(timeoutMinutes() * constants::MINUTE_MSECS);
    // lambda func to simplify returning false and releasing in progress flag
    auto falseAndRelease = [this]() -> bool {
        m_execution->inProgress = false;
        m_execution->timer.stop();
        emit finished(false);
        return false;
    };

    qDebug() << "Kedro is executing...";
    if (!validityCheck(tab))
        return falseAndRelease();
    if (!m_setup) {
        qCritical() << "Kedro is not setup yet, please setup kedro before executing";
        return falseAndRelease();
    }
    m_execution->tab = tab;
    m_execution->project = initWorkspace(tab);
    if (!generateParametersYml(m_execution->project, tab->getGraph()))
        return falseAndRelease();
    if (!generateCatalogYml(m_execution->project, tab))
        return falseAndRelease();
    if (!generatePipelinePy(m_execution->project, tab->getGraph()))
        return falseAndRelease();

    // call kedro run
    m_execution->process.setWorkingDirectory(m_execution->project.absolutePath());
    m_execution->process.start();
    return true;
}

bool Kedro::validityCheck(std::shared_ptr<TabComponents> tab)
{
    auto graph = tab->getGraph();
    qInfo() << "Checking graph validity...";
    if (graph->isEmpty()) {
        qWarning() << "There is no blocks in the graph to execute";
        return false;
    }
    if (!graph->isConnected()) {
        qWarning() << "The blocks in the graph are not connected";
        return false;
    }
    // TODO: add check that every node input is connected
    // TODO: add check that every node is "ready" e.g. data source has imported something
    qInfo() << "Passed validity checks!";
    return true;
}

QDir Kedro::initWorkspace(std::shared_ptr<TabComponents> tab)
{
    auto name = tab->getFileInfo().baseName();
    // kedro dir inside of temp dir, to avoid cases where file name conflicts with existing folder
    QDir kedroDir = ensureDirExists(tab->getTempDir()->filePath("kedro"));
    QDir workspaceDir = QDir(kedroDir.absolutePath() + QDir::separator() + name);
    if (workspaceDir.exists()) {
        qInfo() << "Workspace already exists: " << workspaceDir.absolutePath();
        return workspaceDir;
    }

    qInfo() << "Creating workspace " << workspaceDir.absolutePath();
    QProcess workspaceProcess;
    workspaceProcess.setWorkingDirectory(kedroDir.absolutePath());
    QString command;
#if IS_WINDOWS
    command = QString("cmd.exe /C echo %1 | python -m kedro new -s %2").arg(name, m_DEFAULT_TEMPLATE);
#else
    command = QString("bash -c \"echo %1 | kedro new -s %2\"")
                  .arg(singleQuote(name), singleQuote(m_DEFAULT_TEMPLATE));
#endif
    workspaceProcess.startCommand(command);
    if (!workspaceProcess.waitForFinished()) {
        qCritical() << "Failed to create workspace " << name;
        return QDir();
    }
    if (workspaceProcess.exitStatus() != QProcess::NormalExit || workspaceProcess.exitCode() != 0) {
        qCritical() << "Workspace creation command failed with exit code "
                    << workspaceProcess.exitCode();
        qCritical() << "Command output:\n" << workspaceProcess.readAllStandardOutput();
        qCritical() << "Command error output:\n" << workspaceProcess.readAllStandardError();
        return QDir();
    }
    return workspaceDir;
}

void Kedro::onExecutionFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (!m_execution->timer.isActive()) {
        qDebug() << "Execution finished after timeout (minutes): " << timeoutMinutes();
        return;
    }
    m_execution->timer.stop();
    if (exitStatus == QProcess::ExitStatus::CrashExit) {
        qCritical() << "Failed to run kedro";
    } else if (exitCode != 0) {
        qCritical() << "Kedro process finished with exit code: " << exitCode;
    }

    auto output = QString::fromUtf8(m_execution->process.readAllStandardOutput());
    auto errorOutput = m_execution->process.readAllStandardError().trimmed();
    if (!errorOutput.isEmpty())
        output += "\nERROR LOG:\n" + QString::fromUtf8(errorOutput);

    postExecutionProcess();

    // compress dir to zip and cache to runtime dir
    auto zip = QtUtility::file::getUniqueFile(
        QFileInfo(m_runtimeCache.filePath(m_execution->tab->getFileInfo().baseName() + ".zip")));
    if (JlCompress::compressDir(zip.absoluteFilePath(), m_execution->project.absolutePath()))
        qDebug() << "Kedro executed, result is cached to: " << zip.absoluteFilePath();
    emit executed(output);
    releaseExecution();
    emit finished(true);
}

void Kedro::onTimeOut()
{
    releaseExecution();
    qInfo() << "Kedro execution timed out, exceeded limit (minutes): " << timeoutMinutes();
    emit finished(false);
}

QString Kedro::serializeNode(const QtNodes::NodeId &id, CustomGraph *graph) const
{
    return toString(*graph->delegateModel<FdfBlockModel>(id));
}

void Kedro::verifySetup()
{
    m_setup = true;
    qInfo() << "Kedro is ready to execute!";
}

bool Kedro::generateParametersYml(const QDir &kedroProject, CustomGraph *graph)
{
    QStringList parameters;
    for (const auto &id : graph->allNodeIds())
        if (auto block = graph->delegateModel<FdfBlockModel>(id)) {
            if (!block->hasParameters())
                continue;
            parameters << block->caption().replace(' ', '_') + ':';
            for (auto &pair : block->getParameters())
                parameters << QString("  %1: %2").arg(pair.first, pair.second);
        }
    QDir conf = ensureDirExists(kedroProject.absoluteFilePath(constants::kedro::CONF_PATH));
    //generate parameters.yml
    QFile parametersYml(conf.absoluteFilePath("parameters.yml"));
    if (!parametersYml.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Cannot open parameters.yml:" << parametersYml.errorString();
        return false;
    }
    QTextStream out(&parametersYml);
    out << parameters.join("\n");
    parametersYml.close();
    return true;
}

bool Kedro::generateCatalogYml(const QDir &kedroProject, std::shared_ptr<TabComponents> tab)
{
    QDir conf = ensureDirExists(kedroProject.absoluteFilePath(constants::kedro::CONF_PATH));
    auto dataSources = tab->getGraph()->getDataSourceModels();
    QDir rawDataDir = ensureDirExists(
        kedroProject.absoluteFilePath(constants::kedro::RAW_DATA_PATH));
    QStringList catalogEntries;
    for (auto data : dataSources) {
        auto fileName = data->file().fileName();
        // copy data to raw data dir
        QFile::copy(tab->getDataDir().absoluteFilePath(fileName),
                    rawDataDir.absoluteFilePath(fileName));
        // add external data to catalog.yml
        catalogEntries << constants::kedro::CATALOG_YML_ENTRY.arg(data->file().baseName(),
                                                                  data->fileTypeString(),
                                                                  constants::kedro::RAW_DATA_PATH
                                                                      + fileName);
    }
    // add outputs to catalog.yml
    auto funcOuts = tab->getGraph()->getFuncOutModels();
    for (auto funcOut : funcOuts) {
        auto name = funcOut->getFileName().replace(' ', '_');
        catalogEntries << constants::kedro::CATALOG_YML_ENTRY
                              .arg(name,
                                   funcOut->fileTypeString(),
                                   constants::kedro::MODELS_PATH + name + '.'
                                       + funcOut->getFileExtenstion());
    }
    //generate catalog.yml
    QFile catalogYml(conf.absoluteFilePath("catalog.yml"));
    if (!catalogYml.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Cannot open catalog.yml:" << catalogYml.errorString();
        return false;
    }
    QTextStream out(&catalogYml);
    out << catalogEntries.join("\n");
    catalogYml.close();
    return true;
}

bool Kedro::generatePipelinePy(const QDir &kedroProject, CustomGraph *graph)
{
    // for some reason dir name char '-' will convert to '_'
    QDir source = ensureDirExists(kedroProject.absoluteFilePath(
        QString(constants::kedro::SOURCE_PATH).arg(kedroProject.dirName().replace('-', '_'))));
    QStringList serializedObjects;
    for (const auto &id : graph->topologicalOrder())
        if (auto block = graph->delegateModel<FdfBlockModel>(id))
            if (EXCLUDED_TYPES.count(block->type()) < 1)
                serializedObjects.append(serializeNode(id, graph));
    QString data = constants::kedro::PIPELINE_PY.arg(serializedObjects.join(",\n"));
    QFile pipelinePy(source.absoluteFilePath("pipeline.py"));
    if (!pipelinePy.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Cannot open pipeline.py:" << pipelinePy.errorString();
        return false;
    }
    QTextStream out(&pipelinePy);
    out << data;
    pipelinePy.close();
    return true;
}

QDir Kedro::ensureDirExists(const QString &path)
{
    // check that the dire exists. This check is added because of the behaviour in windows for temp dirs.

    QDir dir(path);
    if (!dir.exists() && !dir.mkpath(".")) {
        qCritical() << "Failed to create directory:" << dir.absolutePath();
    }
    return dir;
}

void Kedro::postExecutionProcess()
{
    auto graph = m_execution->tab->getGraph();
    for (auto &id : graph->allNodeIds()) {
        postScoreModel(graph, id);
        postSensitivityAnalysisModel(graph, id);
    }
}

void Kedro::postScoreModel(CustomGraph *graph, const QtNodes::NodeId &id)
{
    auto score = graph->delegateModel<ScoreModel>(id);
    if (!score)
        return;

    QDir reportDir(m_execution->project.absoluteFilePath(constants::kedro::REPORTING_PATH)
                   + score->caption().replace(' ', '_') // XXX /!\ needs to be in the block directly
    );

    // save the graphs
    auto graphs = reportDir.entryList({"*.png"}, QDir::Files);
    for (auto &graph : graphs)
        graph = reportDir.absoluteFilePath(graph);
    score->setExecutedGraphs(graphs);

    // save the score
    if (reportDir.exists("score.yml")) {
        // parse score.yml
        // XXX HACKY PARSER
        std::unordered_map<QString, QString> map;
        QFile yml(reportDir.absoluteFilePath("score.yml"));
        if (!yml.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Cannot open score.yml";
            return;
        }
        for (auto &line : yml.readAll().split('\n')) {
            auto pair = line.split(':');
            if (pair.size() != 2)
                continue;
            map[pair.front().trimmed()] = pair.back().trimmed();
        }
        yml.close();
        score->setExecutedValues(map);
    }
}

void Kedro::postSensitivityAnalysisModel(CustomGraph *graph, const QtNodes::NodeId &id)
{
    auto block = graph->delegateModel<SensitivityAnalysisModel>(id);
    if (!block)
        return;

    QDir reportDir(m_execution->project.absoluteFilePath(constants::kedro::REPORTING_PATH)
                   + block->caption().replace(' ', '_') // XXX /!\ needs to be in the block directly
    );
    // save the graphs
    auto graphs = reportDir.entryList({"*.png"}, QDir::Files);
    for (auto &graph : graphs)
        graph = reportDir.absoluteFilePath(graph);
    block->setExecutedGraphs(graphs);
}

void Kedro::releaseExecution()
{
    m_execution->tab.reset();
    m_execution->inProgress = false;
}
