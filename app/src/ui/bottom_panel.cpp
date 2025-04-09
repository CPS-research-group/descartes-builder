#include "ui/bottom_panel.hpp"

#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedWidget>

#include "ui/log_panel.hpp"
#include "ui/output_panel.hpp"

BottomPanel::BottomPanel()
    : QDockWidget("Bottom Panel")
    , m_content(new QStackedWidget)
{
    auto widget = new QWidget();
    auto layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    layout->setAlignment(Qt::AlignLeft);
    setTitleBarWidget(widget);
    setWidget(m_content);
    setFeatures(QDockWidget::NoDockWidgetFeatures);

    auto logButton = new QPushButton("Log Panel");
    auto outputButton = new QPushButton("Output");
    m_outputPanel = new OutputPanel();
    m_panels.push_back({"Log Panel", logButton, 0, new LogPanel});
    m_panels.push_back({"Output", outputButton, 0, m_outputPanel});
    for (auto panel : m_panels) {
        layout->addWidget(panel.button);
        const auto index = m_content->addWidget(panel.widget);
        connect(panel.button, &QPushButton::clicked, m_content, [this, index]() {
            m_content->setCurrentIndex(index);
        });
    }
}

void BottomPanel::appendOutputPanel(const QString &text)
{
    m_outputPanel->appendPlainText(text);
    m_outputPanel->setFont(QFont("Courier", 12));
    m_outputPanel->setLineWrapMode(QPlainTextEdit::NoWrap);
}
