#pragma once

#include <QObject>
#include <QtMessageHandler>

#include <QtUtility/data/qsingleton.hpp>

#include "ui/log_panel.hpp"

class LogManager : public QSingleton<LogManager>
{
    Q_OBJECT
    friend class QSingleton<LogManager>;

public:
    ~LogManager() override;
    void init();
    void registerLogPanel(LogPanel *panel);

signals:
    void callAppendMessage(const QString &message, const QtMsgType &type);

public slots:
    void appendMessage(const QString &message, const QtMsgType &type = QtInfoMsg);

private:
    QtMessageHandler m_originalHandler;
    QVector<LogPanel *> m_logPanels;
    struct LogMessage
    {
        QString string;
        QColor color;
    };
    std::vector<LogMessage> m_notPrinted;
};