/**
 * @file fs_test.cpp
 * @brief Unit tests for the filesystem module
 */

#include <gtest/gtest.h>
#include "../core/fs/fs.hpp"
#include <filesystem>
#include <fstream>
#include <cstring>

using namespace ezquake;
using namespace ezquake::fs;

// ============================================================================
// FileError Tests
// ============================================================================

TEST(FileErrorTest, ErrorToString) {
    EXPECT_EQ(fileErrorToString(FileError::None), "No error");
    EXPECT_EQ(fileErrorToString(FileError::NotFound), "File not found");
    EXPECT_EQ(fileErrorToString(FileError::AccessDenied), "Access denied");
    EXPECT_EQ(fileErrorToString(FileError::EndOfFile), "End of file");
    EXPECT_EQ(fileErrorToString(FileError::OutOfMemory), "Out of memory");
}

// ============================================================================
// FileFlags Tests
// ============================================================================

TEST(FileFlagsTest, BitwiseOperations) {
    auto flags = FileFlags::ReadOnly | FileFlags::CopyProtected;
    
    EXPECT_TRUE(hasFlag(flags, FileFlags::ReadOnly));
    EXPECT_TRUE(hasFlag(flags, FileFlags::CopyProtected));
    EXPECT_FALSE(hasFlag(flags, FileFlags::Compressed));
    
    auto combined = flags & FileFlags::ReadOnly;
    EXPECT_TRUE(hasFlag(combined, FileFlags::ReadOnly));
    EXPECT_FALSE(hasFlag(combined, FileFlags::CopyProtected));
}

// ============================================================================
// MemoryFile Tests
// ============================================================================

class MemoryFileTest : public ::testing::Test {
protected:
    void SetUp() override {
        testData_ = {0x48, 0x65, 0x6C, 0x6C, 0x6F};  // "Hello"
    }
    
    std::vector<Byte> testData_;
};

TEST_F(MemoryFileTest, CreateEmpty) {
    auto file = MemoryFile::create("test");
    
    ASSERT_TRUE(file != nullptr);
    EXPECT_TRUE(file->isOpen());
    EXPECT_EQ(file->path(), "test");
    
    auto size = file->size();
    ASSERT_TRUE(size.has_value());
    EXPECT_EQ(*size, 0u);
}

TEST_F(MemoryFileTest, CreateFromData) {
    auto file = MemoryFile::fromData(testData_, "test.bin");
    
    ASSERT_TRUE(file != nullptr);
    
    auto size = file->size();
    ASSERT_TRUE(size.has_value());
    EXPECT_EQ(*size, 5u);
}

TEST_F(MemoryFileTest, CreateFromString) {
    auto file = MemoryFile::fromString("Hello, World!", "greeting.txt");
    
    ASSERT_TRUE(file != nullptr);
    
    auto size = file->size();
    ASSERT_TRUE(size.has_value());
    EXPECT_EQ(*size, 13u);
}

TEST_F(MemoryFileTest, ReadData) {
    auto file = MemoryFile::fromData(testData_, "test.bin");
    
    std::vector<Byte> buffer(5);
    auto result = file->read(buffer);
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 5u);
    EXPECT_EQ(buffer, testData_);
}

TEST_F(MemoryFileTest, ReadPartial) {
    auto file = MemoryFile::fromData(testData_, "test.bin");
    
    std::vector<Byte> buffer(3);
    auto result = file->read(buffer);
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 3u);
    EXPECT_EQ(buffer[0], 0x48);  // 'H'
    EXPECT_EQ(buffer[1], 0x65);  // 'e'
    EXPECT_EQ(buffer[2], 0x6C);  // 'l'
    
    // Read rest
    result = file->read(buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 2u);  // Only 2 bytes left
}

TEST_F(MemoryFileTest, ReadPastEnd) {
    auto file = MemoryFile::fromData(testData_, "test.bin");
    
    // Seek to end
    auto seekResult = file->seek(0, SeekOrigin::End);
    ASSERT_TRUE(seekResult.has_value());
    
    std::vector<Byte> buffer(10);
    auto result = file->read(buffer);
    
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), FileError::EndOfFile);
}

