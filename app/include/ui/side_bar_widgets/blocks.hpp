#pragma once

#include <QObject>
#include <QWidget>

#include <unordered_set>
#include <QtNodes/NodeDelegateModel>

class QLabel;
class QTreeWidgetItem;
class QLineEdit;
class QSpinBox;
class QStackedWidget;
class QFormLayout;
class BlockManager;
class TabManager;
class FdfBlockModel;
using QtNodes::PortType;

namespace QtUtility {
namespace widgets {
class QCollapsibleWidget;
} // namespace widgets
} // namespace QtUtility

#include <QtNodes/Definitions>

class Blocks : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QtNodes::NodeId nodeId READ nodeId WRITE setNodeId NOTIFY nodeIdChanged)
public:
    Blocks(std::shared_ptr<BlockManager> blockManager,
           std::shared_ptr<TabManager> tabManager,
           QWidget *parent = nullptr);

    QtNodes::NodeId nodeId() const { return m_nodeId; }
    void setNodeId(QtNodes::NodeId id);

signals:
    void nodeIdChanged(QtNodes::NodeId id);

private slots:
    void updateFields();
    void onLibraryItemClicked(QTreeWidgetItem *item);
    void onNodeUpdated(QtNodes::NodeId id);

private:
    void initUi();
    void initEditor();
    void initLibrary();
    void blockEditorSignals(bool value);
    void enableEditorWidgets(bool value);
    QWidget *generateParameterWidget(FdfBlockModel *block);
    QWidget *generatePortsWidget(FdfBlockModel *block, const PortType &portType);
    void handleInputRows(FdfBlockModel *block);

    std::shared_ptr<BlockManager> m_blockManager;
    std::shared_ptr<TabManager> m_tabManager;
    QtNodes::NodeId m_nodeId;

    QtUtility::widgets::QCollapsibleWidget *m_blockEditor;
    std::unordered_set<QWidget *> m_blockableEditorWidgets;
    std::unordered_set<QWidget *> m_editableEditorWidgets;
    QFormLayout *m_editorLayout;
    QLineEdit *m_idEdit;
    QLineEdit *m_captionEdit;
    QLineEdit *m_functionNameEdit;
    QSpinBox *m_inputPortEdit;
    QStackedWidget *m_outputPorts;
    QSpinBox *m_outputPortEdit;
    QSpinBox *m_trainerInputEdit;
    QSpinBox *m_trainerOutputEdit;
    QStackedWidget *m_parametersWidget;
    QtUtility::widgets::QCollapsibleWidget *m_library;
};
