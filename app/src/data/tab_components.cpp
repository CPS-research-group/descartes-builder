#include "data/tab_components.hpp"
#include "data/tab_manager.hpp"
#include "ui/models/function_names.hpp"
#include <QDebug>
#include <QFileDialog>
#include <QJsonArray>
#include <QMessageBox>
#include <QStandardPaths>

#include <QtNodes/DagGraphicsScene>
#include <QtNodes/DirectedAcyclicGraphModel>
#include <QtNodes/GraphicsView>

#include <quazip/JlCompress.h>

#include "data/block_manager.hpp"
#include "data/custom_graph.hpp"
#include "ui/models/io_models.hpp"

using QtNodes::DagGraphicsScene;
using QtNodes::DirectedAcyclicGraphModel;
using QtNodes::GraphicsView;

namespace {
const QString SCENE_EXTENSION = ".dag";
const QString FILE_EXTENSION = "dcb";
} // namespace

TabComponents::TabComponents(QWidget *parent, std::optional<QFileInfo> fileInfo)
    : m_graph(new CustomGraph(BlockManager::getRegistry()))
    , m_scene(new DagGraphicsScene(*m_graph, parent))
    , m_view(new GraphicsView(m_scene))
    , m_dir(std::make_shared<QTemporaryDir>())
    , m_dataDir(m_dir->filePath("data"))
    , m_uidManager(std::make_unique<UIDManager>())
{
    // this is needed to keep the temp dir alive to avoid race condition during unit tests. This will eventually get deleted later by the os
    m_dir->setAutoRemove(false);
    m_graph->setParent(parent);
    if (!m_dir->isValid())
        qCritical() << "Temp dir failed to init";
    m_dataDir.mkpath(".");
    // Qt bug for MacOS throws warnings when using touch pad with graphics view
    // touch pad seems to trigger touch events, so touch events are disabled to supress the bug
    m_view->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, false);
    m_uidManager->setGraph(m_graph);
    m_scene->setDataDir(m_dataDir);

    connect(m_graph,
            &DirectedAcyclicGraphModel::graphLoadedFromFile,
            this,
            &TabComponents::postLoadProcess);
    connect(m_scene, &DagGraphicsScene::sceneLoaded, m_view, &GraphicsView::centerScene);
    if (parent)
        QObject::connect(m_scene, &DagGraphicsScene::modified, parent, [parent]() {
            parent->setWindowModified(true);
        });
    connect(m_graph,
            &CustomGraph::dataSourceModelImportClicked,
            this,
            &TabComponents::onDataSourceImportClicked);
    connect(m_graph,
            &CustomGraph::funcSourceModelImportClicked,
            this,
            &TabComponents::onFuncSourceImportClicked);
    if (fileInfo) {
        m_localFile = fileInfo.value();
    }
}

TabComponents::~TabComponents()
{
    m_view->deleteLater();
    m_scene->deleteLater();
    m_graph->deleteLater();
}

bool TabComponents::save()
{
    if (m_localFile.filePath().isEmpty() || m_localFile.suffix().isEmpty())
        return saveAs();

    // Check if the m_localFile directory is writable
    // qt's isWritable() does not work on Windows
    auto isQDirWritable = [this](QDir dir) -> bool {
        QFile testFile(dir.absoluteFilePath(".write_test"));
        if (testFile.open(QIODevice::WriteOnly)) {
            testFile.close();
            testFile.remove();
            return true;
        }
        return false;
    };
    if (!isQDirWritable(m_localFile.dir())) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            nullptr,
            "Save Error",
            "The file location is not writable. Do you want to save it to a different location?",
            QMessageBox::Yes | QMessageBox::Cancel);

        if (reply == QMessageBox::Yes) {
            if (!saveAs()) {
                return false;
            }
        } else
            return false;
    }

    QString sceneFilename = "scene" + SCENE_EXTENSION; // maintain 1 dag file per dcb
    QJsonObject metadata;                              // save metadata
    if (m_globals.m_randomState.has_value()) {
        metadata["random_state"] = *m_globals.m_randomState;
    }
    if (!m_scene->save(m_dataDir.absoluteFilePath(sceneFilename), metadata))
        return false;
    if (!JlCompress::compressDir(m_localFile.absoluteFilePath(), m_dataDir.absolutePath()))
        return false;
    qInfo() << "File saved to: " << m_localFile.absoluteFilePath();
    return true;
}

