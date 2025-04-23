#include "ui/side_bar_widgets/blocks.hpp"
#include "ui/models/uid_manager.hpp"
#include <QComboBox>
#include <QFormLayout>
#include <QGraphicsItem>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QTreeWidget>
#include <QWidgetAction>

#include <QtNodes/DagGraphicsScene>
#include <QtNodes/GraphicsView>
#include <QtNodes/NodeDelegateModelRegistry>

#include <QtUtility/widgets/qcollapsible_widget.hpp>

#include "data/block_manager.hpp"
#include "data/constants.hpp"
#include "data/tab_manager.hpp"
#include "ui/models/fdf_block_model.hpp"
#include "ui/models/function_names.hpp"
#include "ui/models/trainer_models.hpp"

using QCollapsibleWidget = QtUtility::widgets::QCollapsibleWidget;

namespace {
constexpr uint ADD_BLOCK_SPACING = 20;
constexpr uint FUNCTION_ROW = 1;
constexpr uint INPUT_PORT_ROW = 3;
constexpr uint OUTPUT_PORT_ROW = 4;
constexpr uint TRAINER_INPUT_ROW = 5;
constexpr uint TRAINER_OUTPUT_ROW = 6;
constexpr uint PARAMETER_ROW = 7;
constexpr uint PORT_TYPE_MAP_ROW = 9;
} // namespace

Blocks::Blocks(std::shared_ptr<BlockManager> blockManager,
               std::shared_ptr<TabManager> tabManager,
               QWidget *parent)
    : QWidget(parent)
    , m_blockManager(blockManager)
    , m_tabManager(tabManager)
    , m_nodeId(QtNodes::InvalidNodeId)
    , m_blockEditor(new QCollapsibleWidget("Editor"))
    , m_idEdit(new QLineEdit)
    , m_captionEdit(new QLineEdit)
    , m_functionNameEdit(new QLineEdit)
    , m_inputPortEdit(new QSpinBox)
    , m_outputPorts(new QStackedWidget)
    , m_outputPortEdit(new QSpinBox)
    , m_trainerInputEdit(new QSpinBox)
    , m_trainerOutputEdit(new QSpinBox)
    , m_parametersWidget(new QStackedWidget)
    , m_library(new QCollapsibleWidget("Library"))
{
    initUi();

    connect(m_blockManager.get(), &BlockManager::nodeSelected, this, &Blocks::setNodeId);
}

void Blocks::setNodeId(QtNodes::NodeId id)
{
    if (m_nodeId == id)
        return;
    m_nodeId = id;
    emit nodeIdChanged(id);
}

void Blocks::updateFields()
{
    blockEditorSignals(true);
    auto block = m_blockManager->getBlock(m_nodeId);
    enableEditorWidgets(block);
    handleInputRows(block);
    if (auto widget = m_parametersWidget->currentWidget()) {
        m_parametersWidget->removeWidget(widget);
        widget->deleteLater();
    }
    if (auto widget = m_outputPorts->currentWidget()) {
        m_outputPorts->removeWidget(widget);
        widget->deleteLater();
    }
    if (!block) {
        m_idEdit->clear();
        m_captionEdit->clear();
        m_functionNameEdit->clear();
        m_inputPortEdit->clear();
        m_outputPortEdit->clear();
    } else {
        // fill the fields
        m_idEdit->setText(QString::number(m_nodeId));
        m_functionNameEdit->setText(block->functionName());
        QString sanitizedCaption = constants::sanitizeCaption(block->caption());
        m_captionEdit->setText(sanitizedCaption);
        m_inputPortEdit->setMinimum(
            block->minModifiablePorts(PortType::In, constants::DATA_PORT_ID));
        m_inputPortEdit->setValue(block->nPorts(PortType::In, constants::DATA_PORT_ID));
        m_inputPortEdit->setEnabled(block->portNumberModifiable(PortType::In));
        m_outputPortEdit->setMinimum(
            block->minModifiablePorts(PortType::Out, constants::DATA_PORT_ID));
        m_outputPortEdit->setValue(block->nPorts(PortType::Out, constants::DATA_PORT_ID));
        m_outputPortEdit->setEnabled(block->portNumberModifiable(PortType::Out));

        if (auto parameterWidget = generateParameterWidget(block))
            m_parametersWidget->addWidget(parameterWidget);
        if (auto outputWidget = generatePortsWidget(block))
            m_outputPorts->addWidget(outputWidget);
    }
    m_editorLayout->setRowVisible(FUNCTION_ROW, block && !block->functionName().isEmpty());
    m_editorLayout->setRowVisible(PARAMETER_ROW, m_parametersWidget->currentWidget());
    m_editorLayout->setRowVisible(PORT_TYPE_MAP_ROW, m_outputPorts->currentWidget());

    blockEditorSignals(false);
}

