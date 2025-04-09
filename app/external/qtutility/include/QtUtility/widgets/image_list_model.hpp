#pragma once

#include <QAbstractListModel>
#include <QPixmap>

#include "QtUtility/export.hpp"

namespace QtUtility {
namespace widgets {

class QTUTILITY_EXPORT ImageListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    ImageListModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    size_t count() const;
    QPixmap at(const size_t &index) const;
    void add(const QPixmap &pixmap);
    void remove(int index);
    void clear();

private:
    QList<QPixmap> m_images;
};

} // namespace widgets
} // namespace QtUtility