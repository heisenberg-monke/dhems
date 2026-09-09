#include "Logger.hpp"
#include "dhems.hpp"

static constexpr const char *PATIENTS_PATH = "data/patients.json";

int main(int argc, char **argv)
{
    bool debug = false;

    for(int i = 1; i < argc; ++i)
    {
        std::string_view arg = argv[i];

        if(arg == "-d")
            debug = true;
    }

    dhems::Logger::setDebug(debug);
    auto &fs = dhems::FileSystem::fs();

    try
    {
        nlohmann::json patient1 =
        {
            {"name",       "John Doe"},
            {"phone",      {"9876543210", "9123456780"}},
            {"address",    "12 Main Street, Delhi"},
            {"bloodGroup", "O+"},
            {"condition",  "Fractured arm"},
            {"priority",   dhems::PatientPriority::MEDIUM},
            {"gender",     dhems::Gender::MALE},
            {"age",        35}
        };

        nlohmann::json patient2 =
        {
            {"name",       "Jane Smith"},
            {"phone",      {"9988776655"}},
            {"address",    "42 Park Avenue, Ghaziabad"},
            {"bloodGroup", "A-"},
            {"condition",  "Severe chest pain"},
            {"priority",   dhems::PatientPriority::SEVERE},
            {"gender",     dhems::Gender::FEMALE},
            {"age",        62}
        };

        nlohmann::json patient3 =
        {
            {"name",       "Rahul Kumar"},
            {"phone",      {"9812345678"}},
            {"address",    "7 Gandhi Road, Noida"},
            {"bloodGroup", "B+"},
            {"condition",  "High fever"},
            {"priority",   dhems::PatientPriority::HIGH},
            {"gender",     dhems::Gender::MALE},
            {"age",        24}
        };

        fs.load<dhems::Patient>(patient1.dump());
        fs.load<dhems::Patient>(patient2.dump());
        fs.load<dhems::Patient>(patient3.dump());

        dhems::Logger::display() << "Patients inserted:\n";

        for(const auto &[id, data] : fs.get<dhems::Patient>())
        {
            const auto &patient =
                static_cast<const dhems::Patient &>(*data);

            dhems::Logger::display()
                << "  ID: " << patient.id
                << " | Name: " << patient.name
                << " | Age: " << patient.age
                << '\n';
        }

        fs.save<dhems::Patient>(PATIENTS_PATH);

        dhems::Logger::display()
            << "\nSaved patients to: "
            << fs.resolveDir(PATIENTS_PATH)
            << '\n';

        std::vector<uint64_t> ids;

        for(const auto &[id, data] : fs.get<dhems::Patient>())
            ids.push_back(id);

        for(uint64_t id : ids)
            fs.erase<dhems::Patient>(id);

        dhems::Logger::display()
            << "\nAfter erase: "
            << fs.get<dhems::Patient>().size()
            << " patients in memory\n";


         fs.load<dhems::Patient>(std::filesystem::path(PATIENTS_PATH));

        dhems::Logger::display() << "\nPatients loaded from disk:\n";

        for(const auto &[id, data] : fs.get<dhems::Patient>())
        {
            const auto &patient =
                static_cast<const dhems::Patient &>(*data);

            dhems::Logger::display()
                << "  ID: " << patient.id
                << " | Name: " << patient.name
                << " | Age: " << patient.age
                << " | Blood: " << patient.bloodGroup
                << " | Priority: ";

            switch(patient.priority)
            {
                case dhems::PatientPriority::LOW:
                    dhems::Logger::display() << "LOW";
                    break;

                case dhems::PatientPriority::MEDIUM:
                    dhems::Logger::display() << "MEDIUM";
                    break;

                case dhems::PatientPriority::HIGH:
                    dhems::Logger::display() << "HIGH";
                    break;

                case dhems::PatientPriority::SEVERE:
                    dhems::Logger::display() << "SEVERE";
                    break;
            }
            dhems::Logger::display() << '\n';
        }

        const auto &patients = fs.get<dhems::Patient>();

        if(!patients.empty())
        {
            const uint64_t id = patients.begin()->first;

            auto &patient = fs.find<dhems::Patient>(id);

            dhems::Logger::display()
                << "\nfind(" << id << ") returned: "
                << patient.name
                << '\n';
        }

        dhems::Logger::display() << "\nAll tests passed.\n";
    }
    catch(const std::exception &e)
    {
        dhems::Logger::err()
            << "Test failed: "
            << e.what()
            << '\n';

        return 1;
    }

    return 0;
}