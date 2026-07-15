#include <iostream>
#include <string>

#ifdef _DEBUG
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#endif

#include "ConsoleApp.h"
#include "DataPersistenceDemo.h"
#include "JsonRecordStore.h"

int main(int argc, char** argv) {
#ifdef _DEBUG
    if (argc > 1 && std::string(argv[1]) == "--test") {
        ::testing::InitGoogleMock(&argc, argv);
        return RUN_ALL_TESTS();
    }
#endif

    if (argc > 1 && std::string(argv[1]) == "--demo") {
        crudapp::RunDataPersistenceDemo(std::cout);
        return 0;
    }

    crudapp::JsonRecordStore store("data.json");
    crudapp::ConsoleApp app(store);
    return app.Run();
}
