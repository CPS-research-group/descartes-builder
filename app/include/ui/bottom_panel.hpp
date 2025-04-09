#pragma once

#include <QDockWidget>

#include <unordered_set>

class QStackedWidget;
class QPushButton;
class QPlainTextEdit;

class BottomPanel : public QDockWidget
{
    Q_OBJECT
public:
    BottomPanel();

public slots:
    void appendOutputPanel(const QString &text);

private:
    QStackedWidget *m_content;
    struct Panel
    {
        QString title;
        QPushButton *button;
        uint counter;
        QPlainTextEdit *widget;
    };
    std::vector<Panel> m_panels;
    QPlainTextEdit *m_outputPanel;
};
