#include "DataPersistenceDemo.h"

#include <cstdio>
#include <fstream>
#include <sstream>

#include "JsonRecordStore.h"

namespace crudapp {

    namespace {

        void PrintAll(std::ostream& out, const IRecordStore& store) {
            for (const auto& record : store.All()) {
                out << "  " << record.ToString() << "\n";
            }
        }

        void PrintFileContents(std::ostream& out, const std::string& path) {
            std::ifstream in(path);
            std::stringstream buffer;
            buffer << in.rdbuf();
            out << buffer.str() << "\n";
        }

    } // namespace

    void RunDataPersistenceDemo(std::ostream& out, const std::string& filePath) {
        std::remove(filePath.c_str());

        out << "=== Data Persistence Demo (" << filePath << ") ===\n\n";

        out << "[1] Create a fresh store (no file yet -> empty store)\n";
        {
            JsonRecordStore store(filePath);
            out << "  record count: " << store.All().size() << "\n\n";

            out << "[2] Create records\n";
            json::Value r1 = json::Value::MakeObject();
            r1.Set("name", "Silicon Wafer");
            r1.Set("owner", "S-Semi Lab");
            int id1 = store.Create(r1);

            json::Value r2 = json::Value::MakeObject();
            r2.Set("name", "GaN Epitaxial");
            r2.Set("owner", "Fabless Corp");
            int id2 = store.Create(r2);

            out << "  created ids: " << id1 << ", " << id2 << "\n";
            out << "  current store contents:\n";
            PrintAll(out, store);
            out << "\n";
        } // store goes out of scope here, simulating the process exiting

        out << "[3] Simulate an app restart: open the same file in a new store instance\n";
        JsonRecordStore restarted(filePath);
        out << "  record count after restart: " << restarted.All().size()
            << " (data survived via the file)\n";
        PrintAll(out, restarted);
        out << "\n";

        out << "[4] Update / Delete\n";
        json::Value update = json::Value::MakeObject();
        update.Set("owner", "University Lab");
        restarted.Update(1, update);
        restarted.Delete(2);
        out << "  after updating id=1's owner and deleting id=2:\n";
        PrintAll(out, restarted);
        out << "\n";

        out << "[5] Raw file contents on disk (" << filePath << ")\n";
        PrintFileContents(out, filePath);
    }

} // namespace crudapp
