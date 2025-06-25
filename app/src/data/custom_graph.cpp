#include "data/custom_graph.hpp"

#include "data/tab_manager.hpp"
#include "ui/models/fdf_block_model.hpp"
#include "ui/models/io_models.hpp"
#include "ui/models/processor_models.hpp"
#include <QAbstractButton>
#include <QApplication>
#include <QMessageBox>
#include <QMetaObject>

using QtNodes::NodeRole;
using QtNodes::PortRole;
namespace {

template<typename MapType>
void removeByValue(MapType &map, const typename MapType::mapped_type &valueToRemove)
{
    for (auto it = map.begin(); it != map.end();)
        if (it->second == valueToRemove)
            it = map.erase(it);
        else
            ++it;
}

template<typename MapType>
void removeByPairFirst(MapType &map,
                       const typename MapType::mapped_type::first_type &firstValueToRemove)
{
    for (auto it = map.begin(); it != map.end();)
        if (it->second.first == firstValueToRemove)
            it = map.erase(it);
        else
            ++it;
}

} // namespace

CustomGraph::CustomGraph(std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registry)
    : DirectedAcyclicGraphModel(registry)
{
    connect(this, &CustomGraph::nodeDeleted, this, &CustomGraph::onNodeDeleted);
}

std::vector<DataSourceModel *> CustomGraph::getDataSourceModels() const
{
    std::vector<DataSourceModel *> result;
    for (auto &id : m_dataSourceNodes)
        result.push_back(delegateModel<DataSourceModel>(id));
    return result;
}

std::vector<FuncOutModel *> CustomGraph::getFuncOutModels() const
{
    std::vector<FuncOutModel *> result;
    for (auto &id : m_funcOutNodes)
        result.push_back(delegateModel<FuncOutModel>(id));
    return result;
}

bool CustomGraph::connectionPossible(QtNodes::ConnectionId const connectionId) const
{
    if (QApplication::mouseButtons() != Qt::NoButton) {
        return true; // Ignore connection check if mouse is still pressed
    }

    auto uidManager = TabManager::getUIDManager();
    ConnectionInfo connInfo = uidManager->getConnectionInfo(connectionId);
    if (auto block = delegateModel<FdfBlockModel>(connInfo.inNodeId)) {
        auto result = block->canConnect(connInfo);
        if (!result)
            return false;
    }
    return DirectedAcyclicGraphModel::connectionPossible(connectionId);
}

void CustomGraph::initBlockConnections(const QtNodes::NodeId nodeId, FdfBlockModel *block)
{
    connect(block, &FdfBlockModel::captionUpdated, this, [this, block, nodeId]() {
        makeCaptionUnique(nodeId, block);
    });
    connect(block,
            &FdfBlockModel::outPortCaptionUpdated,
            this,
            [this, block, nodeId](const PortIndex &index) {
                makeOutPortsUnique(nodeId, block, index);
            });
    connect(block, &FdfBlockModel::outPortInserted, this, [nodeId, this](const PortIndex index) {
        onOutPortInserted(nodeId, index);
    });
    connect(block, &FdfBlockModel::outPortDeleted, this, [nodeId, this](const PortIndex index) {
        onOutPortDeleted(nodeId, index);
    });
}

void CustomGraph::onNodeCreated(const QtNodes::NodeId nodeId)
{
    DirectedAcyclicGraphModel::onNodeCreated(nodeId);
    auto block = delegateModel<FdfBlockModel>(nodeId);
    if (!block)
        return;
    initBlockConnections(nodeId, block);
    makeCaptionUnique(nodeId, block);
    makeOutPortsUnique(nodeId, block);
    stylePorts(nodeId, block);

    if (block->name() == io_names::DATA_SOURCE) {
        m_dataSourceNodes.insert(nodeId);
        auto dataSourceModel = dynamic_cast<DataSourceModel *>(block);
        connect(dataSourceModel, &DataSourceModel::importClicked, this, [nodeId, this]() {
            emit dataSourceModelImportClicked(nodeId);
        });
    } else if (block->name() == io_names::FUNC_OUT)
        m_funcOutNodes.insert(nodeId);
}

void CustomGraph::stylePorts(const QtNodes::NodeId &nodeId, FdfBlockModel *block)
{
    if (!block)
        return;

    auto styleFunctionPorts = [&](PortType portType, NodeRole rolePortCount) {
        unsigned int portCount = nodeData(nodeId, rolePortCount).toUInt();
        for (PortIndex portIndex = 0; portIndex < portCount; ++portIndex) {
            auto data = portData(nodeId, portType, portIndex, PortRole::DataType);
            assert(data.isValid() && data.canConvert<NodeDataType>());
            // Check if the port is associated with a FunctionNode
            if (constants::isFunctionNode(data.value<NodeDataType>())) {
                setPortData(nodeId,
                            portType,
                            portIndex,
                            constants::FUNCTION_PORT_COLOR,
                            PortRole::FontColor);
            }
        }
    };

    styleFunctionPorts(PortType::In, NodeRole::InPortCount);
    styleFunctionPorts(PortType::Out, NodeRole::OutPortCount);
}

