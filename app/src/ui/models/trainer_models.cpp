#include "ui/models/trainer_models.hpp"
#include "ui/models/function_names.hpp"
namespace {
using Model = BasicTrainerModel::Model;
std::unordered_map<Model, QString> MODEL_STRING = {
    {Model::Mlp, "mlp"},
    {Model::Mlp1, "mlp1"},
    {Model::Mlp2, "mlp2"},
    {Model::Lr, "lr"},
    {Model::Dt, "dt"},
    {Model::Svr, "svr"},
};
} // namespace

TrainerModel::TrainerModel(const QString &name, const QString &functionName)
    : FdfBlockModel(FdfType::Trainer, name, functionName)
    , m_signature({{UIDManager::NONE_ID}, {UIDManager::NONE_ID}})
{}

bool TrainerModel::portNumberModifiable(const PortType &portType) const
{
    return portType == PortType::In;
}

uint TrainerModel::minModifiablePorts(const PortType &portType, const QString &typeId) const
{
    if (portType == PortType::In)
        if (typeId == constants::DATA_PORT_ID)
            return 2;
    if (portType == PortType::Out)
        if (typeId == constants::FUNCTION_PORT_ID)
            return 1;
    return 0;
}

void TrainerModel::setInputPortNumber(uint num)
{
    setPortNumber<DataNode>(PortType::In, num);
}

void TrainerModel::setTrainerInputNumber(uint num)
{
    if (m_signature.inputs.size() == num)
        return;
    m_signature.inputs.resize(num);
    setInputPortNumber(m_signature.inputs.size() + m_signature.outputs.size());
    updateSignature();
}

void TrainerModel::setTrainerOutputNumber(uint num)
{
    if (m_signature.outputs.size() == num)
        return;
    m_signature.outputs.resize(num);
    setInputPortNumber(m_signature.inputs.size() + m_signature.outputs.size());
    updateSignature();
}

void TrainerModel::onDataInputSet(const PortIndex &index)
{
    Q_UNUSED(index);
    updateSignature();
}

void TrainerModel::onDataInputReset(const PortIndex &index)
{
    m_signature.update(index, UIDManager::NONE_ID);
    if (auto function = castedPort<FunctionNode>(PortType::Out, 0))
        function->setSignature(m_signature);
}

void TrainerModel::updateSignature()
{
    std::vector<FdfUID> inputTypeIds;
    for (PortIndex i = 0; i < nPorts(PortType::In); ++i) {
        FdfUID typeId = UIDManager::NONE_ID;
        if (auto data = castedPort<DataNode>(PortType::In, i))
            typeId = data->typeId();
        m_signature.update(i, typeId);
    }
    if (auto function = castedPort<FunctionNode>(PortType::Out, 0))
        function->setSignature(m_signature);
}

BasicTrainerModel::BasicTrainerModel()
    : TrainerModel("trainer", trainer_function::BASIC_TRAINER)
{
    addPort<DataNode>(PortType::In);
    addPort<DataNode>(PortType::In);
    addPort<FunctionNode>(PortType::Out, "predict");

    setModel(Model::Mlp);
    setRandomState(0);
}

std::unordered_map<QString, QString> BasicTrainerModel::getParameters() const
{
    std::unordered_map<QString, QString> result;
    result[MODEL] = MODEL_STRING.at(m_model);
    if (m_randomState)
        result[RANDOM_STATE] = QString::number(m_randomState.value());
    if (m_model == Model::Mlp2 && m_hiddenLayerSizes) {
        QStringList sizes;
        for (auto &size : m_hiddenLayerSizes.value()) {
            sizes << QString::number(size);
        }
        QString input_string = sizes.join(", ");
        QString formatted_string = "[" + input_string + "]"; // Kedro expects [size1, size2, ...]
        result[HIDDEN_LAYER_SIZES] = formatted_string;
    }
    return result;
}

std::unordered_map<QString, QMetaType::Type> BasicTrainerModel::getParameterSchema() const
{
    std::unordered_map<QString, QMetaType::Type> schema;
    schema[RANDOM_STATE] = QMetaType::Int;
    schema[MODEL] = QMetaType::QString;
    schema[HIDDEN_LAYER_SIZES] = QMetaType::QVector2D;
    return schema;
}

QStringList BasicTrainerModel::getParameterOptions(const QString &key) const
{
    QStringList result;
    if (key == MODEL) {
        for (auto pair : MODEL_STRING)
            result << pair.second;
    }
    return result;
}

void BasicTrainerModel::setParameter(const QString &key, const QString &value)
{
    if (key == RANDOM_STATE)
        setRandomState(value.toInt());
    else if (key == MODEL) {
        for (auto pair : MODEL_STRING)
            if (pair.second == value) {
                setModel(pair.first);
                break;
            }
    } else if (key == HIDDEN_LAYER_SIZES) {
        std::vector<int> result;
        // For compatibility with kedro expected format
        auto formattedValue = value;
        auto sizes = formattedValue.replace("[", "").replace("]", "").replace(" ", "").split(",");
        for (auto size : sizes)
            result.push_back(size.toInt());
        setHiddenLayerSizes(result);
    }
}

void BasicTrainerModel::setModel(const Model &model)
{
    if (m_model == model)
        return;
    m_model = model;
    if (m_model == Model::Mlp2) {
        if (!m_hiddenLayerSizes)
            m_hiddenLayerSizes = std::vector<int>();
    } else if (m_hiddenLayerSizes)
        m_hiddenLayerSizes = std::nullopt;
    emit contentUpdated();
}

TorchTrainerModel::TorchTrainerModel()
    : TrainerModel("pytorch_trainer", trainer_function::PYTORCH_TRAINER)
{
    addPort<DataNode>(PortType::In);
    addPort<DataNode>(PortType::In);
    addPort<FunctionNode>(PortType::Out, "model");
}

std::unordered_map<QString, QString> TorchTrainerModel::getParameters() const
{
    std::unordered_map<QString, QString> result;
    result[RANDOM_STATE] = QString::number(m_randomState);
    result[MAX_ITER] = QString::number(m_maxIter);
    result[LEARNING_RATE] = QString::number(m_learningRate);
    return result;
}

std::unordered_map<QString, QMetaType::Type> TorchTrainerModel::getParameterSchema() const
{
    std::unordered_map<QString, QMetaType::Type> schema;
    schema[RANDOM_STATE] = QMetaType::Int;
    schema[MAX_ITER] = QMetaType::Int;
    schema[LEARNING_RATE] = QMetaType::Double;
    return schema;
}

void TorchTrainerModel::setParameter(const QString &key, const QString &value)
{
    if (key == RANDOM_STATE) {
        m_randomState = value.toInt();
    } else if (key == MAX_ITER) {
        m_maxIter = value.toInt();
    } else if (key == LEARNING_RATE) {
        m_learningRate = value.toDouble();
    }
}