TEST_F(MemoryFileTest, WriteData) {
    auto file = MemoryFile::create("test.bin");
    
    auto result = file->write(testData_);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 5u);
    
    auto size = file->size();
    ASSERT_TRUE(size.has_value());
    EXPECT_EQ(*size, 5u);
}

TEST_F(MemoryFileTest, WriteAndRead) {
    auto file = MemoryFile::create("test.bin");
    
    // Write
    file->write(testData_);
    
    // Seek back
    file->seek(0, SeekOrigin::Begin);
    
    // Read
    std::vector<Byte> buffer(5);
    auto result = file->read(buffer);
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(buffer, testData_);
}

TEST_F(MemoryFileTest, SeekBegin) {
    auto file = MemoryFile::fromData(testData_, "test.bin");
    
    // Read some data
    std::vector<Byte> buffer(3);
    file->read(buffer);
    
    // Seek to beginning
    auto result = file->seek(0, SeekOrigin::Begin);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 0u);
    
    auto pos = file->tell();
    ASSERT_TRUE(pos.has_value());
    EXPECT_EQ(*pos, 0u);
}

TEST_F(MemoryFileTest, SeekCurrent) {
    auto file = MemoryFile::fromData(testData_, "test.bin");
    
    file->seek(2, SeekOrigin::Begin);
    
    auto result = file->seek(1, SeekOrigin::Current);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 3u);
}

TEST_F(MemoryFileTest, SeekEnd) {
    auto file = MemoryFile::fromData(testData_, "test.bin");
    
    auto result = file->seek(-2, SeekOrigin::End);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 3u);
}

TEST_F(MemoryFileTest, SeekInvalid) {
    auto file = MemoryFile::fromData(testData_, "test.bin");
    
    // Seek before beginning
    auto result = file->seek(-10, SeekOrigin::Begin);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), FileError::SeekError);
}

TEST_F(MemoryFileTest, Eof) {
    auto file = MemoryFile::fromData(testData_, "test.bin");
    
    EXPECT_FALSE(file->eof());
    
    file->seek(0, SeekOrigin::End);
    EXPECT_TRUE(file->eof());
    
    file->seek(0, SeekOrigin::Begin);
    EXPECT_FALSE(file->eof());
}

TEST_F(MemoryFileTest, ReadAllText) {
    auto file = MemoryFile::fromString("Hello, World!", "test.txt");
    
    auto result = file->readAllText();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "Hello, World!");
}

TEST_F(MemoryFileTest, ReadLine) {
    auto file = MemoryFile::fromString("Line 1\nLine 2\nLine 3", "test.txt");
    
    auto line1 = file->readLine();
    ASSERT_TRUE(line1.has_value());
    EXPECT_EQ(*line1, "Line 1");
    
    auto line2 = file->readLine();
    ASSERT_TRUE(line2.has_value());
    EXPECT_EQ(*line2, "Line 2");
    
    auto line3 = file->readLine();
    ASSERT_TRUE(line3.has_value());
    EXPECT_EQ(*line3, "Line 3");
    
    auto line4 = file->readLine();
    EXPECT_FALSE(line4.has_value());
    EXPECT_EQ(line4.error(), FileError::EndOfFile);
}

TEST_F(MemoryFileTest, ReadLineCRLF) {
    auto file = MemoryFile::fromString("Line 1\r\nLine 2\r\n", "test.txt");
    
    auto line1 = file->readLine();
    ASSERT_TRUE(line1.has_value());
    EXPECT_EQ(*line1, "Line 1");
    
    auto line2 = file->readLine();
    ASSERT_TRUE(line2.has_value());
    EXPECT_EQ(*line2, "Line 2");
}

