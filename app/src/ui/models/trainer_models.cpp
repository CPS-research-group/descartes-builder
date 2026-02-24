#include "ui/models/trainer_models.hpp"
#include "data/parameter_utils.hpp"
#include "ui/models/function_names.hpp"
namespace {
using Model = BasicTrainerModel::Model;
std::unordered_map<Model, QString> MODEL_UI_STRING = {
    {Model::Mlp, "Multi-layer Perceptron"},
    {Model::Lr, "Linear Regression"},
    {Model::Dt, "Decision Tree"},
    {Model::Svr, "Support Vector Regression"},
};

std::unordered_map<Model, QString> MODEL_STRING = {
    {Model::Mlp, "mlp"},
    {Model::Lr, "lr"},
    {Model::Dt, "dt"},
    {Model::Svr, "svr"},
};
} // namespace

TrainerModel::TrainerModel(const QString &name, const QString &functionName)
    : FdfBlockModel(FdfType::Trainer, name, functionName)
    , m_signature({{UIDManager::NONE_ID}, {UIDManager::NONE_ID}})
{
    m_defaultTags = {"X", "Y"};
}

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
    // reset the unconnected data input to the placeholder text
    if (auto inPort = getInputPortAt<DataNode>(index))
        inPort->setPlaceHolderCaption(m_defaultTags[index], "");

    m_signature.update(index, UIDManager::NONE_ID);
    if (auto function = castedPort<FunctionNode>(PortType::Out, 0))
        function->setSignature(m_signature);
}

void TrainerModel::updateSignature()
{
    for (PortIndex i = 0; i < nPorts(PortType::In); ++i) {
        if (auto data = castedPort<DataNode>(PortType::In, i)) {
            FdfUID typeId = data->typeId();
            m_signature.update(i, typeId);
        }
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

    // Initialise the input data ports with placeholder captions
    for (int i = 0; i < 2; ++i) {
        if (auto port = getInputPortAt<DataNode>(i)) {
            port->setPlaceHolderCaption(m_defaultTags[i], "");
        }
    }

    setModel(Model::Mlp);
}

std::unordered_map<QString, QString> BasicTrainerModel::getParameters() const
{
    std::unordered_map<QString, QString> result;
    result[MODEL] = MODEL_STRING.at(m_model);
    result[MODEL_UI] = MODEL_UI_STRING.at(m_model);
    auto rs = getRandomState();
    result[RANDOM_STATE] = rs ? QString::number(*rs) : QString::number(0);
    if (m_model == Model::Mlp && m_hiddenLayerSizes) {
        result[HIDDEN_LAYER_SIZES] = vectorToString(*m_hiddenLayerSizes);
    }
    return result;
}

std::unordered_map<QString, QMetaType::Type> BasicTrainerModel::getParameterSchema() const
{
    std::unordered_map<QString, QMetaType::Type> schema;
    schema[MODEL_UI] = QMetaType::QString;
    schema[HIDDEN_LAYER_SIZES] = QMetaType::QVariantList;
    return schema;
}

QStringList BasicTrainerModel::getParameterOptions(const QString &key) const
{
    QStringList result;
    if (key == MODEL_UI) {
        for (auto pair : MODEL_UI_STRING)
            result << pair.second;
    }
    return result;
}

void BasicTrainerModel::setParameter(const QString &key, const QString &value)
{
    if (key == MODEL_UI) {
        for (auto pair : MODEL_UI_STRING)
            if (pair.second == value) {
                setModel(pair.first);
                break;
            }
    } else if (key == HIDDEN_LAYER_SIZES) {
        setHiddenLayerSizes(stringToVector(value));
    }
}

void BasicTrainerModel::setModel(const Model &model)
{
    if (m_model == model)
        return;
    m_model = model;
    if (m_model == Model::Mlp) {
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

    // Initialise the input data ports with placeholder captions
    for (int i = 0; i < 2; ++i) {
        if (auto port = getInputPortAt<DataNode>(i)) {
            port->setPlaceHolderCaption(m_defaultTags[i], "");
        }
    }
}

std::unordered_map<QString, QString> TorchTrainerModel::getParameters() const
{
    std::unordered_map<QString, QString> result;
    auto rs = getRandomState();
    result[RANDOM_STATE] = rs ? QString::number(*rs) : QString::number(0);
    result[MAX_ITER] = QString::number(m_maxIter);
    result[LEARNING_RATE] = QString::number(m_learningRate);
    return result;
}

std::unordered_map<QString, QMetaType::Type> TorchTrainerModel::getParameterSchema() const
{
    std::unordered_map<QString, QMetaType::Type> schema;
    schema[MAX_ITER] = QMetaType::Int;
    schema[LEARNING_RATE] = QMetaType::Double;
    return schema;
}

void TorchTrainerModel::setParameter(const QString &key, const QString &value)
{
    if (key == MAX_ITER) {
        m_maxIter = value.toInt();
    } else if (key == LEARNING_RATE) {
        m_learningRate = value.toDouble();
    }
}
