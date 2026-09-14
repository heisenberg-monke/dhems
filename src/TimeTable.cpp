#include "TimeTable.hpp"

namespace dhems
{
    NLOHMANN_JSON_SERIALIZE_ENUM(Day, {
        {Day::MONDAY, "MONDAY"},
        {Day::TUESDAY, "TUESDAY"},
        {Day::WEDNESDAY, "WEDNESDAY"},
        {Day::THURSDAY, "THURSDAY"},
        {Day::FRIDAY, "FRIDAY"},
        {Day::SATURDAY, "SATURDAY"},
        {Day::SUNDAY, "SUNDAY"}
    })

    template <typename Slot>
    void TimeTable<Slot>::load(const nlohmann::json &j)
    {
        if(!j.is_array())
            throw std::runtime_error("TimeTable::load: Expected an array");
        m_slots.fill(std::nullopt);
        for(const auto &entry : j)
        {
            const Day day = entry.at("day").get<Day>();
            const int time = entry.at("time").get<int>();
            if(time < 0 || time > 24)
                throw std::runtime_error("TimeTable::load: Invalid time.");
            m_slots[timeSlot(day, time)] = entry.at("slot").get<Slot>();
        }
    }

    template <typename Slot>
    void TimeTable<Slot>::save(nlohmann::json &j) const
    {
        j = nlohmann::json::array();
        for(Day day = Day::MONDAY; day <= Day::SUNDAY; day = Day(int(day)+1))
        {
            for(size_t time = 0; time < 24; ++time)
            {
                const auto &slot = m_slots[timeSlot(day, time)];
                if(!slot)
                    continue;
                j.push_back({
                    {"day", day},
                    {"time", time},
                    {"slot", slot}
                });
            }
        }
    }
}