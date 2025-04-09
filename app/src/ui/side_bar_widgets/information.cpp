#include "ui/side_bar_widgets/information.hpp"

#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

#include "data/constants.hpp"

Information::Information(QWidget *parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 20, 0, 0);

    auto logo = new QLabel;
    logo->setPixmap(
        QPixmap(":/descartes_logo.png").scaledToWidth(constants::SIDE_BAR_MINIMUM_WIDTH));
    auto description = new QLabel(
        "DesCartes Builder - Function + data flow GUI for creating digital twin pipelines.");
    description->setWordWrap(true);

    layout->addWidget(logo);
    layout->addWidget(description);
}
