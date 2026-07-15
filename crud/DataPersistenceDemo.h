#pragma once

#include <ostream>
#include <string>

namespace crudapp {

    // Runs a scripted, non-interactive demonstration of the JSON-backed
    // persistence layer (JsonRecordStore): it creates records, simulates an
    // application restart by re-opening the same file in a fresh store
    // instance, and performs an update/delete, printing each step to `out`.
    // `filePath` is reset (any existing file removed) before the demo runs;
    // it defaults to the path main.cpp uses for `crud.exe --demo`, but tests
    // can pass a temp-directory path to run in isolation.
    void RunDataPersistenceDemo(std::ostream& out, const std::string& filePath = "demo_data.json");

} // namespace crudapp
