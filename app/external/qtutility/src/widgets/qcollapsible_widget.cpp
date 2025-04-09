#include "QtUtility/widgets/qcollapsible_widget.hpp"

#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

namespace QtUtility {
namespace widgets {

QCollapsibleWidget::QCollapsibleWidget(const QString &title, QWidget *parent)
    : QWidget(parent)
    , m_titleButton(new QToolButton())
    , m_layout(new QVBoxLayout(this))
    , m_content(new QScrollArea())
{
    m_titleButton->setText(title);
    m_titleButton->setStyleSheet("QToolButton { border: none; padding-bottom: 5px; }");
    m_titleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_titleButton->setArrowType(Qt::ArrowType::DownArrow);
    m_titleButton->setCheckable(true);
    m_titleButton->setChecked(false);

    m_content->setStyleSheet("QScrollArea { border: none; }");
    m_layout->setAlignment(Qt::AlignTop);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_titleButton);
    m_layout->addWidget(m_content);

    m_content->setWidgetResizable(true);

    QObject::connect(m_titleButton, &QToolButton::clicked, this, &QCollapsibleWidget::setCollapsed);
}

void QCollapsibleWidget::setWidget(QWidget *widget)
{
    if (!widget)
        return;
    if (m_content->widget() == widget)
        return;
    delete m_content->widget();
    m_content->setWidget(widget);
    updateContentSize();
}

void QCollapsibleWidget::updateContentSize()
{
    emit contentSizeChanged();
}

void QCollapsibleWidget::setCollapsed(bool collapsed)
{
    m_titleButton->setArrowType(collapsed ? Qt::ArrowType::RightArrow : Qt::ArrowType::DownArrow);
    m_content->setVisible(!collapsed);
    emit contentSizeChanged();
}

} // namespace widgets
} // namespace QtUtility
