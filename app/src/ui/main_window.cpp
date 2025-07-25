#include "ui/main_window.hpp"

#include <QActionGroup>
#include <QApplication>
#include <QDir>
#include <QDockWidget>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QScreen>
#include <QToolBar>
#include <QVBoxLayout>

#include <QtNodes/ConnectionStyle>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/GraphicsView>
#include <QtNodes/GraphicsViewStyle>
#include <QtNodes/NodeData>
#include <QtNodes/NodeStyle>

using QtNodes::ConnectionStyle;
using QtNodes::GraphicsViewStyle;
using QtNodes::NodeStyle;

#include <QtUtility/media/media.hpp>

#include "data/block_manager.hpp"
#include "data/constants.hpp"
#include "data/tab_components.hpp"
#include "data/tab_manager.hpp"
#include "engine/engine_starter.hpp"
#include "temp.hpp"
#include "ui/bottom_panel.hpp"
#include "ui/graphics_scene_tab_widget.hpp"
#include "ui/models/fdf_block_model.hpp"
#include "ui/side_bar_widgets/blocks.hpp"
#include "ui/side_bar_widgets/charts.hpp"
#include "ui/side_bar_widgets/information.hpp"
#include "ui/side_bar_widgets/settings.hpp"

namespace {
QtNodes::NodeId selectedId = QtNodes::InvalidNodeId;
}

MainWindow::MainWindow()
    : m_engine(EngineStarter::init())
    , m_tabManager(std::shared_ptr<TabManager>(&TabManager::instance(), [](TabManager *) {}))
    , m_blockManager(std::make_shared<BlockManager>())
    , m_temp(new Temp(this))
{
    initManagers();
    initScene();
    initMenuBar();
    initPrimarySideBar();
    initLogPanel();

    setWindowTitle("DesCartes Builder");
    if (qEnvironmentVariableIsEmpty("TEST_MODE")) {
        setGeometry(QApplication::primaryScreen()->availableGeometry());
        showMaximized();
    }
    qInfo() << "Welcome to DesCartes Builder";
}

MainWindow::~MainWindow()
{
    // this incoming connection will cause a qt fatal message since it tries to run a slot on an partially destroyed object
    disconnect(m_blockManager.get(),
               &BlockManager::nodeSelected,
               this,
               &MainWindow::onBlockSelected);
    m_tabManager->clear();
    qInfo() << "Program has finished.";
}

bool MainWindow::openDCB(const QString &filePath)
{
    return m_tabManager->openFrom(filePath);
}

bool MainWindow::executeDCB()
{
    return callExecute();
}

bool MainWindow::callExecute()
{
    auto currentTab = m_tabManager->getCurrentTab();
    if (!currentTab) {
        qWarning() << "No tab to execute";
        return false;
    }
    if (!validateTab(currentTab)) {
        return false;
    }
    return m_engine->execute(currentTab);
}

bool MainWindow::validateTab(std::shared_ptr<TabComponents> &tab)
{
    if (!tab) {
        qWarning() << "No tab to verify";
        return false;
    }
    // Save silently if tab has a file associated with already
    if (!tab->isNewFile()) {
        if (!m_tabManager->save()) {
            qWarning() << "Save failed";
            return false;
        }
    } else {
        QMessageBox::StandardButton reply
            = QMessageBox::question(this,
                                    "Save Before Run",
                                    "Pipeline has to be saved for run. Do you want to save it now?",
                                    QMessageBox::Yes | QMessageBox::Cancel);

        if (reply == QMessageBox::Yes) {
            if (!m_tabManager->saveAs()) {
                return false;
            }
        } else
            return false;
    }
    return true;
}

void MainWindow::onBlockSelected(const uint &id)
{
    selectedId = id;
    auto block = m_blockManager->getBlock(id);
    enableChartAction(
        block && (!block->getExecutedGraphs().isEmpty() || !block->getExecutedValues().empty()));
}

void MainWindow::onBlockUpdated(const uint &id)
{
    if (id == selectedId)
        onBlockSelected(id);
}

void MainWindow::initManagers()
{
    m_blockManager->setTabManager(m_tabManager);
    connect(m_blockManager.get(), &BlockManager::nodeSelected, this, &MainWindow::onBlockSelected);
    connect(m_blockManager.get(), &BlockManager::nodeUpdated, this, &MainWindow::onBlockUpdated);
}