void Blocks::onLibraryItemClicked(QTreeWidgetItem *item)
{
    if (!(item->flags() & (Qt::ItemIsSelectable))) {
        return;
    }
    auto view = m_tabManager->getCurrentTab()->getView();
    auto pos = view->mapToScene(view->rect().center()).toPoint();
    auto scene = m_tabManager->getCurrentTab()->getScene();
    while (auto exisitingItem = scene->itemAt(pos, QTransform())) {
        // sometime will add to a previous location due to itemAt only return 1 item at the pos even though another item may also be there
        if (exisitingItem->pos() != pos)
            break;
        pos += QPoint(ADD_BLOCK_SPACING, ADD_BLOCK_SPACING);
    }
    scene->createNodeAt(item->text(0), pos);
}

void Blocks::onNodeUpdated(QtNodes::NodeId id)
{
    if (id != m_nodeId)
        return;
    updateFields();
}

void Blocks::initUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 0, 0, 0);

    initEditor();
    initLibrary();
    layout->addWidget(m_blockEditor);
    layout->addWidget(m_library);
}

void Blocks::initEditor()
{
    auto formWidget = new QWidget();
    m_editorLayout = new QFormLayout(formWidget);
    m_editorLayout->setContentsMargins(0, 0, 0, 0);

    m_editorLayout->addRow(new QLabel("Id:"), m_idEdit);
    m_idEdit->setDisabled(true);
    m_idEdit->setMaximumWidth(constants::INT_LINE_EDIT_MAXIMUM_WIDTH);

    m_editorLayout->addRow(new QLabel("Function:"), m_functionNameEdit);
    m_editorLayout->setRowVisible(FUNCTION_ROW, false);
    m_functionNameEdit->setDisabled(true);
    m_functionNameEdit->setMaximumWidth(constants::INT_LINE_EDIT_MAXIMUM_WIDTH);

    m_captionEdit->setMaximumWidth(constants::INT_LINE_EDIT_MAXIMUM_WIDTH);
    QPixmap warningPixmap = style()->standardPixmap(QStyle::SP_MessageBoxWarning);
    // to maintain height, as in Windows the default size is bigger
    int editHeight = m_captionEdit->sizeHint().height();
    QPixmap scaledIcon = warningPixmap.scaledToHeight(editHeight, Qt::SmoothTransformation);
    m_invalidIcon = new QLabel(formWidget); // tool tip for invalid caption
    m_invalidIcon->setPixmap(scaledIcon);
    const QString validCaption
        = "Caption must contain only letters, digits, hyphens, underscores, or fullstops.";
    m_invalidIcon->setToolTip(validCaption);
    m_invalidIcon->setVisible(false);

    auto captionLayout = new QHBoxLayout();
    captionLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    captionLayout->addWidget(m_captionEdit);
    captionLayout->addWidget(m_invalidIcon);
    captionLayout->setContentsMargins(0, 0, 0, 0);
    auto captionContainer = new QWidget(formWidget);
    captionContainer->setLayout(captionLayout);
    QLabel *captionLabel = new QLabel("Caption:");
    captionLabel->setToolTip(validCaption);
    captionLabel->setCursor(Qt::PointingHandCursor);
    m_editorLayout->addRow(captionLabel, captionContainer);

    m_inputPortEdit->setRange(0, constants::MAX_DATA_INPUT_PORTS);
    m_inputPortEdit->setMaximumWidth(constants::INT_SPIN_BOX_MAX_WIDTH);
    m_inputPortEdit->setMinimumWidth(constants::INT_SPIN_BOX_MIN_WIDTH);
    m_outputPortEdit->setRange(0, constants::MAX_DATA_OUTPUT_PORTS);
    m_outputPortEdit->setMaximumWidth(constants::INT_SPIN_BOX_MAX_WIDTH);
    m_outputPortEdit->setMinimumWidth(constants::INT_SPIN_BOX_MIN_WIDTH);
    m_editorLayout->addRow(new QLabel("Input Ports:"), m_inputPortEdit);
    m_editorLayout->addRow(new QLabel("Output Ports:"), m_outputPortEdit);

    m_trainerInputEdit->setRange(1, constants::MAX_DATA_INPUT_PORTS);
    m_trainerInputEdit->setMaximumWidth(constants::INT_SPIN_BOX_MAX_WIDTH);
    m_trainerInputEdit->setMinimumWidth(constants::INT_SPIN_BOX_MIN_WIDTH);
    m_trainerOutputEdit->setRange(1, constants::MAX_DATA_OUTPUT_PORTS);
    m_trainerOutputEdit->setMaximumWidth(constants::INT_SPIN_BOX_MAX_WIDTH);
    m_trainerOutputEdit->setMinimumWidth(constants::INT_SPIN_BOX_MIN_WIDTH);
    m_editorLayout->addRow(new QLabel("Trainer Inputs:"), m_trainerInputEdit);
    m_editorLayout->addRow(new QLabel("Trainer Outputs:"), m_trainerOutputEdit);
    m_editorLayout->setRowVisible(TRAINER_INPUT_ROW, false);
    m_editorLayout->setRowVisible(TRAINER_OUTPUT_ROW, false);

    m_editorLayout->addRow(new QLabel("Parameters"));
    m_editorLayout->setRowVisible(PARAMETER_ROW, false);
    m_editorLayout->addRow(m_parametersWidget);
    m_editorLayout->addRow(new QLabel("Ports-Type Map:"));
    m_editorLayout->setRowVisible(PORT_TYPE_MAP_ROW, false);
    m_editorLayout->addRow(m_outputPorts);
    m_blockEditor->setWidget(formWidget);

    // these will be blocked during updateFields()
    m_blockableEditorWidgets = {m_idEdit,
                                m_functionNameEdit,
                                m_captionEdit,
                                m_inputPortEdit,
                                m_outputPortEdit,
                                m_trainerInputEdit,
                                m_trainerOutputEdit};
    // these will be enabled/disabled after updateFields();
    m_editableEditorWidgets = {m_captionEdit, m_inputPortEdit, m_outputPortEdit};

    // init initial disabled state
    updateFields();

    connect(this, &Blocks::nodeIdChanged, this, &Blocks::updateFields);
    connect(m_blockManager.get(), &BlockManager::nodeUpdated, this, &Blocks::onNodeUpdated);

    setupCaptionValidation();

    connect(m_inputPortEdit, &QSpinBox::valueChanged, this, [this](int value) {
        m_blockManager->getBlock(m_nodeId)->setInputPortNumber(value);
    });
    connect(m_outputPortEdit, &QSpinBox::valueChanged, this, [this](int value) {
        m_blockManager->getBlock(m_nodeId)->setOutputPortNumber(value);
    });
    connect(m_trainerInputEdit, &QSpinBox::valueChanged, this, [this](int value) {
        if (auto trainer = dynamic_cast<TrainerModel *>(m_blockManager->getBlock(m_nodeId)))
            trainer->setTrainerInputNumber(value);
    });
    connect(m_trainerOutputEdit, &QSpinBox::valueChanged, this, [this](int value) {
        if (auto trainer = dynamic_cast<TrainerModel *>(m_blockManager->getBlock(m_nodeId)))
            trainer->setTrainerOutputNumber(value);
    });
}

