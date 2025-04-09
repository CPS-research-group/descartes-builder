#pragma once

#include <QWidget>

#include "QtUtility/export.hpp"

class QToolButton;
class QVBoxLayout;
class QScrollArea;

namespace QtUtility {
namespace widgets {

class QTUTILITY_EXPORT QCollapsibleWidget : public QWidget
{
    Q_OBJECT
public:
    QCollapsibleWidget(const QString &title = "", QWidget *parent = nullptr);
    void setWidget(QWidget *widget);

signals:
    void contentSizeChanged();

public slots:
    void setCollapsed(bool collapsed);
    void updateContentSize();

private:
    QToolButton *m_titleButton;
    QVBoxLayout *m_layout;
    QScrollArea *m_content;
};

} // namespace widgets
} // namespace QtUtility