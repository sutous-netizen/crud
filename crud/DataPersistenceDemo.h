#pragma once

#include <ostream>

namespace crudapp {

    // Runs a scripted, non-interactive demonstration of the JSON-backed
    // persistence layer (JsonRecordStore): it creates records, simulates an
    // application restart by re-opening the same file in a fresh store
    // instance, and performs an update/delete, printing each step to `out`.
    void RunDataPersistenceDemo(std::ostream& out);

} // namespace crudapp