void Blocks::setupCaptionValidation()
{
    // Regular expression: allow letters, digits, hyphens, underscores, and fullstops
    // when a user types in an invalid caption, the pop up
    //comes up after 4 seconds. The invalid caption won't be saved
    QTimer *hideIconTimer = new QTimer(this);
    hideIconTimer->setSingleShot(true);
    hideIconTimer->setInterval(4000);
    connect(m_captionEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        QString sanitized = constants::sanitizeCaption(text);
        bool isValid = (sanitized == text && !text.isEmpty());
        if (!isValid) {
            m_invalidIcon->setVisible(true);
            hideIconTimer->start();
        } else {
            hideIconTimer->stop();
            m_invalidIcon->setVisible(false);
            m_blockManager->getBlock(m_nodeId)->setCaption(text);
        }
    });
    connect(hideIconTimer, &QTimer::timeout, this, [=]() { m_invalidIcon->setVisible(false); });
}

void Blocks::initLibrary()
{
    auto widget = new QWidget();
    auto layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    // search box
    auto *searchBox = new QLineEdit();
    searchBox->setPlaceholderText(QStringLiteral("Filter"));
    searchBox->setClearButtonEnabled(true);

    layout->addWidget(searchBox);

    // tree view
    auto treeView = new QTreeWidget();
    treeView->setHeaderHidden(true);

    layout->addWidget(treeView);

    // populate tree view from registry
    auto registry = BlockManager::getRegistry();
    for (auto const &category : registry->categories()) {
        auto item = new QTreeWidgetItem(treeView);
        item->setText(0, category);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    }
    for (auto const &assoc : registry->registeredModelsCategoryAssociation()) {
        QList<QTreeWidgetItem *> parent = treeView->findItems(assoc.second, Qt::MatchExactly);
        if (parent.count() <= 0)
            continue;
        auto item = new QTreeWidgetItem(parent.first());
        item->setText(0, assoc.first);
    }

    treeView->expandAll();

    connect(treeView, &QTreeWidget::itemClicked, this, &Blocks::onLibraryItemClicked);

    // setup filtering
    connect(searchBox, &QLineEdit::textChanged, [treeView](const QString &text) {
        QTreeWidgetItemIterator categoryIt(treeView, QTreeWidgetItemIterator::HasChildren);
        while (*categoryIt)
            (*categoryIt++)->setHidden(true);
        QTreeWidgetItemIterator it(treeView, QTreeWidgetItemIterator::NoChildren);
        while (*it) {
            auto modelName = (*it)->text(0);
            const bool match = (modelName.contains(text, Qt::CaseInsensitive));
            (*it)->setHidden(!match);
            if (match) {
                QTreeWidgetItem *parent = (*it)->parent();
                while (parent) {
                    parent->setHidden(false);
                    parent = parent->parent();
                }
            }
            ++it;
        }
    });

    m_library->setWidget(widget);
}

