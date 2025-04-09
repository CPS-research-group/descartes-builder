#include "data/tab_components.hpp"
#include "data/tab_manager.hpp"
#include <QDebug>
#include <QFileDialog>
#include <QStandardPaths>

#include <QtNodes/DagGraphicsScene>
#include <QtNodes/GraphicsView>

#include <quazip/JlCompress.h>

#include "data/block_manager.hpp"
#include "data/custom_graph.hpp"
#include "ui/models/io_models.hpp"

using QtNodes::DagGraphicsScene;
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
    m_graph->setParent(parent);
    if (!m_dir->isValid())
        qCritical() << "Temp dir failed to init";
    m_dataDir.mkpath(".");
    // Qt bug for MacOS throws warnings when using touch pad with graphics view
    // touch pad seems to trigger touch events, so touch events are disabled to supress the bug
    m_view->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, false);
    m_uidManager->setGraph(m_graph);
    connect(m_scene, &DagGraphicsScene::sceneLoaded, m_view, &GraphicsView::centerScene);
    if (parent)
        QObject::connect(m_scene, &DagGraphicsScene::modified, parent, [parent]() {
            parent->setWindowModified(true);
        });
    connect(m_graph,
            &CustomGraph::dataSourceModelImportClicked,
            this,
            &TabComponents::onDataSourceImportClicked);
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
    if (!m_scene->save(m_dataDir.absoluteFilePath(m_localFile.baseName() + SCENE_EXTENSION)))
        return false;
    if (!JlCompress::compressDir(m_localFile.absoluteFilePath(), m_dataDir.absolutePath()))
        return false;
    qInfo() << "File saved to: " << m_localFile.absoluteFilePath();
    return true;
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
    if (m_localFile.filePath().isEmpty())
        return false;
    JlCompress::extractDir(m_localFile.absoluteFilePath(), m_dataDir.absolutePath());
    if (!m_dataDir.exists(m_localFile.baseName() + SCENE_EXTENSION))
        return false;
    return m_scene->load(m_dataDir.absoluteFilePath(m_localFile.baseName() + SCENE_EXTENSION));
}

void TabComponents::onDataSourceImportClicked(const QtNodes::NodeId nodeId)
{
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
    auto dataSource = m_graph->delegateModel<DataSourceModel>(nodeId);
    QFileInfo oldFile(m_dataDir.absoluteFilePath(dataSource->file().fileName()));
    if (oldFile.exists())
        QFile::remove(oldFile.absoluteFilePath());
    dataSource->setFile(newFile);
}