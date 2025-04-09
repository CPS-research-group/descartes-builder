#include "ui/models/processor_models.hpp"
#include "data/tab_manager.hpp"
#include "ui/models/function_names.hpp"

namespace {
using Plot = ScoreModel::Plot;
std::unordered_map<Plot, QString> PLOT_STRING = {
    {Plot::Regression, "regression"},
    {Plot::TimeSeries, "time_series"},
};
} // namespace

ProcessorModel::ProcessorModel(const QString &name, const QString &functionName)
    : FdfBlockModel(FdfType::Processor, name, functionName)
{}

bool ProcessorModel::portNumberModifiable(const PortType &portType) const
{
    return true;
}

uint ProcessorModel::minModifiablePorts(const PortType &portType, const QString &typeId) const
{
    if (portType == PortType::In)
        if (typeId == constants::DATA_PORT_ID)
            return 1;
    if (portType == PortType::Out)
        if (typeId == constants::DATA_PORT_ID)
            return 1;
    return 0;
}

void ProcessorModel::setInputPortNumber(uint num)
{
    setPortNumber<DataNode>(PortType::In, num);
}

void ProcessorModel::setOutputPortNumber(uint num)
{
    setPortNumber<DataNode>(PortType::Out, num);
}

SplitDataModel::SplitDataModel()
    : ProcessorModel("split_data", processor_function::SPLIT_DATA)
{
    addPort<DataNode>(PortType::In, "X");
    addPort<DataNode>(PortType::In, "Y");
    addPort<DataNode>(PortType::Out, "X_train");
    addPort<DataNode>(PortType::Out, "X_test");
    addPort<DataNode>(PortType::Out, "Y_train");
    addPort<DataNode>(PortType::Out, "Y_test");

    setRandomState(0);
}

std::unordered_map<QString, QString> SplitDataModel::getParameters() const
{
    std::unordered_map<QString, QString> result;
    if (m_randomState)
        result[RANDOM_STATE] = QString::number(m_randomState.value());
    if (m_splitTime)
        result[SPLIT_TIME] = QString::number(m_splitTime.value());
    if (m_trainSize)
        result[TRAIN_SIZE] = QString::number(m_trainSize.value());
    return result;
}

std::unordered_map<QString, QMetaType::Type> SplitDataModel::getParameterSchema() const
{
    std::unordered_map<QString, QMetaType::Type> schema;
    schema[RANDOM_STATE] = QMetaType::Int;
    schema[SPLIT_TIME] = QMetaType::Int;
    schema[TRAIN_SIZE] = QMetaType::Double;
    return schema;
}

void SplitDataModel::setParameter(const QString &key, const QString &value)
{
    if (key == RANDOM_STATE) {
        setRandomState(value.toInt());
    } else if (key == SPLIT_TIME) {
        setSplitTime(value.toInt());
    } else if (key == TRAIN_SIZE) {
        setTrainSize(value.toDouble());
    }
}

void SplitDataModel::onDataInputSet(const PortIndex &index)
{
    FdfUID target;
    QString name;
    if (auto data = castedPort<DataNode>(PortType::In, index)) {
        target = data->typeId();
        name = data->name();
    }
    setOutputType(index, target, name);
}

void SplitDataModel::onDataInputReset(const PortIndex &index)
{
    setOutputType(index, UIDManager::NONE_ID, "");
}

void SplitDataModel::setOutputType(const PortIndex &inputIndex,
                                   const FdfUID &typeId,
                                   const QString &name)
{
    // hardcoded wiring from input to output
    std::unordered_map<PortIndex, std::vector<PortIndex>> relations = {{0, {0, 1}}, {1, {2, 3}}};
    for (auto &relatedIndex : relations.at(inputIndex)) {
        auto port = castedPort<DataNode>(PortType::Out, relatedIndex);
        port->setTypeId(typeId);
        if ((relatedIndex) % 2 == 0)
            port->setName(name + "_train");
        else
            port->setName(name + "_test");
    }
}

ExternalProcessorModel::ExternalProcessorModel()
    : ProcessorModel("process", processor_function::PROCESSOR)
{
    addPort<FunctionNode>(PortType::In);
}

