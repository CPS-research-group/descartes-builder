#include "QtUtility/widgets/image_list_model.hpp"

#include <QIcon>

namespace QtUtility {
namespace widgets {

ImageListModel::ImageListModel(QObject *parent)
    : QAbstractListModel(parent)
{}

int ImageListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_images.size();
}

QVariant ImageListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_images.size())
        return QVariant();
    if (role == Qt::DecorationRole)
        return QIcon(m_images.at(index.row()));
    return QVariant();
}

size_t ImageListModel::count() const
{
    return m_images.size();
}

QPixmap ImageListModel::at(const size_t &index) const
{
    if (index < 0 || index > count())
        return QPixmap();
    return m_images.at(index);
}

void ImageListModel::add(const QPixmap &pixmap)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_images.append(pixmap);
    endInsertRows();
}

void ImageListModel::remove(int index)
{
    if (index >= 0 && index < m_images.size()) {
        beginRemoveRows(QModelIndex(), index, index);
        m_images.removeAt(index);
        endRemoveRows();
    }
}

void ImageListModel::clear()
{
    beginResetModel();
    m_images.clear();
    endResetModel();
}

} // namespace widgets
} // namespace QtUtility