void Blocks::blockEditorSignals(bool value)
{
    for (auto widget : m_blockableEditorWidgets)
        widget->blockSignals(value);
}

void Blocks::enableEditorWidgets(bool value)
{
    for (auto widget : m_editableEditorWidgets)
        widget->setEnabled(value);
}

QWidget *Blocks::generatePortsWidget(FdfBlockModel *block)
{
    if (!block)
        return nullptr;

    int portCount = block->nPorts(PortType::Out);
    if (portCount == 0)
        return nullptr;

    auto uidManager = m_tabManager->getCurrentUIDManager();
    if (!uidManager) {
        qWarning() << "UIDManager is null!";
        return nullptr;
    }

    bool hasDataPort = block->hasDataOutPorts();
    bool hasFuncPort = block->hasFunctionOutPorts();

    /*
    To adjust the port table contents based on the output ports type
     If data nodes are the only output ports,
     | Port ID | Type Tag | Annotation |
     else if function nodes are the only output ports,
     | Port ID | Caption |
     else if both data and function nodes are present,
     | Port ID | Type Tag | Annotation | Caption |
    */

    QVector<int> visibleCols = {constants::PortTableColIndex::COL_PORT_ID};
    QStringList headers = {"Port ID"};
    if (hasDataPort) {
        visibleCols << constants::PortTableColIndex::COL_TYPE_TAG
                    << constants::PortTableColIndex::COL_ANNOTATION;
        headers << "Type Tag" << "Annotation";
    }
    if (hasFuncPort) {
        visibleCols << constants::PortTableColIndex::COL_CAPTION;
        headers << "Caption";
    }

    auto tableWidget = new QTableWidget(portCount,
                                        visibleCols.size()); // Number of columns of side table
    tableWidget->setHorizontalHeaderLabels(headers);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    tableWidget->resizeRowsToContents();
    tableWidget->resizeColumnsToContents();

    for (int i = 0; i < portCount; ++i) {
        int colIndex = 0;
        auto outData = block->outData(i);
        auto dataNode = std::dynamic_pointer_cast<DataNode>(outData);
        auto funcNode = std::dynamic_pointer_cast<FunctionNode>(outData);

        for (int col : visibleCols) {
            QTableWidgetItem *item = nullptr;

            switch (col) {
            case constants::PortTableColIndex::COL_PORT_ID:
                item = new QTableWidgetItem(QString::number(i));
                item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                break;

            case constants::PortTableColIndex::COL_TYPE_TAG:
                if (dataNode) {
                    QString tag = uidManager->getTag(dataNode->typeId());
                    item = new QTableWidgetItem(tag);

                    // Disable editing if the type ID is NONE_ID
                    if (dataNode->typeId() == UIDManager::NONE_ID) {
                        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                        item->setBackground(QColor("#e0e0e0"));
                        item->setToolTip("NONE_ID type cannot be edited");
                    } else
                        item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable
                                       | Qt::ItemIsEnabled);
                } else {
                    item = new QTableWidgetItem("");
                    item->setBackground(QColor("#e0e0e0"));
                    item->setFlags(Qt::NoItemFlags);
                }
                break;

            case constants::PortTableColIndex::COL_ANNOTATION:
                if (dataNode) {
                    item = new QTableWidgetItem(dataNode->annotation());
                    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                } else {
                    item = new QTableWidgetItem("");
                    item->setBackground(QColor("#e0e0e0"));
                    item->setFlags(Qt::NoItemFlags);
                }
                break;

            case constants::PortTableColIndex::COL_CAPTION:
                if (funcNode) {
                    item = new QTableWidgetItem(funcNode->name());
                    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                } else {
                    item = new QTableWidgetItem("");
                    item->setBackground(QColor("#e0e0e0"));
                    item->setFlags(Qt::NoItemFlags);
                }
                break;
            }

            if (item)
                tableWidget->setItem(i, colIndex, item);

            ++colIndex;
        }
    }

    connect(tableWidget,
            &QTableWidget::itemChanged,
            block,
            [block, tableWidget, uidManager, visibleCols, this](QTableWidgetItem *item) {
                handlePortEdit(block, tableWidget, item, uidManager, visibleCols);
            });
    return tableWidget;
}

