#include "ui/side_bar_widgets/charts.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

#include <QtUtility/widgets/qimage_gallery.hpp>

#include "data/block_manager.hpp"
#include "ui/models/fdf_block_model.hpp"

using QImageGallery = QtUtility::widgets::QImageGallery;

Charts::Charts(std::shared_ptr<BlockManager> blockManager, QWidget *parent)
    : QWidget(parent)
    , m_blockManager(blockManager)
    , m_nodeId(QtNodes::InvalidNodeId)
    , m_imageGallery(new QImageGallery)
    , m_fieldsLayout(new QFormLayout)
{
    auto layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 20, 0, 0);

    layout->addWidget(m_imageGallery, 2);
    layout->addWidget(new QLabel("Fields"));
    m_fieldsLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    layout->addLayout(m_fieldsLayout, 1);

    connect(m_blockManager.get(), &BlockManager::nodeSelected, this, &Charts::setNodeId);
    connect(m_blockManager.get(), &BlockManager::nodeUpdated, this, &Charts::onNodeUpdated);
    connect(this, &Charts::nodeIdChanged, this, &Charts::updateFields);
}

void Charts::setNodeId(QtNodes::NodeId id)
{
    if (m_nodeId == id)
        return;
    m_nodeId = id;
    emit nodeIdChanged(id);
}

void Charts::updateFields()
{
    clearFields();
    auto block = m_blockManager->getBlock(m_nodeId);
    if (!block)
        return;

    //images
    for (auto &path : block->getExecutedGraphs())
        m_imageGallery->add(path);

    //fields
    for (auto &pair : block->getExecutedValues()) {
        auto value = new QLineEdit(pair.second);
        value->setEnabled(false);
        m_fieldsLayout->addRow(new QLabel(QString("%1: ").arg(pair.first)), value);
    }
}

void Charts::clearFields()
{
    // images
    m_imageGallery->clear();

    //fields
    for (int i = m_fieldsLayout->rowCount() - 1; i >= 0; --i)
        m_fieldsLayout->removeRow(i);
}

void Charts::onNodeUpdated(QtNodes::NodeId id)
{
    if (id != m_nodeId)
        return;
    updateFields();
}