void MainWindow::initScene()
{
    ConnectionStyle::setConnectionStyle(constants::CONNECTION_STYLE);
    GraphicsViewStyle::setStyle(constants::GRAPHICS_VIEW_STYLE_GRID);
    NodeStyle::setNodeStyle(constants::NODE_STYLE);

    m_centralWidget = new QWidget();
    setCentralWidget(m_centralWidget);
    m_tabManager->setTabParent(m_centralWidget);

    QVBoxLayout *layout = new QVBoxLayout(m_centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_graphicsSceneTabWidget = new GraphicsSceneTabWidget(m_tabManager, m_centralWidget);
    connect(m_graphicsSceneTabWidget,
            &GraphicsSceneTabWidget::runClicked,
            this,
            &MainWindow::callExecute);
    connect(m_engine.get(),
            &AbstractEngine::started,
            m_graphicsSceneTabWidget,
            &GraphicsSceneTabWidget::runStarted);
    connect(m_engine.get(),
            &AbstractEngine::finished,
            m_graphicsSceneTabWidget,
            &GraphicsSceneTabWidget::runFinished);
    connect(m_engine.get(), &AbstractEngine::scoreYmlCreated, this, &MainWindow::scoreParameters);

    layout->addWidget(m_graphicsSceneTabWidget);
}

void MainWindow::scoreParameters(const QString &scoreParameters)
{
    // used for testing
    emit scoreParams(scoreParameters);
}

void MainWindow::initMenuBar()
{
    auto menuBar = new QMenuBar();
    setMenuBar(menuBar);

    QMenu *fileMenu = menuBar->addMenu("File");
    auto newAction = fileMenu->addAction("New");
    auto saveAction = fileMenu->addAction("Save");
    auto saveAsAction = fileMenu->addAction("Save As...");
    auto openAction = fileMenu->addAction("Open");
    fileMenu->addSeparator();
    auto closeAction = fileMenu->addAction("Close current tab");
    auto nextTabAction = fileMenu->addAction("Next tab");
    auto previousTabAction = fileMenu->addAction("Previous tab");
    closeAction->setDisabled(true);
    nextTabAction->setDisabled(true);
    previousTabAction->setDisabled(true);
    fileMenu->addSeparator();
    auto runAction = fileMenu->addAction("Run");

    newAction->setShortcuts({QKeySequence::New, QKeySequence::AddTab});
    saveAction->setShortcut(QKeySequence::Save);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    openAction->setShortcut(QKeySequence::Open);
    closeAction->setShortcut(QKeySequence::Close);
    nextTabAction->setShortcut(QKeyCombination(Qt::MetaModifier, Qt::Key_Tab));
    previousTabAction->setShortcut(
        QKeyCombination(Qt::MetaModifier | Qt::ShiftModifier, Qt::Key_Tab));
    runAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));

    connect(newAction, &QAction::triggered, m_tabManager.get(), &TabManager::newTab);
    connect(saveAction, &QAction::triggered, m_tabManager.get(), &TabManager::save);
    connect(saveAsAction, &QAction::triggered, m_tabManager.get(), &TabManager::saveAs);
    connect(openAction, &QAction::triggered, m_tabManager.get(), &TabManager::open);
    connect(closeAction,
            &QAction::triggered,
            m_graphicsSceneTabWidget,
            &GraphicsSceneTabWidget::closeCurrentTab);
    connect(nextTabAction,
            &QAction::triggered,
            m_graphicsSceneTabWidget,
            &GraphicsSceneTabWidget::nextTab);
    connect(previousTabAction,
            &QAction::triggered,
            m_graphicsSceneTabWidget,
            &GraphicsSceneTabWidget::previousTab);
    connect(m_graphicsSceneTabWidget,
            &GraphicsSceneTabWidget::countChanged,
            fileMenu,
            [closeAction, nextTabAction, previousTabAction](int count) {
                const bool MORE_THAN_ONE = count > 1;
                closeAction->setEnabled(MORE_THAN_ONE);
                nextTabAction->setEnabled(MORE_THAN_ONE);
                previousTabAction->setEnabled(MORE_THAN_ONE);
            });
    connect(runAction, &QAction::triggered, this, &MainWindow::callExecute);

