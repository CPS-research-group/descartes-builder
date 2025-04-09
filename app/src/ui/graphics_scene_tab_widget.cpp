#include "ui/graphics_scene_tab_widget.hpp"

#include <QPushButton>
#include <QTabBar>
#include <QWidget>

#include "data/tab_manager.hpp"

GraphicsSceneTabWidget::GraphicsSceneTabWidget(std::shared_ptr<TabManager> tabManager,
                                               QWidget *parent)
    : QTabWidget(parent)
    , m_tabManager(tabManager)
    , m_runButton(new QPushButton("Run"))
{
    tabBar()->setExpanding(false);
    setCornerWidget(m_runButton);
    connect(m_runButton, &QPushButton::clicked, this, &GraphicsSceneTabWidget::runClicked);

    connect(this,
            &GraphicsSceneTabWidget::tabCloseRequested,
            this,
            &GraphicsSceneTabWidget::closeTab);
    connect(this,
            &GraphicsSceneTabWidget::countChanged,
            this,
            &GraphicsSceneTabWidget::onTabCountChanged);
    connect(this,
            &GraphicsSceneTabWidget::currentChanged,
            this,
            &GraphicsSceneTabWidget::onCurrentChanged);
    connect(m_tabManager.get(),
            &TabManager::tabCreated,
            this,
            &GraphicsSceneTabWidget::onTabCreated);
    connect(m_tabManager.get(),
            &TabManager::currentChanged,
            this,
            &GraphicsSceneTabWidget::setCurrentWidget);
    connect(m_tabManager.get(),
            &TabManager::tabFileNameChanged,
            this,
            &GraphicsSceneTabWidget::onTabFileNameChanged);

    // init with 1 blank tab
    m_tabManager->newTab();
}

void GraphicsSceneTabWidget::closeCurrentTab()
{
    closeTab(currentIndex());
}

void GraphicsSceneTabWidget::closeTab(int index)
{
    if (index < 0 || index >= count())
        return;

    auto targetWidget = widget(index);
    removeTab(index);

    m_tabManager->removeTab(targetWidget);
}

void GraphicsSceneTabWidget::nextTab()
{
    if (count() < 2)
        return;
    int nextIndex = (currentIndex() + 1) % count();
    setCurrentIndex(nextIndex);
}

void GraphicsSceneTabWidget::previousTab()
{
    if (count() < 2)
        return;
    int nextIndex = (currentIndex() - 1 + count()) % count();
    setCurrentIndex(nextIndex);
}

void GraphicsSceneTabWidget::setRunState(bool state)
{
    m_runButton->setEnabled(!state);
    m_runButton->setText(state ? "Running" : "Run");
}

void GraphicsSceneTabWidget::runStarted()
{
    setRunState(true);
}

void GraphicsSceneTabWidget::runFinished()
{
    setRunState(false);
}

void GraphicsSceneTabWidget::onTabCountChanged(int count)
{
    setTabsClosable(count > 1);
}

void GraphicsSceneTabWidget::onCurrentChanged(const int &index)
{
    auto view = widget(index);
    if (m_tabManager->currentWidget() == view)
        return;
    m_tabManager->setCurrentView(view);
}

void GraphicsSceneTabWidget::onTabCreated(QWidget *widget)
{
    auto fileName = m_tabManager->getFileInfo(widget).baseName();
    int index = addTab(widget, fileName.isEmpty() ? "blank" : fileName);
    setCurrentIndex(index);
}

void GraphicsSceneTabWidget::onTabFileNameChanged(QWidget *widget, const QString &fileName)
{
    setTabText(indexOf(widget), fileName);
}

void GraphicsSceneTabWidget::tabInserted(int index)
{
    QTabWidget::tabInserted(index);
    emit countChanged(count());
}

void GraphicsSceneTabWidget::tabRemoved(int index)
{
    QTabWidget::tabRemoved(index);
    emit countChanged(count());
}