#pragma once

#include "data/constants.hpp"
#include "data/tab_manager.hpp"
#include "uid_manager.hpp"
#include <QtNodes/NodeData>

using QtNodes::NodeData;
using QtNodes::NodeDataType;

class NamedNode : public NodeData
{
public:
    NamedNode(const QString &name)
        : m_defaultName(
              name) // TODO : Verify and remove m_defaultName. All calls accessing this attribute have been removed
        , m_type({"NamedNode", name})
    {}

    virtual QString id() { return m_type.id; }
    QString name() const { return m_type.name; }
    QString defaultName() const { return m_defaultName; }
    void setName(const QString &name) { m_type.name = name; }
    void setDefaultName(const QString &name) { m_defaultName = name; }
    void reset() { m_type.name = m_defaultName; }
    NodeDataType type() const override { return m_type; }

protected:
    QString m_defaultName;
    NodeDataType m_type;
};

class DataNode : public NamedNode
{
public:
    DataNode();
    DataNode(const QString &name);
    DataNode(FdfUID typeId);

    FdfUID typeId() const;
    void setTypeId(const FdfUID &typeId);
    void setTypeTagName(const QString &name);
    void setAnnotation(const QString &annot);
    QString typeTagName() const;
    QString annotation() const;
    void updateDisplayName();
    void setPlaceHolderCaption(QString typeTag, QString annot);

private:
    FdfUID m_typeId;
    QString m_typeTagName;
    QString m_annotation;
    void setParams(const QString &name);
};

class FunctionNode : public NamedNode
{
public:
    FunctionNode();
    FunctionNode(const QString &name);

    Signature signature() const;
    void setSignature(const Signature &signature);

private:
    Signature m_signature;
};
