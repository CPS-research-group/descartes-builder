#pragma once

#include <QFileInfo>
#include <QObject>
#include <QTemporaryDir>

#include "ui/models/uid_manager.hpp"
#include <QtNodes/Definitions>

namespace QtNodes {
class DagGraphicsScene;
class GraphicsView;
} // namespace QtNodes

class CustomGraph;

class TabComponents : public QObject
{
    Q_OBJECT
public:
    TabComponents(QWidget *parent, std::optional<QFileInfo> fileInfo = std::nullopt);
    ~TabComponents();
    CustomGraph *getGraph() const { return m_graph; }
    QtNodes::DagGraphicsScene *getScene() const { return m_scene; }
    QtNodes::GraphicsView *getView() const { return m_view; }
    std::unique_ptr<UIDManager> &getUIDManager() { return m_uidManager; }
    std::shared_ptr<QTemporaryDir> getTempDir() { return m_dir; }
    QDir getDataDir() { return m_dataDir; }
    QFileInfo getFileInfo() { return m_localFile; }
    void setFileInfo(const QFileInfo &fileInfo) { m_localFile = fileInfo; }
    bool save();
    bool saveAs();
    bool open();
    bool openExisting();
    bool isNewFile() const;
    bool isValidProjectName(const QString &name);
    QString getBasename() const;

private slots:
    void onDataSourceImportClicked(const QtNodes::NodeId nodeId);
    void postLoadProcess(const QJsonArray &nodesJsonArray);

private:
    CustomGraph *m_graph;
    QtNodes::DagGraphicsScene *m_scene;
    QtNodes::GraphicsView *m_view;
    std::shared_ptr<QTemporaryDir> m_dir;
    // dir with .dag and imported files
    QDir m_dataDir;
    // zipped local file to output to
    QFileInfo m_localFile;
    // UID Manager for this tab
    std::unique_ptr<UIDManager> m_uidManager;
};
