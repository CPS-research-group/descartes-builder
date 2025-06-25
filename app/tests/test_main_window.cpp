#include "data/constants.hpp"
#include "data/custom_graph.hpp"
#include "data/tab_manager.hpp"
#include "data/yml_parser.hpp"
#include "ui/main_window.hpp"
#include "ui/models/fdf_block_model.hpp"
#include "ui/models/io_models.hpp"
#include "ui/models/uid_manager.hpp"
#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>

class MainWindowTest : public ::testing::Test
{
protected:
    void SetUp() override { mainWindow = new MainWindow(); }

    void TearDown() override { delete mainWindow; }

    MainWindow *mainWindow = nullptr;
};

TEST_F(MainWindowTest, WindowIsCreated)
{
    ASSERT_NE(mainWindow, nullptr) << "MainWindow should be created.";
}

double roundTo(double value, int decimalPlaces)
{
    double factor = std::pow(10.0, decimalPlaces);
    return std::round(value * factor) / factor;
}

TEST_F(MainWindowTest, WindowOpenDCB)
{
    QFileInfo testFileInfo(__FILE__);
    QString baseDir = testFileInfo.absolutePath();
    QDir dcbDir(baseDir + "/../../examples");
    QString dcbPath = dcbDir.absoluteFilePath("pipe-deformation-train.dcb");
    QDir resultsDir(baseDir + "/results");
    QString resultsPath = resultsDir.absoluteFilePath("pipe_deformation_expected_score.yml");

    QSignalSpy scoreCreatedSpy(mainWindow, SIGNAL(scoreParams(QString)));
    ASSERT_TRUE(mainWindow->openDCB(dcbPath))
        << "openDCB failed for path: " << dcbPath.toStdString();
    mainWindow->executeDCB();

    bool scoreSignalReceived = scoreCreatedSpy.wait(constants::MINUTE_MSECS);
    ASSERT_TRUE(scoreSignalReceived) << "Timed out waiting for score.yml creation";

    QString generatedScoreContent = scoreCreatedSpy.takeFirst().at(0).toString();
    ASSERT_FALSE(generatedScoreContent.isEmpty()) << "Received score.yml content is empty";

    QFile expectedFile(resultsPath);
    ASSERT_TRUE(expectedFile.open(QIODevice::ReadOnly | QIODevice::Text))
        << "Failed to open expected score.yml";

    QString expectedScoreContent = QString::fromUtf8(expectedFile.readAll());

    auto expectedMap = parseYml(expectedScoreContent);
    auto generatedMap = parseYml(generatedScoreContent);

    constexpr double FAULT_TOLERANCE = 1e-2;
    constexpr int DECIMAL_PLACES = 4;
    for (const auto &pair : expectedMap) {
        const QString &key = pair.first;
        ASSERT_TRUE(generatedMap.find(key) != generatedMap.end())
            << "Missing key in generated map: " << key.toStdString();
        double expectedVal = pair.second.toDouble();
        double generatedVal = generatedMap.at(key).toDouble();
        expectedVal = roundTo(expectedVal, DECIMAL_PLACES);
        generatedVal = roundTo(generatedVal, DECIMAL_PLACES);
        ASSERT_NEAR(generatedVal, expectedVal, FAULT_TOLERANCE)
            << "Mismatch for " << key.toStdString();
    }
}

TEST_F(MainWindowTest, TypePropagation)
{
    QFileInfo testFileInfo(__FILE__);
    QString baseDir = testFileInfo.absolutePath();
    QDir dcbDir(baseDir + "/../../examples/tests");
    QString dcbPath = dcbDir.absoluteFilePath("test_propagation.dcb");

    ASSERT_TRUE(mainWindow->openDCB(dcbPath))
        << "openDCB failed for path: " << dcbPath.toStdString();
    CustomGraph *graph = mainWindow->getTabManager()->getCurrentTab()->getGraph();
    ASSERT_TRUE(graph != nullptr) << "Graph should not be null after opening DCB.";

    auto dataSourceModels = graph->getDataSourceModels();
    ASSERT_TRUE(dataSourceModels.size() == 1)
        << "Expected 1 data source model, but found: " << dataSourceModels.size();

    auto dataSourceBlock = dataSourceModels[0];
    dataSourceBlock->setPortTagAndAnnotation(PortType::Out, 0, "upd_tag", "");

    auto targetBlock = graph->getBlockByCaption("process_2");
    ASSERT_TRUE(targetBlock != nullptr) << "Target model should not be null.";

    auto dataSourcePort = dataSourceBlock->allOutData<DataNode>()[0];
    auto targetPort = targetBlock->allOutData<DataNode>()[0];
    ASSERT_TRUE(targetPort->typeTagName() == "upd_tag"
                && targetPort->typeId() == dataSourcePort->typeId())
        << "Type tag and ID should match after propagation.";

    ASSERT_EQ(targetPort->typeTagName(), "upd_tag") << "Type tag should match after propagation.";
    ASSERT_EQ(targetPort->typeId(), dataSourcePort->typeId())
        << "Type ID should match after propagation.";
}