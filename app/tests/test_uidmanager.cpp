#include "ui/models/io_models.hpp"
#include "ui/models/uid_manager.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <QFileInfo>

std::shared_ptr<DataNode> fetchDataNodeFromFile(const QString &filePath)
{
    DataSourceModel dataSource;
    QFileInfo file(filePath);
    dataSource.setFile(file);
    return std::dynamic_pointer_cast<DataNode>(dataSource.outData(0));
}

TEST(UIDManagerTest, CanSetAndGetTagFromDataSource)
{
    auto d1 = fetchDataNodeFromFile("data/flux_a.csv");
    auto d2 = fetchDataNodeFromFile("data/flux_b.csv");

    ASSERT_NE(d1, nullptr);
    ASSERT_NE(d2, nullptr);

    EXPECT_EQ(d1->typeTagName(), "flux_a");
    EXPECT_EQ(d2->typeTagName(), "flux_b");
}

TEST(UIDManagerTest, DifferentTagReturnsCorrectID)
{
    UIDManager uidManager;
    FdfUID id1 = uidManager.createUID("type_1");
    FdfUID id2 = uidManager.createUID("type_2");

    ASSERT_NE(id1, id2);
    EXPECT_EQ(uidManager.getTag(id1), "type_1");
    EXPECT_EQ(uidManager.getTag(id2), "type_2");
}

TEST(UIDManagerTest, UpdateMapOverridesTag)
{
    UIDManager uidManager;
    FdfUID id = uidManager.createUID("original_tag");
    QString updatedTag = "updated_tag";
    uidManager.updateMap(id, updatedTag);

    EXPECT_EQ(uidManager.getTag(id), "updated_tag");
}

TEST(UIDManagerTest, ReusingTagCreatesUniqueUID)
{
    UIDManager uidManager;
    auto id1 = uidManager.createUID("shared_tag");
    auto id2 = uidManager.createUID("shared_tag");

    // TODO : Change the implementation later to raise exception if
    // the same tag is used to create a new UID entry
    ASSERT_NE(id1, id2);
    EXPECT_EQ(uidManager.getTag(id1), "shared_tag");
    EXPECT_EQ(uidManager.getTag(id2), "shared_tag_1");
}

TEST(UIDManagerTest, InvalidUIDReturnsEmptyTag)
{
    UIDManager uidManager;
    FdfUID unknown = 999;
    auto tag = uidManager.getTag(unknown);

    ASSERT_TRUE(tag == UIDManager::NONE_TAG);
}
