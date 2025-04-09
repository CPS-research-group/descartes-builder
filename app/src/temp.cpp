#include "temp.hpp"

#include <QDebug>

#include "data/settings.hpp"

Temp::Temp(QObject *parent)
    : QObject(parent)
{}

void Temp::printInfo()
{
    qInfo() << "This is an info print";
}

void Temp::printDebug()
{
    qDebug() << "This is a debug print";
}

void Temp::printError()
{
    qCritical() << "This is an error print";
}

void Temp::printAllSettings()
{
    data::Settings::instance().printAll();
}
