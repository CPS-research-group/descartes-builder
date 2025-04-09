#include "data/block_manager.hpp"

#include <QDebug>

#include <QtNodes/ConnectionStyle>
#include <QtNodes/DagGraphicsScene>
#include <QtNodes/NodeDelegateModelRegistry>

#include "data/custom_graph.hpp"
#include "data/tab_manager.hpp"
#include "ui/model_registry.hpp"
#include "ui/models/fdf_block_model.hpp"

using QtNodes::ConnectionStyle;
using QtNodes::DagGraphicsScene;

std::shared_ptr<QtNodes::NodeDelegateModelRegistry> BlockManager::m_registry
    = model_registry::registerDataModels();

BlockManager::BlockManager(QObject *parent)
    : QObject(parent)
{
    ConnectionStyle::registerColor(DataNode().id(), Qt::black);
    ConnectionStyle::registerColor(FunctionNode().id(), Qt::red);
}

void BlockManager::setTabManager(std::shared_ptr<TabManager> tabManager)
{
    if (m_tabManager == tabManager)
        return;
    if (m_tabManager) {
        disconnect(m_tabManager.get(),
                   &TabManager::currentChanged,
                   this,
                   &BlockManager::onSelectionChanged);
        disconnect(m_tabManager.get(), &TabManager::tabCreated, this, &BlockManager::onTabCreated);
    }
    m_tabManager = tabManager;
    connect(m_tabManager.get(),
            &TabManager::currentChanged,
            this,
            &BlockManager::onSelectionChanged);
    connect(m_tabManager.get(), &TabManager::tabCreated, this, &BlockManager::onTabCreated);
}

QJsonObject BlockManager::getJson(QtNodes::NodeId id) const
{
    auto graph = m_tabManager->currentGraph();
    if (!graph || id == QtNodes::InvalidNodeId)
        return QJsonObject();
    return graph->saveNode(id);
}

FdfBlockModel *BlockManager::getBlock(QtNodes::NodeId id) const
{
    auto graph = m_tabManager->currentGraph();
    if (!graph || id == QtNodes::InvalidNodeId)
        return nullptr;
    return graph->delegateModel<FdfBlockModel>(id);
}

QPointF BlockManager::getBlockPosition(QtNodes::NodeId id) const
{
    auto graph = m_tabManager->currentGraph();
    if (!graph || id == QtNodes::InvalidNodeId)
        return QPointF();
    return graph->nodeData(id, QtNodes::NodeRole::Position).value<QPointF>();
}

void BlockManager::setBlockPosition(QtNodes::NodeId id, QPointF point)
{
    auto graph = m_tabManager->currentGraph();
    if (!graph || id == QtNodes::InvalidNodeId)
        return;
    graph->setNodeData(id, QtNodes::NodeRole::Position, point);
}

void BlockManager::onTabCreated(QWidget *view)
{
    if (auto tab = m_tabManager->getTab(view)) {
        connect(tab->getScene(),
                &DagGraphicsScene::selectionChanged,
                this,
                &BlockManager::onSelectionChanged);
        connect(tab->getGraph(),
                &CustomGraph::nodePositionUpdated,
                this,
                &BlockManager::nodeUpdated);
        connect(tab->getGraph(), &CustomGraph::nodeUpdated, this, &BlockManager::nodeUpdated);
    }
}

void BlockManager::onSelectionChanged()
{
    auto scene = m_tabManager->currentScene();
    if (!scene)
        return emit nodeSelected(QtNodes::InvalidNodeId);
    m_selectedNodes = scene->selectedNodes();
    emit nodeSelected(m_selectedNodes.size() < 1 ? QtNodes::InvalidNodeId : m_selectedNodes.front());
}
