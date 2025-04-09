#pragma once

#include <QWidget>

#include <QtNodes/Definitions>

class QFormLayout;

class BlockManager;
namespace QtUtility {
namespace widgets {
class QImageGallery;
} // namespace widgets
} // namespace QtUtility

class Charts : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QtNodes::NodeId nodeId READ nodeId WRITE setNodeId NOTIFY nodeIdChanged)
public:
    Charts(std::shared_ptr<BlockManager> blockManager, QWidget *parent = nullptr);
    QtNodes::NodeId nodeId() const { return m_nodeId; }

public slots:
    void setNodeId(QtNodes::NodeId id);

signals:
    void nodeIdChanged(QtNodes::NodeId id);

private slots:
    void updateFields();
    void clearFields();
    void onNodeUpdated(QtNodes::NodeId id);

private:
    std::shared_ptr<BlockManager> m_blockManager;
    QtNodes::NodeId m_nodeId;

    QFormLayout *m_fieldsLayout;
    QtUtility::widgets::QImageGallery *m_imageGallery;
};
