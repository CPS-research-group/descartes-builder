#include "QtUtility/widgets/qimage_gallery.hpp"

#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QStandardPaths>
#include <QVBoxLayout>

#include "QtUtility/widgets/image_list_model.hpp"
#include <QtUtility/data/constexpr_qstring.hpp>
#include <QtUtility/media/media.hpp>

using ConstLatin1String = QtUtility::data::ConstLatin1String;

namespace {
constexpr uint ICON_SIZE = 100;
constexpr uint IMAGE_WIDTH = 450;
constexpr ConstLatin1String NO_SELECTION_TEXT = "No Image Selected.";
} // namespace

namespace QtUtility {
namespace widgets {

QImageGallery::QImageGallery(QWidget *parent)
    : QWidget(parent)
    , m_images(new ImageListModel)
{
    auto layout = new QVBoxLayout(this);

    m_list = new QListView;
    m_list->setModel(m_images);
    m_list->setViewMode(QListView::IconMode);
    m_list->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_list->setWrapping(false);
    m_list->setFlow(QListView::LeftToRight);
    m_list->setResizeMode(QListView::Adjust);
    m_list->setFixedHeight(ICON_SIZE);

    auto downloadButton = new QPushButton("Download All");
    downloadButton->setIcon(QtUtility::media::recolor(QIcon(":/download.png"), Qt::white));

    m_viewer = new QLabel(NO_SELECTION_TEXT);

    layout->addWidget(m_list);
    layout->addWidget(downloadButton);
    layout->addWidget(m_viewer, 1);

    connect(m_list->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &QImageGallery::onImageSelected,
            Qt::QueuedConnection);
    connect(downloadButton, &QPushButton::clicked, this, &QImageGallery::downloadAllClicked);
    connect(this, &QImageGallery::hasImage, downloadButton, &QPushButton::setEnabled);
}

size_t QImageGallery::count() const
{
    return m_images->rowCount();
}

QPixmap QImageGallery::at(const size_t &index) const
{
    return m_images->at(index);
}

void QImageGallery::add(const QPixmap &image)
{
    m_images->add(image);
    if (m_images->count() == 1)
        emit hasImage(true);
}

void QImageGallery::add(const QString &path)
{
    QPixmap image(path);
    if (image.isNull())
        return;
    add(image);
}

void QImageGallery::remove(const size_t &index)
{
    m_images->remove(index);
    if (m_images->count() == 0)
        emit hasImage(false);
}

void QImageGallery::clear()
{
    clearViewer();
    m_images->clear();
}

void QImageGallery::downloadAllClicked()
{
    QString selectedDir = QFileDialog::getExistingDirectory(this,
                                                            "Download graphs to",
                                                            QStandardPaths::writableLocation(
                                                                QStandardPaths::DownloadLocation));
    if (selectedDir.isEmpty())
        return;
    QDir target(selectedDir + "/graphs");
    target.mkpath(".");
    for (int i = 0; i < m_images->count(); ++i) {
        QString path = target.absoluteFilePath(QString("graph%1.png").arg(i + 1));
        if (!m_images->at(i).save(path, "PNG")) {
            qCritical() << "Failed to save png: " << path;
            return;
        }
    }
    qInfo() << "Graphs downloaded to: " << target.absolutePath();
}

void QImageGallery::clearViewer()
{
    m_viewer->clear();
    m_viewer->setText(NO_SELECTION_TEXT);
}

void QImageGallery::onImageSelected()
{
    QItemSelectionModel *selectionModel = m_list->selectionModel();
    if (selectionModel->selectedIndexes().isEmpty()) {
        clearViewer();
    } else {
        QModelIndex index = selectionModel->currentIndex();
        m_viewer->setPixmap(
            m_images->at(index.row()).scaledToWidth(IMAGE_WIDTH, Qt::SmoothTransformation));
    }
}

} // namespace widgets
} // namespace QtUtility