bool TabComponents::isValidProjectName(const QString &name)
{
    // must be at least 2 characters long and can contain letters, numbers, spaces, underscores, or hyphens
    static const QRegularExpression namePattern("^[\\w\\s-]{2,}$");
    return namePattern.match(name).hasMatch();
}

bool TabComponents::saveAs()
{
    QFileInfo newFile(
        QFileDialog::getSaveFileName(nullptr,
                                     tr("Save DCB File"),
                                     QStandardPaths::writableLocation(
                                         QStandardPaths::DocumentsLocation),
                                     tr("DesCartes Builder File (*%1)").arg(FILE_EXTENSION)));
    if (newFile.filePath().isEmpty())
        return false; // dialog cancelled
    if (newFile.suffix().compare(FILE_EXTENSION, Qt::CaseInsensitive) != 0)
        newFile.setFile(newFile.dir(), newFile.baseName() + '.' + FILE_EXTENSION);
    QString baseName = newFile.baseName();
    if (!isValidProjectName(baseName)) {
        QMessageBox::warning(nullptr,
                             tr("Invalid File Name"),
                             tr("The file name must be at least 2 characters long."));
        return false;
    }
    m_localFile.setFile(newFile.absoluteFilePath());
    return save();
}

bool TabComponents::open()
{
    m_localFile.setFile(
        QFileDialog::getOpenFileName(nullptr,
                                     tr("Open DCB File"),
                                     QStandardPaths::writableLocation(
                                         QStandardPaths::DocumentsLocation),
                                     tr("DesCartes Builder File (*%1)").arg(FILE_EXTENSION)));
    if (!m_localFile.exists() || m_localFile.suffix() != FILE_EXTENSION)
        return false; // dialog cancelled
    return openExisting();
}

bool TabComponents::openExisting()
{
    if (m_localFile.filePath().isEmpty()) {
        qWarning() << "File path is empty";
        return false;
    }

    JlCompress::extractDir(m_localFile.absoluteFilePath(), m_dataDir.absolutePath());
    QString sceneFilename = "scene" + SCENE_EXTENSION;
    if (!m_dataDir.exists(sceneFilename)) {
        qWarning() << "Scene file does not exist:" << sceneFilename;
        return false;
    }
    if (!m_scene->load(m_dataDir.absoluteFilePath(sceneFilename)))
        return false;
    loadMetadataFromExisting(sceneFilename); // Load additional metadata such as global variables
    return true;
}

void TabComponents::loadMetadataFromExisting(const QString &sceneFilename)
{
    // Load metadata
    QFile file(m_dataDir.absoluteFilePath(sceneFilename));
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray jsonData = file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        QJsonObject root = doc.object();
        if (root.contains("globals") && root["globals"].isObject()) {
            QJsonObject globals = root["globals"].toObject();
            if (globals.contains("random_state"))
                m_globals.m_randomState = globals["random_state"].toInt();
            else {
                // Ideally this case should not happen as random_state is always saved
                qWarning() << "random_state not found in globals, setting to 0";
                m_globals.m_randomState = 0;
            }
        } else {
            // For backward compatibility, set random state to 0 for older dcb without globals
            qInfo() << "Setting global random_state to 0";
            m_globals.m_randomState = 0;
        }
    }
}

void TabComponents::onDataSourceImportClicked(const QtNodes::NodeId nodeId)
{
    // Check if node has data imported already, if so, do not allow re-import
    auto dataSource = m_graph->delegateModel<DataSourceModel>(nodeId);
    if (!dataSource->file().fileName().isEmpty()) {
        QMessageBox::information(
            nullptr,
            tr("Import Not Allowed"),
            QString("Cannot re-import data into an already-initialized data_source. \n"
                    "File: %1")
                .arg(dataSource->file().fileName()));

        return;
    }
    QFileInfo originalFile(
        QFileDialog::getOpenFileName(nullptr,
                                     tr("Import Data Source"),
                                     QStandardPaths::writableLocation(
                                         QStandardPaths::DocumentsLocation),
                                     tr("data (*%1)").arg(DataSourceModel::fileFilter())));
    if (originalFile.filePath().isEmpty() || originalFile.suffix().isEmpty())
        return; // cancelled
    qDebug() << "copy to: " << m_dataDir.absoluteFilePath(originalFile.fileName());
    QFileInfo newFile(m_dataDir.absoluteFilePath(originalFile.fileName()));
    // move to temp dir
    QFile::copy(originalFile.absoluteFilePath(), newFile.absoluteFilePath());
    QFileInfo oldFile(m_dataDir.absoluteFilePath(dataSource->file().fileName()));
    if (oldFile.exists())
        QFile::remove(oldFile.absoluteFilePath());
    dataSource->setFile(newFile);
}

