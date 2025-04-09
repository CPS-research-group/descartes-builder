#include "ui/models/io_models.hpp"
#include "data/tab_manager.hpp"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

std::unordered_map<CatalogType, QString> CATALOG_STRING = {
    {CatalogType::Pickle, "pickle.PickleDataset"},
    {CatalogType::Csv, "pandas.CSVDataset"},
    {CatalogType::H5, "kedro_umbrella.library.H5Dataset"},
};

std::unordered_map<QString, CatalogType> CATALOG_EXTENSIONS = {
    {"csv", CatalogType::Csv},
    {"pickle", CatalogType::Pickle},
    {"mat", CatalogType::H5},
    {"jld2", CatalogType::H5},
};

} // namespace

DataSourceModel::DataSourceModel()
    : FdfBlockModel(FdfType::Data, io_names::DATA_SOURCE)
    , m_widget(nullptr)
    , m_label(nullptr)
{
    addPort<DataNode>(PortType::Out);
    auto uidManager = TabManager::instance().getCurrentUIDManager();
    if (!uidManager) {
        qWarning() << "UIDManager is null!";
        return;
    }

    for (auto &port : allOutData<DataNode>())
        port->setTypeId(uidManager->createUID());
}

QWidget *DataSourceModel::embeddedWidget()
{
    if (!m_widget) {
        m_widget = new QWidget();
        m_widget->setProperty("bg", "transparent");
        m_widget->setStyleSheet("*[bg='transparent']{ background: transparent; }");

        auto layout = new QVBoxLayout(m_widget);
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);

        m_label = new QLabel(portCaption(PortType::Out, 0));
        layout->addWidget(m_label);

        auto button = new QPushButton("Import");
        layout->addWidget(button);

        connect(button, &QPushButton::clicked, this, &DataSourceModel::importClicked);
    }

    return m_widget;
}

QJsonObject DataSourceModel::save() const
{
    QJsonObject modelJson = FdfBlockModel::save();
    modelJson["data-name"] = m_file.fileName();
    return modelJson;
}

void DataSourceModel::load(QJsonObject const &p)
{
    FdfBlockModel::load(p);
    QJsonValue value = p["data-name"];

    if (value.isUndefined())
        return;

    setFile(QFileInfo(value.toString()));
}

QString DataSourceModel::fileTypeString() const
{
    if (m_fileType && CATALOG_STRING.count(m_fileType.value()) > 0)
        return CATALOG_STRING.at(m_fileType.value());
    return "NONE";
}

void DataSourceModel::setFile(const QFileInfo &file)
{
    if (file == m_file)
        return;
    m_file = file;
    if (CATALOG_EXTENSIONS.count(m_file.suffix()) > 0)
        m_fileType = CATALOG_EXTENSIONS.at(m_file.suffix());
    if (m_label)
        m_label->setText(m_file.fileName());
    updatePortCaption(m_file.baseName());
    emit contentUpdated();
}

QString DataSourceModel::fileFilter()
{
    QStringList extensions;
    for (auto &pair : CATALOG_EXTENSIONS)
        extensions << "*." + pair.first;
    return extensions.join(' ');
}

void DataSourceModel::updatePortCaption(const QString &name)
{
    if (name == portCaption(PortType::Out, 0))
        return;
    setPortCaption(PortType::Out, 0, name);
}

FuncOutModel::FuncOutModel()
    : FdfBlockModel(FdfType::Output, io_names::FUNC_OUT)
    , m_fileType(CatalogType::Pickle)
{
    addPort<FunctionNode>(PortType::In);
}

QString FuncOutModel::fileTypeString() const
{
    if (CATALOG_STRING.count(m_fileType) > 0)
        return CATALOG_STRING.at(m_fileType);
    return "NONE";
}

QString FuncOutModel::getFileName() const
{
    return portCaption(PortType::In, 0);
}

QString FuncOutModel::getFileExtenstion() const
{
    for (auto &pair : CATALOG_EXTENSIONS)
        if (pair.second == m_fileType)
            return pair.first;
    return QString();
}

DataOutModel::DataOutModel()
    : FdfBlockModel(FdfType::Output, io_names::DATA_OUT)
    , m_fileType(CatalogType::Pickle)
{
    addPort<DataNode>(PortType::In);
}

QString DataOutModel::fileTypeString() const
{
    if (CATALOG_STRING.count(m_fileType) > 0)
        return CATALOG_STRING.at(m_fileType);
    return "NONE";
}

QString DataOutModel::getFileName() const
{
    return portCaption(PortType::In, 0);
}

QString DataOutModel::getFileExtenstion() const
{
    for (auto &pair : CATALOG_EXTENSIONS)
        if (pair.second == m_fileType)
            return pair.first;
    return QString();
}

GraphModel::GraphModel()
    : FdfBlockModel(FdfType::Output, io_names::GRAPH_FUNCTION)
    , m_graph(nullptr)
{
    addPort<FunctionNode>(PortType::In, "f");
}

void GraphModel::setFile(const QFileInfo &file)
{
    if (file == m_file)
        return;
    m_file = file;
    updateGraph();
}

QWidget *GraphModel::embeddedWidget()
{
    if (!m_graph) {
        m_graph = new QLabel;
        m_graph->setStyleSheet("QLabel{ background: transparent; }");
        if (!m_file.absoluteFilePath().isEmpty())
            m_graph->setPixmap(QPixmap(m_file.absoluteFilePath()));
    }
    return m_graph;
}

void GraphModel::updateGraph()
{
    if (!m_graph)
        return;
    m_graph->setPixmap(QPixmap(m_file.absoluteFilePath()));
    emit contentUpdated();
}
