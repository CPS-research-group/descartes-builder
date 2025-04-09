#pragma once

#include <QLabel>
#include <QObject>
#include <QPointer>
#include <QtNodes/NodeDelegateModel>
using QtNodes::ConnectionId;
using QtNodes::NodeDelegateModel;
using QtNodes::NodeShape;
using QtNodes::PortIndex;
using QtNodes::PortType;

#include "ui/models/nodes.hpp"

class FdfBlockModel : public NodeDelegateModel
{
    Q_OBJECT
public:
    enum FdfType {
        Coder,
        Processor,
        Trainer,
        Data,
        Output,
    };

    FdfBlockModel(FdfType type, const QString &name, const QString &functionName = QString());
    FdfType type() const { return m_type; }
    QString typeAsString() const { return TYPE_STRING.at(m_type); }
    QString name() const override { return m_name; }
    NodeShape shape() const override { return m_shape; }
    QString functionName() const { return m_functionName; }
    QString caption() const override { return m_caption; }
    virtual bool hasParameters() const { return getParameters().size() > 0; }
    unsigned int nPorts(PortType const portType) const override;
    NodeDataType dataType(PortType const portType, PortIndex const portIndex) const override;
    std::shared_ptr<NodeData> outData(PortIndex const index) override;
    virtual void setInData(std::shared_ptr<NodeData> data, PortIndex const index) override;
    virtual QWidget *embeddedWidget() override;
    QString portCaption(PortType portType, PortIndex portIndex) const override;
    QString defaultPortCaption(PortType portType, PortIndex portIndex) const;
    QJsonObject save() const override;
    void load(QJsonObject const &p) override;

    virtual std::shared_ptr<NodeData> portData(PortType const type, PortIndex const index) const;
    virtual std::vector<std::shared_ptr<NodeData>> connectedPortData(PortType const type) const;

    void setCaption(const QString &caption);
    bool setPortCaption(PortType type, PortIndex index, const QString &caption);
    bool setPortDefaultCaption(PortType type, PortIndex index, const QString &caption);
    bool resetPortCaption(PortType portType, PortIndex portIndex);
    void propagateUpdate();

    virtual std::shared_ptr<NodeData> inData(PortIndex const index);
    virtual std::unordered_map<QString, QString> getParameters() const;
    virtual std::unordered_map<QString, QMetaType::Type> getParameterSchema() const;
    virtual QStringList getParameterOptions(const QString &key) const;
    virtual void setParameter(const QString &key, const QString &value);
    uint nPorts(const PortType &portType, const QString &typeId) const;
    virtual uint minModifiablePorts(const PortType &portType, const QString &typeId) const;
    virtual bool portNumberModifiable(const PortType &portType) const { return false; };

    std::unordered_map<QString, QString> getExecutedValues() const { return m_executedValues; }
    void setExecutedValues(const std::unordered_map<QString, QString> &values);
    QStringList getExecutedGraphs() const { return m_executedGraphs; }
    void setExecutedGraphs(const QStringList &paths);
    virtual bool canConnect(ConnectionInfo &connInfo) const;

