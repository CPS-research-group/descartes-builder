#pragma once

#include <QObject>

#include "data/tab_components.hpp"

class TabManager : public QObject
{
    Q_OBJECT
public:
    using ViewWidget = QWidget;

    static TabManager &instance()
    {
        static TabManager instance;
        return instance;
    }
    static UIDManager *getUIDManager()
    {
        return TabManager::instance().getCurrentUIDManager();
    }

    TabManager(const TabManager &) = delete;
    TabManager &operator=(const TabManager &) = delete;

    std::shared_ptr<TabComponents> getCurrentTab() const;
    std::shared_ptr<TabComponents> getTab(QWidget *view) const;
    size_t size() const { return m_tabs.size(); }
    QWidget *currentWidget() const { return m_currentView; }
    CustomGraph *currentGraph() const;
    QtNodes::DagGraphicsScene *currentScene() const;
    QtNodes::GraphicsView *currentView() const;
    QString currentTabName() const;
    bool addTab(std::shared_ptr<TabComponents> tab);
    void removeTab(const TabComponents &tab);
    void removeTab(ViewWidget *view);
    void setTabParent(QWidget *parent) { m_tabParent = parent; }
    void setCurrentView(ViewWidget *view);
    QFileInfo getFileInfo(ViewWidget *view) const;
    UIDManager *getCurrentUIDManager() const;
    void clear();

signals:
    void tabCreated(ViewWidget *view);
    void tabDeleted(ViewWidget *view);
    void currentChanged(ViewWidget *view);
    void tabFileNameChanged(ViewWidget *view, QString fileName);

public:
    void newTab();
    bool save();
    bool saveAs();
    bool open();
    bool openFrom(const QString &filePath);

private:
    bool openIfExists(const QFileInfo &file);
    explicit TabManager(QObject *parent = nullptr);

    std::unordered_map<ViewWidget *, std::shared_ptr<TabComponents>> m_tabs;
    ViewWidget *m_currentView;
    QWidget *m_tabParent;
};
