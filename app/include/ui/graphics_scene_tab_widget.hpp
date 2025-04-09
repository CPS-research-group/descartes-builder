#pragma once

#include <QTabWidget>

class TabComponents;
class TabManager;
class QPushButton;

class GraphicsSceneTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    GraphicsSceneTabWidget(std::shared_ptr<TabManager> tabManager, QWidget *parent = nullptr);

signals:
    void countChanged(int count);
    void runClicked();

public slots:
    void closeCurrentTab();
    void nextTab();
    void previousTab();
    void setRunState(bool state);
    void runStarted();
    void runFinished();

private slots:
    void closeTab(int index);
    void onTabCountChanged(int count);
    void onCurrentChanged(const int &index);
    // slots for tab manager
    void onTabCreated(QWidget *widget);
    void onTabFileNameChanged(QWidget *widget, const QString &fileName);

protected:
    virtual void tabInserted(int index) override;
    virtual void tabRemoved(int index) override;

private:
    std::shared_ptr<TabManager> m_tabManager;

    QPushButton *m_runButton;
};