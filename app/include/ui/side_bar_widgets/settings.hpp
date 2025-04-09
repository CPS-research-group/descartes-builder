#pragma once

#include <QWidget>

class QComboBox;
class QSpinBox;

class Settings : public QWidget
{
    Q_OBJECT
public:
    Settings(QWidget *parent = nullptr);

public slots:
    void onSettingUpdated(const QString &key, const QVariant &value);

private:
    QComboBox *m_formatBox;
    QComboBox *m_engineBox;
    QSpinBox *m_engineTimeoutBox;
};
