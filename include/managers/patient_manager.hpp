#pragma once

#include "data/Patient.hpp"

namespace dhems
{
    class patient_manager
    {
        std::unordered_map<size_t, std::unique_ptr<patient>> patients;
        std::queue<patient*> regular_queue;
        std::vector<emergency_patient*> heap;

        void heapify_up(int index);
        void heapify_down(int index);

    public:
        void add_patient(std::unique_ptr<patient> p);

        void add_to_regular_queue(size_t patient_id);
        void add_to_heap(size_t patient_id);

        void remove_patient(size_t patient_id);
        patient* find_patient(size_t patient_id);
        
        patient* process_next_patient();
        emergency_patient* process_next_emergency_patient();

        patient* peek_next_patient();
        emergency_patient* peek_next_emergency_patient();
    };
}