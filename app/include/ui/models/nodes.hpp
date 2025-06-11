#pragma once

#include <QtNodes/NodeData>

#include "data/constants.hpp"
#include "uid_manager.hpp"

using QtNodes::NodeData;
using QtNodes::NodeDataType;

class NamedNode : public NodeData
{
public:
    NamedNode(const QString &name)
        : m_defaultName(name)
        , m_type({"NamedNode", m_defaultName})
    {}

    virtual QString id() { return m_type.id; }
    QString name() const { return m_type.name; }
    QString defaultName() const { return m_defaultName; }
    // In setter functions, sanitize the name silently to ensure it meets kedro requirements, and does not raise errors
    // valid name characters are a-z, A-Z, 0-9, underscore (_), hyphen (-), and full stop (.)
    void setName(const QString &name) { m_type.name = constants::sanitizeCaption(name); }
    void setDefaultName(const QString &name) { m_defaultName = constants::sanitizeCaption(name); }
    void reset() { m_type.name = m_defaultName; }
    NodeDataType type() const override { return m_type; }

protected:
    QString m_defaultName;
    NodeDataType m_type;
};

class DataNode : public NamedNode
{
public:
    DataNode()
        : DataNode("data")
    {}
    DataNode(const QString &name)
        : NamedNode(name)
        , m_typeId(UIDManager::NONE_ID) // Initialize m_typeId with a default value
    {
        m_type.id = constants::DATA_PORT_ID;
        m_type.name = name;
    }
    FdfUID typeId() const { return m_typeId; }
    void setTypeId(const FdfUID &typeId) { m_typeId = typeId; }

private:
    FdfUID m_typeId;
};

class FunctionNode : public NamedNode
{
public:
    FunctionNode()
        : FunctionNode("function")
    {}
    FunctionNode(const QString &name)
        : NamedNode(name)
    {
        m_type.id = constants::FUNCTION_PORT_ID;
        m_type.name = name;
    }
    Signature signature() const { return m_signature; }
    void setSignature(const Signature &signature) { m_signature = signature; }

private:
    Signature m_signature;
};
