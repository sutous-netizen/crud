#include <cstdio>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "DataPersistenceDemo.h"
#include "Json.h"

namespace {

    std::string TempDemoPath(const std::string& name) {
        return ::testing::TempDir() + name;
    }

} // namespace

TEST(DataPersistenceDemoTest, ReportsEmptyStoreBeforeAnyRecordExists) {
    std::string path = TempDemoPath("demo_initial_state.json");
    std::remove(path.c_str());

    std::ostringstream out;
    crudapp::RunDataPersistenceDemo(out, path);

    EXPECT_NE(out.str().find("record count: 0"), std::string::npos);

    std::remove(path.c_str());
}

TEST(DataPersistenceDemoTest, CreatedRecordsSurviveSimulatedRestart) {
    std::string path = TempDemoPath("demo_restart.json");
    std::remove(path.c_str());

    std::ostringstream out;
    crudapp::RunDataPersistenceDemo(out, path);

    std::string text = out.str();
    EXPECT_NE(text.find("created ids: 1, 2"), std::string::npos);
    EXPECT_NE(text.find("record count after restart: 2 (data survived via the file)"), std::string::npos);
}

TEST(DataPersistenceDemoTest, UpdateAndDeleteAreReflectedOnDisk) {
    std::string path = TempDemoPath("demo_update_delete.json");
    std::remove(path.c_str());

    std::ostringstream out;
    crudapp::RunDataPersistenceDemo(out, path);

    json::Value onDisk = json::Value::ParseFile(path);
    ASSERT_TRUE(onDisk.IsArray());
    ASSERT_EQ(onDisk.AsArray().size(), 1u);

    const json::Value& remaining = onDisk[static_cast<size_t>(0)];
    EXPECT_DOUBLE_EQ(remaining["id"].AsNumber(), 1.0);
    EXPECT_EQ(remaining["owner"].AsString(), "University Lab");

    std::remove(path.c_str());
}

TEST(DataPersistenceDemoTest, PrintsRawFileContentsAsTheFinalStep) {
    std::string path = TempDemoPath("demo_raw_contents.json");
    std::remove(path.c_str());

    std::ostringstream out;
    crudapp::RunDataPersistenceDemo(out, path);

    std::string text = out.str();
    EXPECT_NE(text.find("Raw file contents on disk"), std::string::npos);
    // The surviving record's fields should appear in the printed raw JSON.
    EXPECT_NE(text.find("Silicon Wafer"), std::string::npos);
    EXPECT_NE(text.find("University Lab"), std::string::npos);

    std::remove(path.c_str());
}

TEST(DataPersistenceDemoTest, RerunningResetsToTheSameOutcome) {
    std::string path = TempDemoPath("demo_rerun.json");
    std::remove(path.c_str());

    std::ostringstream firstRun;
    crudapp::RunDataPersistenceDemo(firstRun, path);
    json::Value firstResult = json::Value::ParseFile(path);

    std::ostringstream secondRun;
    crudapp::RunDataPersistenceDemo(secondRun, path);
    json::Value secondResult = json::Value::ParseFile(path);

    EXPECT_EQ(firstResult.ToString(), secondResult.ToString());

    std::remove(path.c_str());
}