bool ExternalProcessorModel::canConnect(ConnectionInfo &connInfo) const
{
    if (m_signature.isEmpty())
        return true;
    PortIndex index = connInfo.inIndex;
    if (index == 0)
        // function input, always allow functions to be connected,
        // may want to disconnect data after this
        return true;
    // note that the first input is the function from a previous block
    connInfo.expectedInType = m_signature.inputs.at(--index);
    if (connInfo.expectedInType != connInfo.receivedOutType) {
        return warnInvalidConnection(connInfo, constants::TYPE_MISMATCH);
    }
    return true;
}

void ExternalProcessorModel::onFunctionInputSet(const PortIndex &index)
{
    if (auto function = castedPort<FunctionNode>(PortType::In, 0))
        m_signature = function->signature();
    updateDataPortsWithSignature();
}

void ExternalProcessorModel::onFunctionInputReset(const PortIndex &index)
{
    m_signature = Signature();
    updateDataPortsWithSignature();
}

bool ExternalProcessorModel::portNumberModifiable(const PortType &portType) const
{
    // port number will be modified by function instead
    return false;
}

void ExternalProcessorModel::updateDataPortsWithSignature()
{
    setPortNumber<DataNode>(PortType::In, m_signature.inputs.size());
    // propagate the types from the function to the data port
    setPortNumber<DataNode>(PortType::Out, m_signature.outputs.size());
    for (int i = 0; i < m_signature.outputs.size(); ++i) {
        auto port = castedPort<DataNode>(PortType::Out, i);
        port->setTypeId(m_signature.outputs.at(i));
    }
}

ScoreModel::ScoreModel()
    : ProcessorModel("score", processor_function::SCORE)
{
    addPort<DataNode>(PortType::In, "Y_test");
    addPort<DataNode>(PortType::In, "Y_pred");
    addPort<DataNode>(PortType::Out, "nrmse");
    addPort<DataNode>(PortType::Out, "r2");

    setPlot(Plot::Regression);
    auto uidManager = TabManager::instance().getCurrentUIDManager();
    if (!uidManager) {
        qWarning() << "UIDManager is null!";
        return;
    }
    for (auto &port : allOutData<DataNode>()) {
        port->setTypeId(uidManager->createUID());
    }
}

std::unordered_map<QString, QString> ScoreModel::getParameters() const
{
    std::unordered_map<QString, QString> result;
    if (m_plot)
        result[PLOT] = PLOT_STRING.at(m_plot.value());
    return result;
}

std::unordered_map<QString, QMetaType::Type> ScoreModel::getParameterSchema() const
{
    std::unordered_map<QString, QMetaType::Type> schema;
    schema[PLOT] = QMetaType::QString;
    return schema;
}

QStringList ScoreModel::getParameterOptions(const QString &key) const
{
    QStringList result;
    if (key == PLOT) {
        for (auto pair : PLOT_STRING)
            result << pair.second;
    }
    return result;
}

void ScoreModel::setParameter(const QString &key, const QString &value) {}

bool ScoreModel::canConnect(ConnectionInfo &connInfo) const
{
    auto otherIndex = (connInfo.inIndex == 0) ? 1 : 0;
    // check if the other input is already set, if so, get its type
    if (auto otherInput = const_cast<ScoreModel *>(this)->castedPort<DataNode>(PortType::In,
                                                                               otherIndex)) {
        connInfo.expectedInType = otherInput->typeId();
        if (connInfo.expectedInType != connInfo.receivedOutType)
            return warnInvalidConnection(connInfo, constants::TYPE_MISMATCH);
    }
    return true;
}

DifferenceModel::DifferenceModel()
    : ProcessorModel("difference", processor_function::DIFFERENCE)
{
    addPort<DataNode>(PortType::In, "a");
    addPort<DataNode>(PortType::In, "b");
    addPort<DataNode>(PortType::Out, "a-b");
}

void DifferenceModel::onDataInputSet(const PortIndex &index)
{
    FdfUID target;
    if (auto data = castedPort<DataNode>(PortType::In, index)) {
        target = data->typeId();
        setOutputTypeId(0, target);
    }
}

void DifferenceModel::onDataInputReset(const PortIndex &index)
{
    //reset the output port id to NONE if both the inputs are reset
    PortIndex other = (index == 0) ? 1 : 0;
    if (!castedPort<DataNode>(PortType::In, other))
        setOutputTypeId(0, UIDManager::NONE_ID);
}

void DifferenceModel::setOutputTypeId(const PortIndex &inputIndex, const FdfUID &typeId)
{
    if (auto port = castedPort<DataNode>(PortType::Out, inputIndex))
        port->setTypeId(typeId);
}