void CustomGraph::onNodeDeleted(const QtNodes::NodeId nodeId)
{
    removeByValue(m_usedNodeCaptions, nodeId);
    removeByPairFirst(m_usedOutPortCaptions, nodeId);

    auto block = delegateModel<FdfBlockModel>(nodeId);
    if (!block)
        return;
    if (block->name() == io_names::DATA_SOURCE)
        m_dataSourceNodes.erase(nodeId);
    else if (block->name() == io_names::FUNC_OUT)
        m_funcOutNodes.erase(nodeId);
    m_trackedNodes.erase(nodeId);
}

void CustomGraph::onOutPortInserted(const QtNodes::NodeId nodeId, const QtNodes::PortIndex oldIndex)
{
    auto block = delegateModel<FdfBlockModel>(nodeId);
    if (!block)
        return;
    makeOutPortsUnique(nodeId, block, oldIndex);
}

void CustomGraph::onOutPortDeleted(const QtNodes::NodeId nodeId, const QtNodes::PortIndex oldIndex)
{
    removeByValue(m_usedOutPortCaptions, std::make_pair(nodeId, oldIndex));
    auto block = delegateModel<FdfBlockModel>(nodeId);
    if (!block)
        return;
    if (block->nPorts(PortType::Out) > oldIndex)
        // shift index of ports after the deleted one
        for (int i = oldIndex; i < block->nPorts(PortType::Out); ++i) {
            auto caption = block->portCaption(PortType::Out, i);
            --m_usedOutPortCaptions.at(caption).second;
        }
}

void CustomGraph::makeCaptionUnique(const QtNodes::NodeId &nodeId, FdfBlockModel *block)
{
    QString uniqueCaption = block->caption();
    uint counter = 1;
    if (m_trackedNodes.count(nodeId) > 0) { // if node is already tracked
        if (m_usedNodeCaptions.count(uniqueCaption) > 0
            && m_usedNodeCaptions.at(uniqueCaption) == nodeId)
            return;
        // if caption is different, remove old caption
        removeByValue(m_usedNodeCaptions, nodeId);
    }
    while (m_usedNodeCaptions.count(uniqueCaption) > 0)
        uniqueCaption = QString("%1_%2").arg(block->caption(), QString::number(++counter));
    m_usedNodeCaptions[uniqueCaption] = nodeId;
    m_trackedNodes.insert(nodeId);
    if (block->caption() != uniqueCaption)
        block->setCaption(uniqueCaption);
}

FdfBlockModel *CustomGraph::getBlockByCaption(const QString &caption) const
{
    auto it = m_usedNodeCaptions.find(caption);
    if (it != m_usedNodeCaptions.end()) {
        auto nodeId = it->second;
        return delegateModel<FdfBlockModel>(nodeId);
    }
    return nullptr;
}

void CustomGraph::makeOutPortsUnique(const QtNodes::NodeId &nodeId, FdfBlockModel *block)
{
    auto portType = QtNodes::PortType::Out;
    for (uint i = 0; i < block->nPorts(portType); ++i) {
        makeOutPortsUnique(nodeId, block, i);
    }
}

void CustomGraph::makeOutPortsUnique(const QtNodes::NodeId &nodeId,
                                     FdfBlockModel *block,
                                     const PortIndex &index)
{
    auto portType = QtNodes::PortType::Out;
    const auto ORIGINAL_NAME = block->portCaption(portType, index);
    uint counter = 1;

    if (m_trackedNodes.count(nodeId) > 0) {
        if (m_usedOutPortCaptions.count(ORIGINAL_NAME) > 0
            && m_usedOutPortCaptions.at(ORIGINAL_NAME) == std::make_pair(nodeId, index))
            return;
        // if caption is different, remove old caption
        removeByValue(m_usedOutPortCaptions, std::make_pair(nodeId, index));
    }
    auto uniqueName = ORIGINAL_NAME;
    while (m_usedOutPortCaptions.count(uniqueName) > 0) {
        uniqueName = QString("%1_%2").arg(ORIGINAL_NAME, QString::number(++counter));
    }
    m_usedOutPortCaptions[uniqueName] = std::make_pair(nodeId, index);
    block->setPortCaption(portType, index, uniqueName);
}