TEST_F(MemoryFileTest, ViewIsReadOnly) {
    auto file = MemoryFile::fromView(testData_, "view.bin");
    
    EXPECT_TRUE(file->isReadOnly());
    
    std::vector<Byte> data = {0x01, 0x02};
    auto result = file->write(data);
    
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), FileError::UnsupportedOperation);
}

TEST_F(MemoryFileTest, Detach) {
    auto file = MemoryFile::create("test.bin");
    file->write(testData_);
    
    auto* memFile = static_cast<MemoryFile*>(file.get());
    auto detached = memFile->detach();
    
    EXPECT_EQ(detached, testData_);
    
    auto size = file->size();
    ASSERT_TRUE(size.has_value());
    EXPECT_EQ(*size, 0u);
}

// ============================================================================
// OsFile Tests
// ============================================================================

class OsFileTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for tests
        tempDir_ = std::filesystem::temp_directory_path() / "ezquake_fs_test";
        std::filesystem::create_directories(tempDir_);
        
        testData_ = {0x48, 0x65, 0x6C, 0x6C, 0x6F};  // "Hello"
    }
    
    void TearDown() override {
        // Clean up temp directory
        std::error_code ec;
        std::filesystem::remove_all(tempDir_, ec);
    }
    
    std::filesystem::path createTestFile(const std::string& name, 
                                          const std::vector<Byte>& data) {
        auto path = tempDir_ / name;
        std::ofstream out(path, std::ios::binary);
        out.write(reinterpret_cast<const char*>(data.data()), 
                  static_cast<std::streamsize>(data.size()));
        return path;
    }
    
    std::filesystem::path tempDir_;
    std::vector<Byte> testData_;
};

TEST_F(OsFileTest, OpenNonexistent) {
    auto result = OsFile::open(tempDir_ / "nonexistent.txt", OpenMode::Read);
    
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), FileError::NotFound);
}

TEST_F(OsFileTest, OpenExisting) {
    auto path = createTestFile("test.bin", testData_);
    
    auto result = OsFile::open(path, OpenMode::Read);
    ASSERT_TRUE(result.has_value());
    
    auto& file = *result;
    EXPECT_TRUE(file->isOpen());
    EXPECT_FALSE(file->eof());
}

TEST_F(OsFileTest, ReadData) {
    auto path = createTestFile("test.bin", testData_);
    auto file = OsFile::open(path, OpenMode::Read);
    ASSERT_TRUE(file.has_value());
    
    std::vector<Byte> buffer(5);
    auto result = (*file)->read(buffer);
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 5u);
    EXPECT_EQ(buffer, testData_);
}

TEST_F(OsFileTest, Size) {
    auto path = createTestFile("test.bin", testData_);
    auto file = OsFile::open(path, OpenMode::Read);
    ASSERT_TRUE(file.has_value());
    
    auto size = (*file)->size();
    ASSERT_TRUE(size.has_value());
    EXPECT_EQ(*size, 5u);
}

TEST_F(OsFileTest, WriteNewFile) {
    auto path = tempDir_ / "new_file.bin";
    
    auto file = OsFile::open(path, OpenMode::Write);
    ASSERT_TRUE(file.has_value());
    
    auto result = (*file)->write(testData_);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 5u);
    
    (*file)->flush();
    
    // Verify file contents
    EXPECT_TRUE(std::filesystem::exists(path));
    EXPECT_EQ(std::filesystem::file_size(path), 5u);
}

TEST_F(OsFileTest, CreateExisting) {
    auto path = createTestFile("existing.bin", testData_);
    
    auto result = OsFile::create(path);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), FileError::AlreadyExists);
}

TEST_F(OsFileTest, CreateNew) {
    auto path = tempDir_ / "new_file.bin";
    
    auto result = OsFile::create(path);
    ASSERT_TRUE(result.has_value());
    
    EXPECT_TRUE(std::filesystem::exists(path));
}

