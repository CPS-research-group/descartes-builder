#pragma once

#include "fdf_block_model.hpp"

#include <QFileInfo>

#include <QtUtility/data/constexpr_qstring.hpp>

class QWidget;
class QLabel;

namespace io_names {
using ConstLatin1String = QtUtility::data::ConstLatin1String;
constexpr ConstLatin1String DATA_SOURCE = "data_source";
constexpr ConstLatin1String FUNC_OUT = "func_out";
constexpr ConstLatin1String DATA_OUT = "data_out";
constexpr ConstLatin1String GRAPH_FUNCTION = "graph_function";
} // namespace io_names

enum CatalogType { Pickle, Csv, H5 };

class DataSourceModel : public FdfBlockModel
{
    Q_OBJECT
public:
    DataSourceModel();
    QWidget *embeddedWidget() override;
    QJsonObject save() const override;
    void load(QJsonObject const &p) override;
    QFileInfo file() const { return m_file; }
    std::optional<CatalogType> fileType() const { return m_fileType; }
    QString fileTypeString() const;
    void setFile(const QFileInfo &file);
    static QString fileFilter();
    QString outPortCaption();

signals:
    void importClicked();

private:
    QWidget *m_widget;
    QLabel *m_label;

    // not the actual file path, using it for relative path
    QFileInfo m_file;
    std::optional<CatalogType> m_fileType;
};

class FuncOutModel : public FdfBlockModel
{
    Q_OBJECT
public:
    FuncOutModel();
    CatalogType getFileType() const { return m_fileType; }
    QString fileTypeString() const;
    void setFileType(const CatalogType &fileType) { m_fileType = fileType; }
    QString getFileName() const;
    QString getFileExtenstion() const;

private:
    CatalogType m_fileType;
};

class DataOutModel : public FdfBlockModel
{
    Q_OBJECT
public:
    DataOutModel();
    CatalogType getFileType() const { return m_fileType; }
    QString fileTypeString() const;
    void setFileType(const CatalogType &fileType) { m_fileType = fileType; }
    QString getFileName() const;
    QString getFileExtenstion() const;

private:
    CatalogType m_fileType;
};

class GraphModel : public FdfBlockModel
{
    Q_OBJECT
public:
    GraphModel();
    QFileInfo file() const { return m_file; }
    void setFile(const QFileInfo &file);
    QWidget *embeddedWidget() override;

private:
    void updateGraph();

    QLabel *m_graph;
    QFileInfo m_file;
};