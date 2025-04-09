#include "ui/output_panel.hpp"

#include <QApplication>

OutputPanel::OutputPanel(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setReadOnly(true);
}