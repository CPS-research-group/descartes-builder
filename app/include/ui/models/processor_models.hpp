#pragma once

#include "fdf_block_model.hpp"
#include "uid_manager.hpp"

class ProcessorModel : public FdfBlockModel
{
    Q_OBJECT
public:
    ProcessorModel(const QString &name, const QString &functionName);
    virtual bool portNumberModifiable(const PortType &portType) const override;
    virtual uint minModifiablePorts(const PortType &portType, const QString &typeId) const override;

public slots:
    virtual void setInputPortNumber(uint num) override;
    virtual void setOutputPortNumber(uint num) override;
};

class SplitDataModel : public ProcessorModel
{
    Q_OBJECT
public:
    SplitDataModel();
    // need to be true even if parameters is empty
    virtual bool hasParameters() const override { return true; }
    virtual std::unordered_map<QString, QString> getParameters() const override;
    virtual std::unordered_map<QString, QMetaType::Type> getParameterSchema() const override;
    virtual void setParameter(const QString &key, const QString &value) override;
    std::optional<int> getRandomState() const { return m_randomState; }
    std::optional<int> getSplitTime() const { return m_splitTime; }
    std::optional<int> getTrainSize() const { return m_trainSize; }
    void setRandomState(const std::optional<int> &randomState) { m_randomState = randomState; }
    void setSplitTime(const std::optional<int> &splitTime) { m_splitTime = splitTime; }
    void setTrainSize(const std::optional<double> &trainSize) { m_trainSize = trainSize; }

public slots:
    virtual void onDataInputSet(const PortIndex &index) override;
    virtual void onDataInputReset(const PortIndex &index) override;

private:
    // define the output type for a given input type
    void setOutputType(const PortIndex &inputIndex, const FdfUID &typeId, const QString &name);

    inline static const QString RANDOM_STATE = "random_state";
    inline static const QString SPLIT_TIME = "split_time";
    inline static const QString TRAIN_SIZE = "train_size";

    std::optional<int> m_randomState;
    std::optional<int> m_splitTime = 0;
    std::optional<double> m_trainSize = 0.0;
};

// Reusing a function for a predecessor node
class ExternalProcessorModel : public ProcessorModel
{
    Q_OBJECT
public:
    ExternalProcessorModel();
    virtual bool canConnect(ConnectionInfo &connInfo) const override;

public slots:
    virtual void onFunctionInputSet(const PortIndex &index) override;
    virtual void onFunctionInputReset(const PortIndex &index) override;
    virtual bool portNumberModifiable(const PortType &portType) const override;

private:
    void updateDataPortsWithSignature();
    Signature m_signature;
};

class ScoreModel : public ProcessorModel
{
    Q_OBJECT
public:
    enum Plot {
        Regression,
        TimeSeries,
    };

    ScoreModel();
    virtual std::unordered_map<QString, QString> getParameters() const override;
    virtual std::unordered_map<QString, QMetaType::Type> getParameterSchema() const override;
    virtual QStringList getParameterOptions(const QString &key) const override;
    virtual void setParameter(const QString &key, const QString &value) override;
    virtual bool canConnect(ConnectionInfo &connInfo) const override;

    std::optional<Plot> getPlot() const { return m_plot; }
    void setPlot(const std::optional<Plot> &plot) { m_plot = plot; }

private:
    inline static const QString PLOT = "plot";
    // TODO get rid of this funky optionals..
    std::optional<Plot> m_plot;
};

class SensitivityAnalysisModel : public ProcessorModel
{
    Q_OBJECT
public:
    SensitivityAnalysisModel();

    virtual bool hasParameters() const override { return true; }
    virtual std::unordered_map<QString, QString> getParameters() const override;
    virtual std::unordered_map<QString, QMetaType::Type> getParameterSchema() const override;
    virtual void setParameter(const QString &key, const QString &value) override;
    virtual void setOutputTypeId(const QtNodes::PortIndex &inputIndex, const FdfUID &typeId);
    virtual void onFunctionInputSet(const PortIndex &index) override;
    virtual void onFunctionInputReset(const PortIndex &index) override;
    void updateDataPortsWithSignature();
    virtual bool canConnect(ConnectionInfo &connInfo) const override;

private:
    inline static const QString NUM_SAMPLE = "num_sample";
    inline static const QString TARGET = "target";
    inline static const QString DIFF_STEP = "diff_step";
    inline static const QString GRID_SIZE = "grid_size";

    int m_numSample = 1;
    int m_target = 0;
    int m_diffStep = 10;
    int m_gridSize = 1000;
    Signature m_signature;
};

class DifferenceModel : public ProcessorModel
{
    Q_OBJECT
public:
    DifferenceModel();
    virtual void onDataInputSet(const PortIndex &index) override;
    virtual void onDataInputReset(const PortIndex &index);
    virtual void setOutputTypeId(const QtNodes::PortIndex &, const FdfUID &);
    virtual bool canConnect(ConnectionInfo &connInfo) const override;
};
