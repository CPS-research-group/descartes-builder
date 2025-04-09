#pragma once

#include <QPlainTextEdit>

class OutputPanel : public QPlainTextEdit
{
    Q_OBJECT
public:
    OutputPanel(QWidget *parent = nullptr);
};
