#pragma once

#include <QMainWindow>

class GraphicsSceneTabWidget;
class AbstractEngine;
class TabManager;
class BlockManager;
class QAction;
class Temp;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    enum class SideBarAction { Blocks, Charts, Settings, Information };

    MainWindow();
    ~MainWindow();
    bool openDCB(const QString &fileName);
    bool executeDCB();
    std::shared_ptr<TabManager> getTabManager() const { return m_tabManager; }

public slots:
    void gridToggled(bool enabled);

private slots:
    bool callExecute();
    void onBlockSelected(const uint &id);
    void onBlockUpdated(const uint &id);

signals:
    void scoreParams(const QString &scoreParams);

private:
    void initManagers();
    // Ui inits
    void initScene();
    void initMenuBar();
    void initPrimarySideBar();
    void initLogPanel();
    void scoreParameters(const QString &scorePath);

    void enableChartAction(bool state);

    // UI
    GraphicsSceneTabWidget *m_graphicsSceneTabWidget;
    QWidget *m_centralWidget;
    std::unordered_map<SideBarAction, QAction *> m_sidebarActions;

    // Logic
    std::unique_ptr<AbstractEngine> m_engine;
    std::shared_ptr<TabManager> m_tabManager;
    std::shared_ptr<BlockManager> m_blockManager;
    Temp *m_temp; // class used for testing temporary code
};
