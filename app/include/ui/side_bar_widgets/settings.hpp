#pragma once

#include <QWidget>

class QComboBox;
class QSpinBox;
class MainWindow;
class Settings : public QWidget
{
    Q_OBJECT
public:
    Settings(MainWindow *mw, QWidget *parent = nullptr);

public slots:
    void onSettingUpdated(const QString &key, const QVariant &value);
signals:
    void gridToggled(bool enabled);

private:
    QComboBox *m_formatBox;
    QComboBox *m_engineBox;
    QSpinBox *m_engineTimeoutBox;
    MainWindow *mainWindowPtr;
};
