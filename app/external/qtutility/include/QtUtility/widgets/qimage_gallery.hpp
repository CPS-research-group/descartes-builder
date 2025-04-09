#pragma once

#include <QWidget>

#include "QtUtility/export.hpp"

class QListView;
class QLabel;

namespace QtUtility {
namespace widgets {

class ImageListModel;

class QTUTILITY_EXPORT QImageGallery : public QWidget
{
    Q_OBJECT
public:
    QImageGallery(QWidget *parent = nullptr);
    size_t count() const;
    QPixmap at(const size_t &index) const;
    void add(const QPixmap &image);
    void add(const QString &path);
    void remove(const size_t &index);
    void clear();

signals:
    void hasImage(bool state);

private slots:
    void onImageSelected();
    void downloadAllClicked();

private:
    void clearViewer();

    ImageListModel *m_images;

    QListView *m_list;
    // could be upgraded to a QGraphicsScene if we need more capabilities
    QLabel *m_viewer;
};

} // namespace widgets
} // namespace QtUtility