#ifdef DEBUG
    { // temp menu for testing code
        QMenu *tempMenu = menuBar->addMenu("Temp");
        auto infoAction = tempMenu->addAction("print info");
        auto debugAction = tempMenu->addAction("print debug");
        auto errorAction = tempMenu->addAction("print error");
        auto printSettingsAction = tempMenu->addAction("print settings");
        auto openPipe = tempMenu->addAction("Open Pipe deformation");
        auto openMagnet = tempMenu->addAction("Open Magnetic bearing");
        connect(infoAction, &QAction::triggered, m_temp, &Temp::printInfo);
        connect(debugAction, &QAction::triggered, m_temp, &Temp::printDebug);
        connect(errorAction, &QAction::triggered, m_temp, &Temp::printError);
        connect(printSettingsAction, &QAction::triggered, m_temp, &Temp::printAllSettings);
        connect(openPipe, &QAction::triggered, m_tabManager.get(), [this]() {
            QDir dir(QApplication::applicationDirPath());
            for (int i = 0; i < 2; ++i) // linux and windows
                dir.cdUp();
            dir.cd("examples");
            m_tabManager->openFrom(dir.absoluteFilePath("pipe-deformation-test.dcb"));
        });
        openPipe->setShortcut(QKeyCombination(Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_E));
        connect(openMagnet, &QAction::triggered, m_tabManager.get(), [this]() {
            QDir dir(QApplication::applicationDirPath());
            for (int i = 0; i < 2; ++i) // mac only due to bundle dir
                dir.cdUp();
            dir.cd("examples");
            m_tabManager->openFrom(dir.absoluteFilePath("magnetic-bearing.dcb"));
        });
        openMagnet->setShortcut(QKeyCombination(Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_M));
    }
#endif
}

void MainWindow::initPrimarySideBar()
{
    // prevent log panel from taking the corner
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    struct SideBarWidgetData
    {
        SideBarAction action;
        QIcon icon;
        QString title;
        QWidget *widget;
    };
    std::vector<SideBarWidgetData> widgets = {
        {SideBarAction::Blocks,
         QtUtility::media::recolor(QIcon(":/blocks.png"), constants::COLOR_SECONDARY),
         "Box",
         new Blocks(m_blockManager, m_tabManager)},
        {SideBarAction::Charts,
         QtUtility::media::recolor(QIcon(":/charts.png"), constants::COLOR_SECONDARY),
         "Charts",
         new Charts(m_blockManager)},
        {SideBarAction::Settings,
         QtUtility::media::recolor(QIcon(":/settings.png"), constants::COLOR_SECONDARY),
         "Settings",
         new Settings(this)},
        {SideBarAction::Information,
         QtUtility::media::recolor(QIcon(":/information.png"), constants::COLOR_SECONDARY),
         "Information",
         new Information},
    };

    auto toolBar = new QToolBar("Primary Side Bar");
    toolBar->setMovable(false);
    addToolBar(Qt::LeftToolBarArea, toolBar);

    auto primarySideBarGroup = new QActionGroup(this);
    primarySideBarGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);

    for (auto widgetData : widgets) {
        auto action = toolBar->addAction(widgetData.icon, widgetData.title);
        action->setCheckable(true);
        m_sidebarActions[widgetData.action] = action;
        primarySideBarGroup->addAction(action);
        auto dockWidget = new QDockWidget(widgetData.title);
        dockWidget->setMinimumWidth(constants::SIDE_BAR_MINIMUM_WIDTH);
        dockWidget->setTitleBarWidget(new QLabel(widgetData.title));
        dockWidget->setWidget(widgetData.widget);
        dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
        dockWidget->hide();

        connect(action, &QAction::toggled, dockWidget, &QDockWidget::setVisible);
    }
    // disable by default, only enable when the selected block has chart/output data
    enableChartAction(false);
    if (auto firstAction = toolBar->actions().first())
        firstAction->trigger();
}

void MainWindow::initLogPanel()
{
    auto bottomPanel = new BottomPanel();
    addDockWidget(Qt::BottomDockWidgetArea, bottomPanel);
    connect(m_engine.get(), &AbstractEngine::executed, bottomPanel, &BottomPanel::appendOutputPanel);
}

void MainWindow::enableChartAction(bool state)
{
    if (m_sidebarActions.count(SideBarAction::Charts) < 1)
        return;
    auto action = m_sidebarActions.at(SideBarAction::Charts);
    action->setEnabled(state);
    QString tooltip = state ? "Charts" : "Select a block with chart/output data to view charts";
    action->setToolTip(tooltip);
    if (!state && action->isChecked()) // if currently at charts and we disable it, switch to blocks
        m_sidebarActions.at(SideBarAction::Blocks)->trigger();
}

void MainWindow::gridToggled(bool enabled)
{
    if (auto *view = m_tabManager->currentView()) {
        if (enabled) {
            GraphicsViewStyle::setStyle(constants::GRAPHICS_VIEW_STYLE_GRID);
        } else {
            GraphicsViewStyle::setStyle(constants::GRAPHICS_VIEW_STYLE_PLAIN);
        }
        if (auto *scene = qobject_cast<QtNodes::BasicGraphicsScene *>(view->scene())) {
            QRect visibleRect = view->viewport()->rect();
            QRectF visibleSceneRect = view->mapToScene(visibleRect).boundingRect();
            scene->invalidate(visibleSceneRect, QGraphicsScene::BackgroundLayer);
            view->viewport()->update();
        }
    }
}
