#pragma once

#include "fdf_block_model.hpp"

class TrainerModel : public FdfBlockModel
{
    Q_OBJECT
public:
    TrainerModel(const QString &name, const QString &functionName);
    virtual bool portNumberModifiable(const PortType &portType) const override;
    virtual uint minModifiablePorts(const PortType &portType, const QString &typeId) const override;
    uint getTrainerInputPortNum() const { return m_signature.inputs.size(); }
    uint getTrainerOutputPortNum() const { return m_signature.outputs.size(); }

public slots:
    virtual void setInputPortNumber(uint num) override;
    // TODO: improve how ports are added, need a way to add ports to the middle
    // current system will only add/remove ports to the end like a stack
    void setTrainerInputNumber(uint num);
    void setTrainerOutputNumber(uint num);
    virtual void onDataInputSet(const PortIndex &index) override;
    virtual void onDataInputReset(const PortIndex &index) override;

private:
    void updateSignature();

    Signature m_signature;
};

class BasicTrainerModel : public TrainerModel
{
    Q_OBJECT
public:
    enum Model {
        Mlp,
        Mlp1,
        Mlp2,
        Lr,
        Dt,
        Svr,
    };

    BasicTrainerModel();
    virtual std::unordered_map<QString, QString> getParameters() const override;
    virtual std::unordered_map<QString, QMetaType::Type> getParameterSchema() const override;
    virtual QStringList getParameterOptions(const QString &key) const override;
    virtual void setParameter(const QString &key, const QString &value) override;
    Model getModel() const { return m_model; }
    std::optional<int> getRandomState() const { return m_randomState; }
    std::optional<std::vector<int>> getHiddenLayerSizes() const { return m_hiddenLayerSizes; }
    void setModel(const Model &model);
    void setRandomState(const std::optional<int> &randomState) { m_randomState = randomState; }
    void setHiddenLayerSizes(const std::optional<std::vector<int>> &hiddenLayerSizes)
    {
        m_hiddenLayerSizes = hiddenLayerSizes;
    }

private:
    inline static const QString RANDOM_STATE = "random_state";
    inline static const QString MODEL = "model";
    inline static const QString HIDDEN_LAYER_SIZES = "hidden_layer_sizes";

    Model m_model;
    std::optional<int> m_randomState;
    // only for mlp2
    std::optional<std::vector<int>> m_hiddenLayerSizes;
};

class TorchTrainerModel : public TrainerModel
{
    Q_OBJECT
public:
    TorchTrainerModel();
    virtual bool hasParameters() const override { return true; }
    virtual std::unordered_map<QString, QString> getParameters() const override;
    virtual std::unordered_map<QString, QMetaType::Type> getParameterSchema() const override;
    virtual void setParameter(const QString &key, const QString &value) override;

private:
    inline static const QString RANDOM_STATE = "random_state";
    inline static const QString MAX_ITER = "max_iter";
    inline static const QString LEARNING_RATE = "learning_rate";

    int m_randomState = 0;
    int m_maxIter = 1000;
    double m_learningRate = 0.001;
};