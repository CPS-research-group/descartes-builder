// this class is meant to hold temporary code for testing purpose
// will be removed in the future or excluded from compilation
#pragma once

#include <QObject>

class Temp : public QObject
{
    Q_OBJECT
public:
    Temp(QObject *parent = nullptr);

public slots:
    void printInfo();
    void printDebug();
    void printError();
    void printAllSettings();
};