TEST_F(OsFileTest, SeekAndTell) {
    auto path = createTestFile("test.bin", testData_);
    auto file = OsFile::open(path, OpenMode::Read);
    ASSERT_TRUE(file.has_value());
    
    // Initial position
    auto pos = (*file)->tell();
    ASSERT_TRUE(pos.has_value());
    EXPECT_EQ(*pos, 0u);
    
    // Seek to middle
    auto seekResult = (*file)->seek(2, SeekOrigin::Begin);
    ASSERT_TRUE(seekResult.has_value());
    EXPECT_EQ(*seekResult, 2u);
    
    pos = (*file)->tell();
    ASSERT_TRUE(pos.has_value());
    EXPECT_EQ(*pos, 2u);
}

TEST_F(OsFileTest, ReadAllText) {
    auto path = createTestFile("hello.txt", 
        std::vector<Byte>{'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!'});
    
    auto file = OsFile::open(path, OpenMode::Read);
    ASSERT_TRUE(file.has_value());
    
    auto result = (*file)->readAllText();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "Hello, World!");
}

TEST_F(OsFileTest, Path) {
    auto path = createTestFile("test.bin", testData_);
    auto file = OsFile::open(path, OpenMode::Read);
    ASSERT_TRUE(file.has_value());
    
    EXPECT_EQ((*file)->path(), path.string());
}

TEST_F(OsFileTest, IsDirectory) {
    auto result = OsFile::open(tempDir_, OpenMode::Read);
    
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), FileError::IsDirectory);
}

// ============================================================================
// FileSystem Tests
// ============================================================================

class FileSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directories
        tempDir_ = std::filesystem::temp_directory_path() / "ezquake_fs_test";
        baseDir_ = tempDir_ / "base";
        homeDir_ = tempDir_ / "home";
        
        std::filesystem::create_directories(baseDir_ / "qw");
        std::filesystem::create_directories(homeDir_ / "qw");
        std::filesystem::create_directories(baseDir_ / "id1");
        
        // Create some test files
        createFile(baseDir_ / "qw" / "base_config.cfg", "// Base config\n");
        createFile(homeDir_ / "qw" / "user_config.cfg", "// User config\n");
        createFile(baseDir_ / "id1" / "pak0.txt", "PAK0 contents\n");
    }
    
    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(tempDir_, ec);
    }
    
    void createFile(const std::filesystem::path& path, const std::string& content) {
        std::ofstream out(path);
        out << content;
    }
    
    std::filesystem::path tempDir_;
    std::filesystem::path baseDir_;
    std::filesystem::path homeDir_;
};

TEST_F(FileSystemTest, Init) {
    FileSystem fs;
    fs.init(baseDir_, homeDir_, "qw");
    
    EXPECT_EQ(fs.baseDir(), baseDir_);
    EXPECT_EQ(fs.homeDir(), homeDir_);
    EXPECT_EQ(fs.gameDirName(), "qw");
}

TEST_F(FileSystemTest, LocateInHome) {
    FileSystem fs;
    fs.init(baseDir_, homeDir_, "qw");
    
    auto loc = fs.locate("user_config.cfg");
    ASSERT_TRUE(loc.has_value());
    EXPECT_EQ(loc->fullPath, homeDir_ / "qw" / "user_config.cfg");
}

TEST_F(FileSystemTest, LocateInBase) {
    FileSystem fs;
    fs.init(baseDir_, homeDir_, "qw");
    
    auto loc = fs.locate("base_config.cfg");
    ASSERT_TRUE(loc.has_value());
    EXPECT_EQ(loc->fullPath, baseDir_ / "qw" / "base_config.cfg");
}

TEST_F(FileSystemTest, Exists) {
    FileSystem fs;
    fs.init(baseDir_, homeDir_, "qw");
    
    EXPECT_TRUE(fs.exists("user_config.cfg"));
    EXPECT_TRUE(fs.exists("base_config.cfg"));
    EXPECT_FALSE(fs.exists("nonexistent.cfg"));
}

