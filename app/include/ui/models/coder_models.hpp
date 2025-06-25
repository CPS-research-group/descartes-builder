#pragma once

#include "fdf_block_model.hpp"

class CoderModel : public FdfBlockModel
{
    Q_OBJECT
public:
    enum Process {
        None,
        Std,
        Pca,
        PcaStd,
        StdPca,
    };
    inline static const QString PROCESS = "data_xform";
    inline static const QString RANDOM_STATE = "random_state";

    CoderModel(const QString &name, const QString &functionName);
    virtual std::unordered_map<QString, QMetaType::Type> getParameterSchema() const override;
    virtual bool portNumberModifiable(const PortType &portType) const override;
    virtual uint minModifiablePorts(const PortType &portType, const QString &typeId) const override;
    virtual std::vector<FdfUID> fetchOrCreateOutputType(const std::vector<FdfUID> &inputTypeIds) = 0;

protected:
    // A given set of input types should always return the same output type
    std::map<std::vector<FdfUID>, std::vector<FdfUID>> m_typeMap;

public slots:
    virtual void setInputPortNumber(uint num) override;
    // Implicit Typing handling
};

class TransformDataModel : public CoderModel
{
    Q_OBJECT
public:
    TransformDataModel();
    virtual std::unordered_map<QString, QString> getParameters() const override;
    virtual QStringList getParameterOptions(const QString &key) const override;
    virtual void setParameter(const QString &key, const QString &value) override;
    virtual void onDataInputSet(const PortIndex &index) override;
    virtual void onDataInputReset(const PortIndex &index) override;
    std::vector<FdfUID> fetchOrCreateOutputType(const std::vector<FdfUID> &inputTypeIds) override;

private:
    Process m_process;
    std::optional<int> m_randomState;
    Process getProcess() const { return m_process; }
    std::optional<int> getRandomState() const { return m_randomState; }
    void setRandomState(const std::optional<int> &randomState) { m_randomState = randomState; }
    void setProcess(const Process &dataXform) { m_process = dataXform; }
};

class ReduceDataModel : public CoderModel
{
    Q_OBJECT
public:
    ReduceDataModel();
    virtual std::unordered_map<QString, QString> getParameters() const override;
    virtual QStringList getParameterOptions(const QString &key) const override;
    virtual void setParameter(const QString &key, const QString &value) override;
    virtual void onDataInputSet(const PortIndex &index) override;
    virtual void onDataInputReset(const PortIndex &index) override;
    std::vector<FdfUID> fetchOrCreateOutputType(const std::vector<FdfUID> &inputTypeIds) override;

private:
    Process m_process;
    std::optional<int> m_randomState;
    Process getProcess() const { return m_process; }
    std::optional<int> getRandomState() const { return m_randomState; }
    void setRandomState(const std::optional<int> &randomState) { m_randomState = randomState; }
    void setProcess(const Process &dataXform) { m_process = dataXform; }
};
