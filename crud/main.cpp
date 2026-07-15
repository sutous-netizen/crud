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
    // Debug 빌드는 TC 검증이 목적이므로, 인자 없이 실행해도 항상 전체 테스트를 수행한다.
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
#else
    if (argc > 1 && std::string(argv[1]) == "--demo") {
        crudapp::RunDataPersistenceDemo(std::cout);
        return 0;
    }

    crudapp::JsonRecordStore store("data.json");
    crudapp::ConsoleApp app(store);
    return app.Run();
#endif
}
