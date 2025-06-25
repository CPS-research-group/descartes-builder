#include "ui/models/coder_models.hpp"
#include "data/tab_manager.hpp"

#include "ui/models/function_names.hpp"

namespace {
using Process = CoderModel::Process;
std::unordered_map<Process, QString> REDUCE_STRING = {
    {Process::Pca, "pca"},
    {Process::PcaStd, "pca_std"},
    {Process::StdPca, "std_pca"},
};
std::unordered_map<Process, QString> TRANSFORM_STRING = {
    {Process::None, "none"},
    {Process::Std, "std"},
};
} // namespace

CoderModel::CoderModel(const QString &name, const QString &functionName)
    : FdfBlockModel(FdfType::Coder, name, functionName)
{}

std::unordered_map<QString, QMetaType::Type> CoderModel::getParameterSchema() const
{
    std::unordered_map<QString, QMetaType::Type> schema;
    schema[PROCESS] = QMetaType::QString;
    schema[RANDOM_STATE] = QMetaType::Int;
    return schema;
}

bool CoderModel::portNumberModifiable(const PortType &portType) const
{
    return portType == PortType::In;
}

uint CoderModel::minModifiablePorts(const PortType &portType, const QString &typeId) const
{
    if (portType == PortType::In)
        if (typeId == constants::DATA_PORT_ID)
            return 1;
    if (portType == PortType::Out)
        if (typeId == constants::FUNCTION_PORT_ID)
            return 1;
    return 0;
}

void CoderModel::setInputPortNumber(uint num)
{
    setPortNumber<DataNode>(PortType::In, num);
}

TransformDataModel::TransformDataModel()
    : CoderModel("transform", coder_function::TRANSFORM_DATA)
{
    addPort<DataNode>(PortType::In);
    addPort<FunctionNode>(PortType::Out, "encode");
    addPort<FunctionNode>(PortType::Out, "decode");
    setProcess(Process::Std);
    setRandomState(0);
}

std::unordered_map<QString, QString> TransformDataModel::getParameters() const
{
    std::unordered_map<QString, QString> result;
    result[PROCESS] = TRANSFORM_STRING.at(m_process);
    if (m_randomState)
        result[RANDOM_STATE] = QString::number(m_randomState.value());
    return result;
}

QStringList TransformDataModel::getParameterOptions(const QString &key) const
{
    QStringList result;
    if (key == PROCESS) {
        for (auto pair : TRANSFORM_STRING)
            result << pair.second;
    }
    return result;
}

void TransformDataModel::setParameter(const QString &key, const QString &value)
{
    if (key == PROCESS) {
        for (auto pair : TRANSFORM_STRING)
            if (pair.second == value)
                setProcess(pair.first);
    } else if (key == RANDOM_STATE) {
        setRandomState(value.toInt());
    }
}

std::vector<FdfUID> TransformDataModel::fetchOrCreateOutputType(
    const std::vector<FdfUID> &inputTypeIds)
{
    auto uidManager = TabManager::getUIDManager();

    // For any given inputs of types 't1, t2, t3', any two xforms
    // shall produce functions that generate fresh new output types with
    // the same dimensionality as inputs (tnew, tnew, tnew) and (tnew1, tnew2, tnew3)
    auto it = m_typeMap.find(inputTypeIds);
    if (it != m_typeMap.end())
        return it->second;
    std::vector<FdfUID> outputTypeIds;
    for (const auto &inputTypeId : inputTypeIds) {
        auto outputType = uidManager->createUID();
        outputTypeIds.push_back(outputType);
    }
    m_typeMap[inputTypeIds] = outputTypeIds;
    return outputTypeIds;
}

// Set the inputs to a particular index
void TransformDataModel::onDataInputSet(const PortIndex &index)
{
    Q_UNUSED(index);
    auto uidManager = TabManager::getUIDManager();

    std::vector<FdfUID> inputTypeIds, outputTypeIds;
    for (int i = 0; i < nPorts(PortType::In); ++i)
        if (auto data = castedPort<DataNode>(PortType::In, i)) {
            inputTypeIds.push_back(data->typeId());
        }
    outputTypeIds = fetchOrCreateOutputType(inputTypeIds);
    // Set encode/decode function type
    Signature signature;
    signature.inputs = inputTypeIds;
    signature.outputs = outputTypeIds;
    if (auto encode = castedPort<FunctionNode>(PortType::Out, 0))
        encode->setSignature(signature);
    // set decode to the inverse
    signature.inverse();
    if (auto decode = castedPort<FunctionNode>(PortType::Out, 1))
        decode->setSignature(signature);
}

void TransformDataModel::onDataInputReset(const PortIndex &index)
{
    onDataInputSet(index);
}

ReduceDataModel::ReduceDataModel()
    : CoderModel("reduce", coder_function::REDUCE_DATA)
{
    addPort<DataNode>(PortType::In);
    addPort<FunctionNode>(PortType::Out, "reduce");
    addPort<FunctionNode>(PortType::Out, "inv_reduce");
    setProcess(Process::StdPca);
    setRandomState(0);
}

std::unordered_map<QString, QString> ReduceDataModel::getParameters() const
{
    std::unordered_map<QString, QString> result;
    result[PROCESS] = REDUCE_STRING.at(m_process);
    if (m_randomState)
        result[RANDOM_STATE] = QString::number(m_randomState.value());
    return result;
}

QStringList ReduceDataModel::getParameterOptions(const QString &key) const
{
    QStringList result;
    if (key == PROCESS) {
        for (auto pair : REDUCE_STRING)
            result << pair.second;
    }
    return result;
}

void ReduceDataModel::setParameter(const QString &key, const QString &value)
{
    if (key == PROCESS) {
        for (auto pair : REDUCE_STRING)
            if (pair.second == value)
                setProcess(pair.first);
    } else if (key == RANDOM_STATE) {
        setRandomState(value.toInt());
    }
}

std::vector<FdfUID> ReduceDataModel::fetchOrCreateOutputType(const std::vector<FdfUID> &inputTypeIds)
{
    auto uidManager = TabManager::getUIDManager();

    // For any given inputs of types 't1, t2, t3', any two reducers
    // shall produce functions that generate fresh new output types (tnew) and (tnew2)
    auto it = m_typeMap.find(inputTypeIds);
    if (it != m_typeMap.end())
        return it->second;
    FdfUID outputType = uidManager->createUID();
    m_typeMap[inputTypeIds] = {outputType};
    return {outputType};
}

// Set the inputs to a particular index
void ReduceDataModel::onDataInputSet(const PortIndex &index)
{
    Q_UNUSED(index);

    // Save input type
    std::vector<FdfUID> inputTypeIds;
    for (int i = 0; i < nPorts(PortType::In); ++i)
        if (auto data = castedPort<DataNode>(PortType::In, i))
            inputTypeIds.push_back(data->typeId());

    auto uidManager = TabManager::getUIDManager();
    // Set encode/decode function type
    Signature signature;
    signature.inputs = inputTypeIds;
    signature.outputs = fetchOrCreateOutputType(inputTypeIds);
    if (auto encode = castedPort<FunctionNode>(PortType::Out, 0))
        encode->setSignature(signature);
    // set decode to the inverse
    signature.inverse();
    if (auto decode = castedPort<FunctionNode>(PortType::Out, 1))
        decode->setSignature(signature);
}

void ReduceDataModel::onDataInputReset(const PortIndex &index)
{
    onDataInputSet(index);
}
