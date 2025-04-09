#include <gtest/gtest.h>

#include <QDebug>
#include <QTemporaryDir>

#include <QtUtility/file/file.hpp>

namespace file = QtUtility::file;

class FileTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        tempDir.reset(new QTemporaryDir);
        ASSERT_TRUE(tempDir->isValid()) << "Failed to create temporary directory";
    }

    void TearDown() override { tempDir.reset(); }

    QString getTempFilePath(const QString &fileName) const { return tempDir->filePath(fileName); }

    const QString FILE_NAME = "test.txt";
    std::unique_ptr<QTemporaryDir> tempDir;
};

TEST_F(FileTest, TestCreate)
{
    QFileInfo fileInfo(getTempFilePath(FILE_NAME));

    // verify file doesn't exist
    EXPECT_FALSE(fileInfo.exists()) << "File already exists";

    // create a new file
    EXPECT_TRUE(file::create(fileInfo)) << "Failed to create file";

    // verify file exists
    EXPECT_TRUE(fileInfo.exists()) << "File was not created";

    // verify result is true when file already exists
    EXPECT_TRUE(file::create(fileInfo)) << "Failed to create file";
}

TEST_F(FileTest, TestGetUniqueFile)
{
    QString filePath = getTempFilePath(FILE_NAME);
    QFileInfo fileInfo(filePath);

    auto result = file::getUniqueFile(fileInfo);
    // verify unique name is the same since we haven't created any files
    ASSERT_TRUE(result == fileInfo) << "Unique name failed initial match";

    // create a file with the initial name
    ASSERT_TRUE(file::create(fileInfo)) << "Failed to create initial file";

    // verify file exists
    ASSERT_TRUE(fileInfo.exists()) << "Initial file doesn't exist";

    auto newResult = file::getUniqueFile(fileInfo);
    ASSERT_TRUE(newResult != result) << "New unique file is the same as the old one";
}

TEST_F(FileTest, TestCreateUnique)
{
    QFileInfo fileInfo(getTempFilePath(FILE_NAME));

    // create a file with the same name
    EXPECT_TRUE(file::create(fileInfo)) << "Failed to create file";

    auto incrementedFileInfo = file::getUniqueFile(fileInfo);
    // verify incremented file info doesn't already exists
    EXPECT_FALSE(incrementedFileInfo.exists()) << "Incremented file already exists";

    // create a file with the same name, should auto increment
    auto unique = file::createUnique(fileInfo);
    EXPECT_TRUE(incrementedFileInfo.exists()) << "Failed to create unique file";

    // verify file is different
    EXPECT_TRUE(unique.absoluteFilePath() != fileInfo.absoluteFilePath()) << "File is not unique";
}