    template<typename T>
    std::vector<std::shared_ptr<T>> allOutData()
    {
        static_assert(std::is_base_of<NodeData, T>::value, "T must derive from NodeData");
        std::vector<std::shared_ptr<T>> result;
        for (auto &pair : m_outPorts)
            if (auto casted = std::dynamic_pointer_cast<T>(pair.first))
                result.push_back(casted);
        return result;
    }

signals:
    void captionUpdated(const QString &caption);
    void outPortCaptionUpdated(const PortIndex &index, const QString &caption);
    void outPortInserted(const PortIndex &index);
    void outPortDeleted(const PortIndex &index);

public slots:
    virtual void outputConnectionCreated(ConnectionId const &conn) override;
    virtual void outputConnectionDeleted(ConnectionId const &conn) override;
    virtual void setInputPortNumber(uint num);
    virtual void setOutputPortNumber(uint num);
    virtual void onFunctionInputSet(const PortIndex &index);
    virtual void onDataInputSet(const PortIndex &index);
    virtual void onFunctionInputReset(const PortIndex &index);
    virtual void onDataInputReset(const PortIndex &index);

protected:
    bool indexCheck(PortType type, PortIndex index) const;
    template<typename T>
    void addPort(PortType type, const QString &name = QString())
    {
        static_assert(std::is_base_of<NodeData, T>::value, "T must derive from NodeData");
        int index = type == PortType::In ? m_inPorts.size() : m_outPorts.size();
        emit portsAboutToBeInserted(type, index, index);
        if (type == PortType::In) {
            auto port = name.isEmpty() ? std::make_unique<T>() : std::make_unique<T>(name);
            m_inPorts.push_back({std::move(port), std::weak_ptr<NodeData>()});
        } else if (type == PortType::Out) {
            auto port = name.isEmpty() ? std::make_shared<T>() : std::make_shared<T>(name);
            m_outPorts.push_back({port, false});
        } else {
            qCritical() << "Unhandled type";
            return;
        }
        emit portsInserted();
        if (type == PortType::Out)
            emit outPortInserted(index);
        emit contentUpdated();
    }
    template<typename T>
    void removePort(PortType type)
    {
        static_assert(std::is_base_of<NodeData, T>::value, "T must derive from NodeData");
        int index = (type == PortType::In ? m_inPorts.size() : m_outPorts.size()) - 1;
        emit portsAboutToBeDeleted(type, index, index);
        if (type == PortType::In) {
            removePort<T, InPortType>(m_inPorts);
        } else if (type == PortType::Out) {
            removePort<T, OutPortType>(m_outPorts);
        } else {
            qCritical() << "Unhandled type";
            return;
        }
        emit portsDeleted();
        if (type == PortType::Out)
            emit outPortDeleted(index);
        emit contentUpdated();
    }
    template<typename NodeData, typename PortType>
    void removePort(PortType &ports)
    {
        for (int i = ports.size() - 1; i >= 0; --i)
            // check whether we are removing port of type T (Data, Function etc.), we might have both types together in "ports"
            if (dynamic_cast<NodeData *>(ports[i].first.get())) {
                ports.erase(ports.begin() + i);
                break;
            }
    }
    template<typename T>
    void setPortNumber(PortType type, uint num)
    {
        static_assert(std::is_base_of<NodeData, T>::value, "T must derive from NodeData");
        uint current = nPorts(type, constants::DATA_PORT_ID);
        if (current > num)
            for (int i = 0; i < current - num; ++i)
                removePort<T>(type);
        else if (current < num)
            for (int i = 0; i < num - current; ++i)
                addPort<T>(type);
    }
    template<typename T>
    std::shared_ptr<T> castedPort(PortType type, PortIndex index)
    {
        static_assert(std::is_base_of<NodeData, T>::value, "T must derive from NodeData");
        if (type == PortType::In) {
            if (auto shared = std::dynamic_pointer_cast<T>(m_inPorts.at(index).second.lock()))
                return shared;
        } else if (type == PortType::Out) {
            if (auto shared = std::dynamic_pointer_cast<T>(m_outPorts.at(index).first))
                return shared;
        }
        return nullptr;
    }
    // show warning for invalid port connection (implicit typing failure)
    bool warnInvalidConnection(ConnectionInfo connInfo, const QString &message) const;
    Q_INVOKABLE bool showWarning(ConnectionInfo connInfo, const QString &message);

private:
    using InPortType = std::vector<std::pair<std::unique_ptr<NodeData>, std::weak_ptr<NodeData>>>;
    // port -> in use
    using OutPortType = std::vector<std::pair<std::shared_ptr<NodeData>, bool>>;

    void updateStyle();
    void updateShape();

    const std::unordered_map<FdfType, QString> TYPE_STRING = {
        {FdfType::Coder, "coder"},
        {FdfType::Processor, "processor"},
        {FdfType::Trainer, "trainer"},
        {FdfType::Data, "data"},
        {FdfType::Output, "output"},
    };

    FdfType m_type;
    // name is used for identification in QtNodes library
    QString m_name;
    NodeShape m_shape;
    QString m_functionName;
    // caption is the label of the block, we regard this as the "name" for kedro
    QString m_caption;
    // first is for structuring, second is actual data linked to connected block
    InPortType m_inPorts;
    // first is for data, second represents whether it's in use
    OutPortType m_outPorts;
    std::unordered_map<QString, QString> m_executedValues;
    QStringList m_executedGraphs;
    QPointer<QLabel> m_label; // For block resize
};