TEST_F(FileSystemTest, LoadText) {
    FileSystem fs;
    fs.init(baseDir_, homeDir_, "qw");
    
    auto result = fs.loadText("user_config.cfg");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "// User config\n");
}

TEST_F(FileSystemTest, SaveAndLoad) {
    FileSystem fs;
    fs.init(baseDir_, homeDir_, "qw");
    
    // Save a new file
    auto saveResult = fs.saveText("new_file.txt", "New content\n");
    ASSERT_TRUE(saveResult.has_value());
    
    // Load it back
    auto loadResult = fs.loadText("new_file.txt");
    ASSERT_TRUE(loadResult.has_value());
    EXPECT_EQ(*loadResult, "New content\n");
    
    // File should be in home directory
    EXPECT_TRUE(std::filesystem::exists(homeDir_ / "qw" / "new_file.txt"));
}

TEST_F(FileSystemTest, AddSearchPath) {
    FileSystem fs;
    fs.init(baseDir_, homeDir_, "qw");
    
    auto customDir = tempDir_ / "custom";
    std::filesystem::create_directories(customDir);
    createFile(customDir / "custom.cfg", "custom content");
    
    fs.addSearchPath(customDir, SearchPathType::Directory, 200);
    
    EXPECT_TRUE(fs.exists("custom.cfg"));
}

TEST_F(FileSystemTest, SearchPathPriority) {
    FileSystem fs;
    fs.init(baseDir_, homeDir_, "qw");
    
    // Create same file in both directories with different content
    createFile(baseDir_ / "qw" / "priority_test.cfg", "base content");
    createFile(homeDir_ / "qw" / "priority_test.cfg", "home content");
    
    // Home directory should have priority
    auto result = fs.loadText("priority_test.cfg");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "home content");
}

TEST_F(FileSystemTest, RemoveSearchPath) {
    FileSystem fs;
    fs.init(baseDir_, homeDir_, "qw");
    
    auto paths = fs.searchPaths();
    size_t initialCount = paths.size();
    
    fs.removeSearchPath(baseDir_ / "qw");
    
    paths = fs.searchPaths();
    EXPECT_EQ(paths.size(), initialCount - 1);
}

TEST_F(FileSystemTest, FlushCache) {
    FileSystem fs;
    fs.init(baseDir_, homeDir_, "qw");
    
    // Access a file to cache it
    fs.locate("base_config.cfg");
    
    // Create a new file
    createFile(homeDir_ / "qw" / "new_cached.cfg", "new");
    
    // Before flush, might use stale cache
    fs.flushCache();
    
    // After flush, should find new file
    EXPECT_TRUE(fs.exists("new_cached.cfg"));
}

TEST_F(FileSystemTest, UnsafeFilename) {
    EXPECT_TRUE(FileSystem::isUnsafeFilename(""));
    EXPECT_TRUE(FileSystem::isUnsafeFilename("/etc/passwd"));
    EXPECT_TRUE(FileSystem::isUnsafeFilename("../secret.txt"));
    EXPECT_TRUE(FileSystem::isUnsafeFilename("foo/../bar"));
    EXPECT_TRUE(FileSystem::isUnsafeFilename("C:\\Windows\\system32"));
    EXPECT_TRUE(FileSystem::isUnsafeFilename("file|cmd"));
    EXPECT_TRUE(FileSystem::isUnsafeFilename("file;rm -rf"));
    
    EXPECT_FALSE(FileSystem::isUnsafeFilename("config.cfg"));
    EXPECT_FALSE(FileSystem::isUnsafeFilename("maps/dm1.bsp"));
    EXPECT_FALSE(FileSystem::isUnsafeFilename("gfx/menu.lmp"));
}

TEST_F(FileSystemTest, SanitizeFilename) {
    EXPECT_EQ(FileSystem::sanitizeFilename("config.cfg"), "config.cfg");
    EXPECT_EQ(FileSystem::sanitizeFilename("maps\\dm1.bsp"), "maps/dm1.bsp");
    
    // Unsafe filenames return empty
    EXPECT_EQ(FileSystem::sanitizeFilename("../secret.txt"), "");
    EXPECT_EQ(FileSystem::sanitizeFilename("/etc/passwd"), "");
}

