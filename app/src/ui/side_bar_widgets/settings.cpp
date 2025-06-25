#include "ui/side_bar_widgets/settings.hpp"

#include "ui/main_window.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

#include "data/settings.hpp"

namespace {

QVariant settingValue(const QString &key)
{
    return data::Settings::instance().value(key);
}

} // namespace

Settings::Settings(MainWindow *mw, QWidget *parent)
    : QWidget(parent)
    , m_formatBox(new QComboBox)
    , m_engineBox(new QComboBox)
    , m_engineTimeoutBox(new QSpinBox)
    , mainWindowPtr(mw)
{
    auto scrollArea = new QScrollArea;
    {
        auto layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(scrollArea);
    }
    auto widget = new QWidget;
    auto layout = new QVBoxLayout(widget);
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 20, 0, 0);

    {
        // settings UI is hard coded, could be improved to generate from a schema
        layout->addWidget(new QLabel("Default export format: "));
        m_formatBox->addItems({".dcb (Graph + data)", ".dag (Graph only)"});
        layout->addWidget(m_formatBox);

        layout->addWidget(new QLabel("Engine: "));
        m_engineBox->addItems({"kedro"});
        layout->addWidget(m_engineBox);

        layout->addWidget(new QLabel("engine timeout (minutes): "));
        m_engineTimeoutBox->setRange(1, 20);
        layout->addWidget(m_engineTimeoutBox);

        QCheckBox *gridEnable = new QCheckBox("Show Grid", this);
        gridEnable->setChecked(true);
        layout->addWidget(gridEnable);
        connect(gridEnable, &QCheckBox::toggled, mainWindowPtr, &MainWindow::gridToggled);

        { // set default values to the UI
            m_formatBox->setCurrentText(settingValue("default export format").toString());
            m_engineBox->setCurrentText(settingValue("engine").toString());
            m_engineTimeoutBox->setValue(settingValue("engine timeout (minutes)").toInt());
        }

        auto &s = data::Settings::instance();
        { // setup connects to modify settings
            connect(m_formatBox, &QComboBox::currentTextChanged, &s, [&s](const QString &value) {
                s.setValue("default export format", value);
            });
            connect(m_engineBox, &QComboBox::currentTextChanged, &s, [&s](const QString &value) {
                s.setValue("engine", value);
            });
            connect(m_engineTimeoutBox, &QSpinBox::valueChanged, &s, [&s](const int &value) {
                s.setValue("engine timeout (minutes)", value);
            });
        }

        // connects for updating setting changes
        connect(&s, &data::Settings::settingUpdated, this, &Settings::onSettingUpdated);
    }

    scrollArea->setWidget(widget);
}

void Settings::onSettingUpdated(const QString &key, const QVariant &value)
{
    if (key == "default export format") {
        m_formatBox->blockSignals(true);
        m_formatBox->setCurrentText(value.toString());
        m_formatBox->blockSignals(false);
    } else if (key == "engine") {
        m_engineBox->blockSignals(true);
        m_engineBox->setCurrentText(value.toString());
        m_engineBox->blockSignals(false);
    } else if (key == "engine timeout (minutes)") {
        m_engineTimeoutBox->blockSignals(true);
        m_engineTimeoutBox->setValue(value.toInt());
        m_engineTimeoutBox->blockSignals(false);
    } else {
        qCritical() << "Setting update key not handled: " << key;
    }
}