void TabComponents::onFuncSourceImportClicked(const QtNodes::NodeId nodeId)
{
    auto funcSource = m_graph->delegateModel<FuncSourceModel>(nodeId);
    if (!funcSource->file().fileName().isEmpty()) {
        QMessageBox::information(
            nullptr,
            tr("Import Not Allowed"),
            QString("Cannot re-import function into an already-initialized function source. \n"
                    "Function: %1")
                .arg(funcSource->file().fileName()));
        return;
    }
    QFileInfo originalFile(QFileDialog::getOpenFileName(nullptr,
                                                        tr("Import Function Source"),
                                                        QStandardPaths::writableLocation(
                                                            QStandardPaths::DocumentsLocation),
                                                        tr("Function Source (*.zip)")));
    if (originalFile.filePath().isEmpty() || originalFile.suffix().isEmpty())
        return;
    QString destFilePath = m_dataDir.absoluteFilePath(originalFile.fileName());

    // if a function with same name gets imported in the graph,
    // to correctly point in the catalog, adjust the file name
    if (QFile::exists(destFilePath)) {
        // inform the user that a file with the same name exists, so, renaming
        int counter = 1;
        while (QFile::exists(destFilePath)) {
            destFilePath = m_dataDir.absoluteFilePath(originalFile.baseName() + "_"
                                                      + QString::number(counter) + ".zip");
            counter++;
        }
        QMessageBox::information(nullptr,
                                 tr("Function already exists, Renaming"),
                                 tr("%1 already exists. Renaming the new function to %2")
                                     .arg(originalFile.fileName(),
                                          QFileInfo(destFilePath).fileName()));
    }
    qDebug() << "copy to:" << destFilePath;
    QFileInfo newFile(destFilePath);
    QFile::copy(originalFile.absoluteFilePath(), newFile.absoluteFilePath());
    funcSource->setFile(newFile);
}

void TabComponents::postLoadProcess(const QJsonArray &nodesJsonArray)
{
    // This function is called after the graph is loaded from a file. It reloads the type tags
    // and annotations for the output ports of the nodes based on the saved JSON data.
    if (nodesJsonArray.isEmpty()) {
        qWarning() << "No nodes found in the loaded graph.";
        return;
    }

    auto findById = [](const QJsonArray &array, int idToFind) -> QJsonObject {
        for (const QJsonValue &value : array) {
            if (value.isObject()) {
                QJsonObject obj = value.toObject();
                if (obj.contains("id") && obj["id"].toInt() == idToFind) {
                    return obj;
                }
            }
        }
        return QJsonObject();
    };

    for (const auto &id : m_graph->allNodeIds()) {
        QJsonObject nodeJson = findById(nodesJsonArray, id);
        if (nodeJson.isEmpty()) {
            qWarning() << "Node with ID" << id << "not found in the loaded graph.";
            continue;
        }
        auto block = m_graph->delegateModel<FdfBlockModel>(id);
        if (!block) {
            continue;
        }

        QJsonArray outputPorts = nodeJson["internal-data"].toObject()["output_ports"].toArray();
        for (const auto &outputPort : outputPorts) {
            QJsonObject portJson = outputPort.toObject();
            int index = portJson["index"].toInt();

            if (index >= 0 && static_cast<size_t>(index) < block->nPorts(PortType::Out)) {
                if (auto port = std::dynamic_pointer_cast<DataNode>(block->outData(index))) {
                    port->setAnnotation(portJson["annotation"].toString());
                    port->setTypeTagName(portJson["type_tag"].toString());
                    // Ensure output ports have unique captions after loading
                    Q_EMIT block->outPortCaptionUpdated(index, port->name());
                } else if (auto port = std::dynamic_pointer_cast<FunctionNode>(
                               block->outData(index))) {
                    port->setName(portJson["caption"].toString());
                    // Ensure output ports have unique captions after loading
                    Q_EMIT block->outPortCaptionUpdated(index, port->name());
                }
            }
        }
    }
}

bool TabComponents::isNewFile() const
{
    return m_localFile.filePath().isEmpty();
}

QString TabComponents::getBasename() const
{
    return m_localFile.baseName();
}
