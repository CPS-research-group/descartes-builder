#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include <QtNodes/Definitions>

class TabComponents;

class AbstractEngine : public QObject
{
    Q_OBJECT
public:
    virtual ~AbstractEngine() {}
    virtual bool execute(std::shared_ptr<TabComponents> tab) = 0;
    QString getExecutionError() const { return m_executionError; }

    virtual bool validityCheck(std::shared_ptr<TabComponents> tab) = 0;
    QStringList getValidityWarnings() const { return m_validityWarnings; }

signals:
    void started();
    void finished(bool success);
    void executed(const QString &output);
    void scoreYmlCreated(const QString &scoreContents); // used for unit tests

protected:
    void setExecutionError(const QString &error) { m_executionError = error; }
    void setValidityWarnings(const QStringList &warnings) { m_validityWarnings = warnings; }
    void appendValidityWarning(const QString &warning) { m_validityWarnings.append(warning); }

private:
    QString m_executionError;
    QStringList m_validityWarnings;
};