TEST_F(FileSystemTest, GetExtension) {
    EXPECT_EQ(FileSystem::getExtension("config.cfg"), "cfg");
    EXPECT_EQ(FileSystem::getExtension("map.BSP"), "bsp");
    EXPECT_EQ(FileSystem::getExtension("noextension"), "");
    EXPECT_EQ(FileSystem::getExtension(".hidden"), "hidden");
    EXPECT_EQ(FileSystem::getExtension("file.tar.gz"), "gz");
}

TEST_F(FileSystemTest, CreatePath) {
    auto newPath = tempDir_ / "a" / "b" / "c" / "file.txt";
    
    auto result = FileSystem::createPath(newPath);
    ASSERT_TRUE(result.has_value());
    
    EXPECT_TRUE(std::filesystem::exists(newPath.parent_path()));
}

TEST_F(FileSystemTest, EnumerateFiles) {
    FileSystem fs;
    fs.init(baseDir_, homeDir_, "qw");
    
    createFile(homeDir_ / "qw" / "test1.cfg", "test1");
    createFile(homeDir_ / "qw" / "test2.cfg", "test2");
    createFile(homeDir_ / "qw" / "test.txt", "test");
    
    std::vector<std::filesystem::path> cfgFiles;
    fs.enumerate("*.cfg", [&cfgFiles](const std::filesystem::path& p) {
        cfgFiles.push_back(p);
    });
    
    // Should find at least the two we created plus user_config.cfg
    EXPECT_GE(cfgFiles.size(), 3u);
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST(MemoryFileThreadSafetyTest, ConcurrentReads) {
    // Note: MemoryFile has internal thread-safety (locks), but position is shared state.
    // For truly concurrent reads from different positions, each thread needs its own file.
    // This test verifies that concurrent operations don't crash or corrupt data.
    
    constexpr int numThreads = 10;
    constexpr int opsPerThread = 100;
    
    // Each thread gets its own file copy (simulating real-world usage)
    std::vector<VirtualFilePtr> files;
    for (int i = 0; i < numThreads; ++i) {
        files.push_back(MemoryFile::fromString("Hello, World! This is test data for concurrent reads."));
    }
    
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&files, &successCount, i]() {
            auto& file = files[static_cast<size_t>(i)];
            for (int j = 0; j < opsPerThread; ++j) {
                file->seek(0, SeekOrigin::Begin);
                std::vector<Byte> buffer(10);
                auto result = file->read(buffer);
                if (result.has_value() && *result > 0) {
                    successCount++;
                }
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(successCount.load(), numThreads * opsPerThread);
}

TEST(FileSystemThreadSafetyTest, ConcurrentLocate) {
    // Create temp directory
    auto tempDir = std::filesystem::temp_directory_path() / "ezquake_fs_thread_test";
    auto baseDir = tempDir / "base";
    auto homeDir = tempDir / "home";
    
    std::filesystem::create_directories(baseDir / "qw");
    std::filesystem::create_directories(homeDir / "qw");
    
    // Create test files
    for (int i = 0; i < 10; ++i) {
        std::ofstream out(homeDir / "qw" / ("file" + std::to_string(i) + ".txt"));
        out << "content " << i;
    }
    
    FileSystem fs;
    fs.init(baseDir, homeDir, "qw");
    
    std::vector<std::thread> threads;
    std::atomic<int> foundCount{0};
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&fs, &foundCount, i]() {
            for (int j = 0; j < 50; ++j) {
                auto filename = "file" + std::to_string(i) + ".txt";
                if (fs.exists(filename)) {
                    foundCount++;
                }
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(foundCount.load(), 500);
    
    // Cleanup
    std::error_code ec;
    std::filesystem::remove_all(tempDir, ec);
}
