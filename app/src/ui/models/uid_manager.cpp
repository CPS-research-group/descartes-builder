#include "ui/models/uid_manager.hpp"
#include "data/custom_graph.hpp"
#include "ui/models/fdf_block_model.hpp"
#include "ui/models/nodes.hpp"
#include <QDebug>

UIDManager::UIDManager() {}

FdfUID UIDManager::createUID()
{
    FdfUID uid = 0;
    if (!uidToTag.empty()) {
        // find biggest uid in the map, and increment 1 to create new uid
        uid = uidToTag.rbegin()->first + 1;
    }
    QString tag = QString("data_%1").arg(uid);
    updateMap(uid, tag);
    return uid;
}

FdfUID UIDManager::createUID(QString tag)
{
    FdfUID uid = 0;
    if (!uidToTag.empty()) {
        // find biggest uid in the map, and increment 1 to create new uid
        uid = uidToTag.rbegin()->first + 1;
    }
    // make sure the tag is unique
    QString newTag = getUniqueTag(tag);
    updateMap(uid, newTag);
    return uid;
}

QString UIDManager::getUniqueTag(QString tag)
{
    int counter = 1;
    QString newTag = QString("%1").arg(tag);
    while (tagToUid.count(newTag) > 0)
        newTag = QString("%1_%2").arg(tag).arg(counter++);
    return newTag;
}

FdfUID UIDManager::getUid(const QString &tag) const
{
    auto elem = tagToUid.find(tag);
    if (elem != tagToUid.end())
        return elem->second;
    return NONE_ID;
}

QString UIDManager::getTag(const FdfUID &uid) const
{
    auto elem = uidToTag.find(uid);
    if (elem != uidToTag.end())
        return elem->second;
    return NONE_TAG;
}

void UIDManager::updateMap(FdfUID &uid, QString &tag)
{
    if (uid == NONE_ID || tag == NONE_TAG)
        return;
    tag.replace(" ", "");
    if (uidToTag.find(uid) == uidToTag.end()) {
        // UID is created afresh, triggered from createUid()
        uidToTag[uid] = tag;
        tagToUid[tag] = uid;
    } else {
        if (tagToUid.find(tag) == tagToUid.end()) // user changes the type tag on UI for readability
        {
            tagToUid.erase(getTag(uid)); // remove the current entry from tagToId
            // Update both maps
            tagToUid[tag] = uid;
            uidToTag[uid] = tag;
        } else if (uidToTag[uid] == tag) {
            // nothing to update, return
            return;
        } else // user chooses to override a type mismatch, setting a type to another existent type
        {
            FdfUID removeId = std::max(uid, getUid(tag));
            FdfUID keepId = std::min(uid, getUid(tag));
            // update the graph to replace all instances of removeId with keepId
            overrideType(removeId, keepId);
            tagToUid.erase(getTag(removeId));
            uidToTag.erase(removeId);
        }
        refreshDisplayNames(); // calls updateDisplayName on all DataNodes on a type override
    }
    // displayMaps();
}

void UIDManager::displayMaps() const
{
    qInfo() << "UID to Tag Map:";
    for (const auto &pair : uidToTag) {
        qInfo() << "UID:" << pair.first << "-> Tag:" << pair.second;
    }

    qInfo() << "Tag to UID Map:";
    for (const auto &pair : tagToUid) {
        qInfo() << "Tag:" << pair.first << "-> UID:" << pair.second;
    }
}

void UIDManager::refreshDisplayNames()
{
    if (!graph) {
        qWarning() << "UID Manager does not have an associated graph!";
        return;
    }

    for (const auto &id : graph->allNodeIds()) {
        auto block = graph->delegateModel<FdfBlockModel>(id);
        if (!block)
            continue;

        for (int i = 0; i < block->nPorts(PortType::Out); i++) {
            if (auto dataPort = std::dynamic_pointer_cast<DataNode>(block->outData(i))) {
                QString currentDisplayName = dataPort->name();
                dataPort->updateDisplayName();
                if (currentDisplayName != dataPort->name()) {
                    block->propagateUpdate();
                }
                // we need to also ensure output port captions remain unique after tag changes
                // since captions are now deterministic
                graph->makeOutPortsUnique(id, block, i);
            }
        }
    }
}

void UIDManager::overrideType(FdfUID removeType, FdfUID keepType)
{
    if (!graph) {
        qWarning() << "UID Manager does not have an associated graph!";
        return;
    }
    // Step 1: Update types in the graph
    for (const auto &id : graph->allNodeIds()) {
        auto block = graph->delegateModel<FdfBlockModel>(id);
        if (!block)
            continue;

        bool updated = false;

        for (int i = 0; i < block->nPorts(PortType::Out); i++) {
            auto dataPort = std::dynamic_pointer_cast<DataNode>(block->outData(i));
            if (dataPort && dataPort->typeId() == removeType) {
                dataPort->setTypeId(keepType);
                updated = true;
                continue;
            }

            auto functionPort = std::dynamic_pointer_cast<FunctionNode>(block->outData(i));
            if (!functionPort)
                continue;

            Signature signature = functionPort->signature();

            // Replace type in input and output signatures
            updated = replaceTypesInUIDVector(signature.inputs, keepType, removeType);
            updated = replaceTypesInUIDVector(signature.outputs, keepType, removeType);

            if (updated)
                functionPort->setSignature(signature);
        }

        if (updated)
            block->propagateUpdate();
    }
}

bool UIDManager::replaceTypesInUIDVector(std::vector<FdfUID> &vec,
                                         FdfUID keepType,
                                         FdfUID removeType)
{
    bool updated = false;
    for (FdfUID &item : vec)
        if (item == removeType) {
            item = keepType;
            updated = true;
        }
    return updated;
}

ConnectionInfo UIDManager::getConnectionInfo(QtNodes::ConnectionId const connectionId) const
{
    ConnectionInfo connInfo;
    if (!graph) {
        qWarning() << "UID Manager does not have an associated graph!";
        return connInfo;
    }
    connInfo.inNodeId = getNodeId(PortType::In, connectionId);
    connInfo.outNodeId = getNodeId(PortType::Out, connectionId);
    if (auto block = graph->delegateModel<FdfBlockModel>(getNodeId(PortType::In, connectionId)))
        if (auto outBlock = graph->delegateModel<FdfBlockModel>(
                getNodeId(PortType::Out, connectionId))) {
            // Fetch the block names
            connInfo.inBlockCaption = block->caption();
            connInfo.outBlockCaption = outBlock->caption();
            // Fetch the incoming/received indices
            connInfo.outIndex = getPortIndex(PortType::Out, connectionId);
            connInfo.inIndex = getPortIndex(PortType::In, connectionId);
            // Fetch the incoming type
            if (auto data = std::dynamic_pointer_cast<DataNode>(
                    outBlock->outData(connInfo.outIndex)))
                connInfo.receivedOutType = data->typeId();
            else if (auto function = std::dynamic_pointer_cast<FunctionNode>(
                         outBlock->outData(connInfo.outIndex))) {
                std::shared_ptr<Signature> receivedSig = std::make_shared<Signature>(
                    function->signature());
                connInfo.receivedSignature = receivedSig;
            }
        }
    return connInfo;
}