bool DifferenceModel::canConnect(ConnectionInfo &connInfo) const
{
    auto otherIndex = (connInfo.inIndex == 0) ? 1 : 0;
    // check if the other input is already set, if so, get its type
    if (auto otherInput = const_cast<DifferenceModel *>(this)->castedPort<DataNode>(PortType::In,
                                                                                    otherIndex)) {
        connInfo.expectedInType = otherInput->typeId();
        if (connInfo.expectedInType != connInfo.receivedOutType)
            return warnInvalidConnection(connInfo, constants::TYPE_MISMATCH);
    }
    return true;
}

SensitivityAnalysisModel::SensitivityAnalysisModel()
    : ProcessorModel("sensitivity_analysis", processor_function::SENSITIVITY_ANALYSIS)
{
    addPort<FunctionNode>(PortType::In, "model");
}

std::unordered_map<QString, QString> SensitivityAnalysisModel::getParameters() const
{
    std::unordered_map<QString, QString> result;
    result[NUM_SAMPLE] = QString::number(m_numSample);
    result[TARGET] = QString::number(m_target);
    result[DIFF_STEP] = QString::number(m_diffStep);
    result[GRID_SIZE] = QString::number(m_gridSize);
    return result;
}

std::unordered_map<QString, QMetaType::Type> SensitivityAnalysisModel::getParameterSchema() const
{
    std::unordered_map<QString, QMetaType::Type> schema;
    schema[NUM_SAMPLE] = QMetaType::Int;
    schema[TARGET] = QMetaType::Int;
    schema[DIFF_STEP] = QMetaType::Int;
    schema[GRID_SIZE] = QMetaType::Int;
    return schema;
}

void SensitivityAnalysisModel::setParameter(const QString &key, const QString &value)
{
    if (key == NUM_SAMPLE) {
        m_numSample = value.toInt();
    } else if (key == TARGET) {
        m_target = value.toInt();
    } else if (key == DIFF_STEP) {
        m_diffStep = value.toInt();
    } else if (key == GRID_SIZE) {
        m_gridSize = value.toInt();
    }
}

void SensitivityAnalysisModel::onFunctionInputSet(const PortIndex &index)
{
    if (auto function = castedPort<FunctionNode>(PortType::In, 0))
        m_signature = function->signature();
    updateDataPortsWithSignature();
}

void SensitivityAnalysisModel::onFunctionInputReset(const PortIndex &index)
{
    m_signature = Signature();
    updateDataPortsWithSignature();
}

void SensitivityAnalysisModel::updateDataPortsWithSignature()
{
    // Set the total number of output ports based on inputs and outputs
    setPortNumber<DataNode>(PortType::Out,
                            (m_signature.inputs.size() + m_signature.outputs.size()) * 2);

    int outputPortIndex = 0;

    // Set the types of X1 and X2 ports with the trainer input type
    if (!m_signature.inputs.empty()) {
        FdfUID trainerInputType = m_signature.inputs[0];
        if (auto port = castedPort<DataNode>(PortType::Out, outputPortIndex))
            port->setTypeId(trainerInputType);
        if (auto port = castedPort<DataNode>(PortType::Out, outputPortIndex + 1))
            port->setTypeId(trainerInputType);
    }

    outputPortIndex += 2;

    // Set the types of Y1 and Y2 ports with the trainer output type
    if (!m_signature.outputs.empty()) {
        FdfUID trainerOutputType = m_signature.outputs[0];
        if (auto port = castedPort<DataNode>(PortType::Out, outputPortIndex))
            port->setTypeId(trainerOutputType);
        if (auto port = castedPort<DataNode>(PortType::Out, outputPortIndex + 1))
            port->setTypeId(trainerOutputType);
    }
}

void SensitivityAnalysisModel::setOutputTypeId(const PortIndex &inputIndex, const FdfUID &typeId)
{
    if (auto port = castedPort<DataNode>(PortType::Out, inputIndex))
        port->setTypeId(typeId);
}

bool SensitivityAnalysisModel::canConnect(ConnectionInfo &connInfo) const
{
    //check if the input function signature is singular (1 input, 1 output)
    if (connInfo.receivedSignature
        && (connInfo.receivedSignature->inputs.size() > 1
            || connInfo.receivedSignature->outputs.size() > 1)) {
        return warnInvalidConnection(connInfo, constants::SINGULAR_SIGNATURE);
    }
    return true;
}