void Blocks::handlePortEdit(FdfBlockModel *block,
                            QTableWidget *tableWidget,
                            QTableWidgetItem *item,
                            UIDManager *uidManager,
                            const QVector<int> &visibleCols)
{
    if (!item || !block || !uidManager)
        return;

    int tableCol = item->column();
    // get the correct column index as the number of columns changes dynamically
    int col = visibleCols.value(tableCol, -1);
    int row = item->row();
    auto dataNode = std::dynamic_pointer_cast<DataNode>(block->outData(row));
    auto funcNode = std::dynamic_pointer_cast<FunctionNode>(block->outData(row));

    auto checkTypeTagConflict = []() -> bool {
        auto box = QMessageBox::question(nullptr,
                                         "Type Tag Conflict",
                                         constants::WARN_MANUAL_OVERRIDE,
                                         QMessageBox::Yes | QMessageBox::No);
        return box == QMessageBox::Yes;
    };

    if (dataNode) {
        if (col == constants::PortTableColIndex::COL_TYPE_TAG) {
            QString newTag = item->text();
            if (uidManager->getUid(newTag) != UIDManager::NONE_ID) {
                if (checkTypeTagConflict()) {
                    block->setPortTagAndAnnotation(PortType::Out,
                                                   row,
                                                   newTag,
                                                   dataNode->annotation());
                }
            } else {
                block->setPortTagAndAnnotation(PortType::Out, row, newTag, dataNode->annotation());
            }
        } else if (col == constants::PortTableColIndex::COL_ANNOTATION) {
            QString newAnnot = item->text();
            block->setPortTagAndAnnotation(PortType::Out, row, dataNode->typeTagName(), newAnnot);
        }
    } else if (funcNode && col == constants::PortTableColIndex::COL_CAPTION) {
        QString newCaption = item->text();
        funcNode->setName(newCaption);
        Q_EMIT block->outPortCaptionUpdated(row, newCaption);
    }
}

