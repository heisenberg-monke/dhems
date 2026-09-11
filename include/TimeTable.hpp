#pragma once

#include "HospitalData.hpp"

#include <array>
#include <optional>

namespace dhems
{
    enum class Day
    {
        MONDAY,
        TUESDAY,
        WEDNESDAY,
        THURSDAY,
        FRIDAY,
        SATURDAY,
        SUNDAY
    };

    template <typename Slot>
    class TimeTable : public HospitalData
    {
        std::array<std::optional<Slot>, 24 * 7> m_slots;

        static constexpr int timeSlot(Day day, int time)
        {
            const int dayIndex = static_cast<int>(day);

            assert(0 <= time && time <= 24);
            assert(0 <= dayIndex && dayIndex <= 7);

            return dayIndex * 7 + time;
        }

    public:
        void load(const nlohmann::json &j) override;
        void save(nlohmann::json &j) const override;

        inline void addEntry(Slot &&slot, Day day, int time) {
            m_slots[timeSlot(day, time)] = std::move(slot);
        }

        inline void removeEntry(Day day, int time) {
            m_slots[timeSlot(day, time)].reset();
        }
    };
}