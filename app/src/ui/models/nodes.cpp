#include "ui/models/nodes.hpp"

DataNode::DataNode()
    : NamedNode("data")
{
    m_type.id = constants::DATA_PORT_ID;
    m_typeId = UIDManager::NONE_ID;
    m_typeTagName = UIDManager::NONE_TAG;
    m_annotation = "";
    updateDisplayName();
}

DataNode::DataNode(const QString &name)
    : NamedNode(name)
    , m_annotation("")
{
    m_type.id = constants::DATA_PORT_ID;
    setParams(name);
    updateDisplayName();
}

DataNode::DataNode(FdfUID typeId)
    : NamedNode("data")
{
    m_type.id = constants::DATA_PORT_ID;
    auto uidManager = TabManager::getUIDManager();
    auto tag = uidManager->getTag(typeId);
    if (tag != UIDManager::NONE_TAG) {
        m_typeId = typeId;
        m_typeTagName = tag;
    } else {
        setParams("data");
    }
    updateDisplayName();
}

FdfUID DataNode::typeId() const
{
    return m_typeId;
}

void DataNode::setTypeId(const FdfUID &typeId)
{
    // Here, the intention is to use the existing type tag for the given type ID
    auto uidManager = TabManager::getUIDManager();
    m_typeId = typeId;
    m_typeTagName = uidManager->getTag(m_typeId);
    updateDisplayName();
}

void DataNode::setTypeTagName(const QString &name)
{
    // Here, the type tag for this port's type ID will be changed
    auto uidManager = TabManager::getUIDManager();
    m_typeTagName = constants::sanitizeCaption(name);
    uidManager->updateMap(m_typeId, m_typeTagName);
    updateDisplayName();
}

void DataNode::setAnnotation(const QString &annot)
{
    m_annotation = constants::sanitizeCaption(annot);
    updateDisplayName();
}

QString DataNode::typeTagName() const
{
    return m_typeTagName;
}

QString DataNode::annotation() const
{
    return m_annotation;
}

void DataNode::updateDisplayName()
{
    auto uidManager = TabManager::getUIDManager();
    m_typeTagName = uidManager->getTag(m_typeId);
    m_type.name = m_annotation.isEmpty() ? m_typeTagName : m_typeTagName + "_" + m_annotation;
}

void DataNode::setPlaceHolderCaption(QString typeTag, QString annot)
{
    m_annotation = annot;
    m_type.name = annot.isEmpty() ? typeTag : typeTag + "_" + annot;
}

void DataNode::setParams(const QString &name)
{
    auto uidManager = TabManager::getUIDManager();
    m_typeId = uidManager->createUID(name);
    m_typeTagName = uidManager->getTag(m_typeId);
}

FunctionNode::FunctionNode()
    : FunctionNode("function")
{}

FunctionNode::FunctionNode(const QString &name)
    : NamedNode(name)
{
    m_type.id = constants::FUNCTION_PORT_ID;
    m_type.name = name;
}

Signature FunctionNode::signature() const
{
    return m_signature;
}

void FunctionNode::setSignature(const Signature &signature)
{
    m_signature = signature;
}
