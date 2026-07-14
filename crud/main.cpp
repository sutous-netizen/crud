#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ConsoleApp.h"
#include "JsonRecordStore.h"

int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--test") {
        ::testing::InitGoogleMock(&argc, argv);
        return RUN_ALL_TESTS();
    }

    crudapp::JsonRecordStore store("data.json");
    crudapp::ConsoleApp app(store);
    return app.Run();
}