QWidget *Blocks::generateParameterWidget(FdfBlockModel *block)
{
    if (!block)
        return nullptr;
    auto values = block->getParameters();
    if (values.empty())
        return nullptr;
    auto widget = new QWidget;
    auto layout = new QFormLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    for (auto &pair : block->getParameterSchema()) {
        auto key = pair.first;
        // value is not found for now, we need to decide how to add optional params
        if (values.count(key) < 1)
            continue;
        auto value = values.at(key);
        if (pair.second == QMetaType::QString) {
            auto options = block->getParameterOptions(key);
            if (options.isEmpty()) {
                auto edit = new QLineEdit(value);
                layout->addRow(new QLabel(key), edit);
                connect(edit, &QLineEdit::textChanged, block, [block, key](const QString &text) {
                    block->setParameter(key, text);
                });
            } else {
                auto comboBox = new QComboBox;
                comboBox->addItems(options);
                comboBox->setCurrentText(value);
                layout->addRow(new QLabel(key), comboBox);
                connect(comboBox,
                        &QComboBox::currentTextChanged,
                        block,
                        [block, key](const QString &text) { block->setParameter(key, text); });
            }
        } else if (pair.second == QMetaType::Int) {
            auto spin = new QSpinBox;
            spin->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());
            spin->setMaximumWidth(constants::INT_SPIN_BOX_MAX_WIDTH);
            spin->setMinimumWidth(constants::INT_SPIN_BOX_MIN_WIDTH);
            spin->setValue(value.toInt());
            layout->addRow(new QLabel(key), spin);
            connect(spin, &QSpinBox::valueChanged, block, [block, key](const int &value) {
                block->setParameter(key, QString::number(value));
            });
        } else if (pair.second == QMetaType::Double) {
            auto spin = new QDoubleSpinBox;
            spin->setRange(0, std::numeric_limits<double>::max());
            spin->setDecimals(4);
            spin->setMaximumWidth(constants::DOUBLE_SPIN_BOX_MAX_WIDTH);
            spin->setMinimumWidth(constants::DOUBLE_SPIN_BOX_MIN_WIDTH);
            spin->setValue(value.toDouble());
            layout->addRow(new QLabel(key), spin);
            connect(spin, &QDoubleSpinBox::valueChanged, block, [block, key](double value) {
                block->setParameter(key, QString::number(value, 'f', 6));
            });
        } else if (pair.second == QMetaType::QPoint) {
            auto pointLayout = new QHBoxLayout();
            {
                int xValue = value.mid(value.indexOf('[') + 1, value.indexOf(',') - 1).toInt();
                int yValue = value.mid(value.indexOf(' ') + 1, value.indexOf(']') - 1).toInt();
                pointLayout->setContentsMargins(0, 0, 0, 0);
                pointLayout->setSpacing(0);
                auto xSpin = new QSpinBox;
                xSpin->setRange(0, std::numeric_limits<int>::max());
                xSpin->setMaximumWidth(constants::INT_SPIN_BOX_MAX_WIDTH);
                xSpin->setMinimumWidth(constants::INT_SPIN_BOX_MIN_WIDTH);
                xSpin->setValue(xValue);
                pointLayout->addWidget(xSpin);
                pointLayout->addWidget(new QLabel(", "));
                auto ySpin = new QSpinBox;
                ySpin->setRange(0, std::numeric_limits<int>::max());
                ySpin->setMaximumWidth(constants::INT_SPIN_BOX_MAX_WIDTH);
                ySpin->setMinimumWidth(constants::INT_SPIN_BOX_MIN_WIDTH);
                ySpin->setValue(yValue);
                pointLayout->addWidget(ySpin);
                connect(xSpin, &QSpinBox::valueChanged, block, [block, key, ySpin](const int &value) {
                    block->setParameter(key,
                                        QString("[%1, %2]")
                                            .arg(QString::number(value),
                                                 QString::number(ySpin->value())));
                });
                connect(ySpin, &QSpinBox::valueChanged, block, [block, key, xSpin](const int &value) {
                    block->setParameter(key,
                                        QString("[%1, %2]")
                                            .arg(QString::number(xSpin->value()), value));
                });
            }
            layout->addRow(new QLabel(key), pointLayout);
        } else if (pair.second == QMetaType::QVector2D) {
            // UI for this can be improved
            auto edit = new QLineEdit(value);
            layout->addRow(new QLabel(key), edit);
            connect(edit, &QLineEdit::textChanged, block, [block, key](const QString &text) {
                block->setParameter(key, text);
            });
        } else {
            qCritical() << "Block parameter type is unhandled" << pair.second;
        }
    }
    return widget;
}

void Blocks::handleInputRows(FdfBlockModel *block)
{
    if (!block)
        return;
    auto trainer = dynamic_cast<TrainerModel *>(block);
    bool isTrainer = (trainer != nullptr);
    m_editorLayout->setRowVisible(INPUT_PORT_ROW, !isTrainer);
    m_editorLayout->setRowVisible(OUTPUT_PORT_ROW, !isTrainer);
    m_editorLayout->setRowVisible(TRAINER_INPUT_ROW, isTrainer);
    m_editorLayout->setRowVisible(TRAINER_OUTPUT_ROW, isTrainer);
    if (isTrainer) {
        m_trainerInputEdit->setValue(trainer->getTrainerInputPortNum());
        m_trainerOutputEdit->setValue(trainer->getTrainerOutputPortNum());
    }
}
