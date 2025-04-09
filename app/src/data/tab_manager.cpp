#include "data/tab_manager.hpp"

#include <QDebug>

#include <QtNodes/DagGraphicsScene>
#include <QtNodes/GraphicsView>

#include "data/custom_graph.hpp"
#include "ui/models/uid_manager.hpp"

using QtNodes::DagGraphicsScene;
using QtNodes::GraphicsView;

TabManager::TabManager(QObject *parent)
    : QObject(parent)
{}

std::shared_ptr<TabComponents> TabManager::getCurrentTab() const
{
    if (m_currentView)
        return getTab(m_currentView);
    return nullptr;
}

std::shared_ptr<TabComponents> TabManager::getTab(QWidget *view) const
{
    if (m_tabs.count(view) < 1)
        return std::shared_ptr<TabComponents>();
    return m_tabs.at(view);
}

CustomGraph *TabManager::currentGraph() const
{
    if (auto tab = getCurrentTab())
        return tab->getGraph();
    return nullptr;
}

DagGraphicsScene *TabManager::currentScene() const
{
    if (auto tab = getCurrentTab())
        return tab->getScene();
    return nullptr;
}

GraphicsView *TabManager::currentView() const
{
    if (auto tab = getCurrentTab())
        return tab->getView();
    return nullptr;
}

QString TabManager::currentTabName() const
{
    if (auto tab = getCurrentTab())
        return tab->getFileInfo().baseName();
    return QString();
}

bool TabManager::addTab(std::shared_ptr<TabComponents> tab)
{
    if (!tab || m_tabs.count(tab->getView()) > 0)
        return false;
    m_tabs[tab->getView()] = tab;
    emit tabCreated(tab->getView());
    setCurrentView(tab->getView());
    return true;
}

void TabManager::removeTab(const TabComponents &tab)
{
    return removeTab(tab.getView());
}

void TabManager::removeTab(ViewWidget *view)
{
    m_tabs.erase(view);
    emit tabDeleted(view);
}

void TabManager::setCurrentView(ViewWidget *view)
{
    if (m_currentView == view)
        return;
    // qDebug() << "Current view changed to: " << m_tabs.at(view)->getFileInfo().baseName();
    m_currentView = view;
    emit currentChanged(view);
}

QFileInfo TabManager::getFileInfo(ViewWidget *view) const
{
    if (m_tabs.count(view) < 1)
        return QFileInfo();
    return m_tabs.at(view)->getFileInfo();
}

UIDManager *TabManager::getCurrentUIDManager() const
{
    if (auto tab = getCurrentTab()) {
        // return the underlying pointer here; the calling fn should not delete/reassign this
        return tab->getUIDManager().get();
    }
    return nullptr;
}

void TabManager::clear()
{
    auto it = m_tabs.begin();
    while (it != m_tabs.end()) {
        auto view = it->first;
        it = m_tabs.erase(it);
        emit tabDeleted(view);
    }
}

void TabManager::newTab()
{
    addTab(std::make_shared<TabComponents>(m_tabParent));
}

bool TabManager::save()
{
    auto tab = getCurrentTab();
    if (!tab)
        return false;
    auto result = tab->save();
    emit tabFileNameChanged(tab->getView(), tab->getFileInfo().baseName());
    return result;
}

bool TabManager::saveAs()
{
    auto tab = getCurrentTab();
    if (!tab)
        return false;
    auto result = tab->saveAs();
    emit tabFileNameChanged(tab->getView(), tab->getFileInfo().baseName());
    return result;
}

bool TabManager::open()
{
    // open in a new tab
    auto tab = std::make_shared<TabComponents>(m_tabParent);
    if (!tab->open() || openIfExists(tab->getFileInfo()))
        return false;
    if (addTab(tab))
        return tab->openExisting();
    return false;
}

bool TabManager::openFrom(const QString &filePath)
{
    // open in a new tab
    QFileInfo file(filePath);
    auto tab = std::make_shared<TabComponents>(m_tabParent, file);
    if (addTab(tab))
        return tab->openExisting();
    return false;
}

bool TabManager::openIfExists(const QFileInfo &file)
{
    if (!file.exists())
        return false;
    for (auto tab : m_tabs)
        if (tab.second->getFileInfo() == file) { // file exists, open that tab
            setCurrentView(tab.first);
            return true;
        }
    return false;
}
