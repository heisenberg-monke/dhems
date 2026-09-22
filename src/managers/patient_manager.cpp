#include "managers/patient_manager.hpp"

#include "logger.hpp"

namespace dhems
{
    bool priority_compare(const emergency_patient* a, const emergency_patient* b)
    {
        if(a->sev > b->sev)
            return true;

        if(a->sev == b->sev)
            return a->patient_id < b->patient_id;

        return false;
    }

    void patient_manager::heapify_up(int index)
    {
        while(index > 0)
        {
            int parent = (index - 1) / 2;

            if(priority_compare(this->heap[index], this->heap[parent]))
            {
                std::swap(this->heap[index], this->heap[parent]);
                index = parent;
            }

            else
                break;
        }
    }

    void patient_manager::heapify_down(int index)
    {
        int n = this->heap.size();

        while(true)
        {
            int leftChild = 2 * index + 1;
            int rightChild = 2 * index + 2;

            int highest = index;

            if(leftChild < n && priority_compare(this->heap[leftChild], this->heap[highest]))
                highest = leftChild;

            if(rightChild < n && priority_compare(this->heap[rightChild], this->heap[highest]))
                highest = rightChild;

            if(highest == index)
                break;

            std::swap(this->heap[index], this->heap[highest]);
            index = highest;
        }
    }    

    void patient_manager::add_patient(std::unique_ptr<patient> p)
    {
        size_t id = p->patient_id;

        if(!patients.try_emplace(id, std::move(p)).second)
            throw std::runtime_error(std::format("Patient ID {} already exists.", id));

        LOG_INFO(std::format("Patient {} added successfully.\n", id));
    }

    void patient_manager::remove_patient(size_t patient_id)
    {
        if(!patients.erase(patient_id))
            throw std::runtime_error(std::format("Patient {} not found.", patient_id));

        LOG_INFO(std::format("Patient {} removed successfully.", patient_id));
    }

    patient* patient_manager::find_patient(size_t patient_id)
    {
        auto it = patients.find(patient_id);

        if(it == patients.end())
            throw std::runtime_error(std::format("Patient {} not found.", patient_id));

        return it->second.get();
    }

    void patient_manager::add_to_regular_queue(size_t patient_id)
    {
        regular_queue.push(find_patient(patient_id));
        LOG_INFO(std::format("Patient {} added to the regular queue.", patient_id));
    }

    void patient_manager::add_to_heap(size_t patient_id)
    {
        this->heap.push_back(static_cast<emergency_patient*>(this->find_patient(patient_id)));
        this->heapify_up(this->heap.size() - 1);

        LOG_INFO(std::format("Emergency patient {} added successfully.\n", patient_id));
    }

    patient* patient_manager::process_next_patient()
    {
        if(regular_queue.empty())
            throw std::runtime_error("The regular queue is empty.");

        auto* p = regular_queue.front();
        regular_queue.pop();

        LOG_INFO("Processing next patient...\n");

        return p;
    }

    emergency_patient* patient_manager::process_next_emergency_patient()
    {
        if(this->heap.empty())
            throw std::runtime_error("The emergency queue is empty.");

        auto *highest = this->heap[0];

        this->heap[0] = this->heap.back();
        this->heap.pop_back();

        if(!this->heap.empty())
            this->heapify_down(0);

        return highest;
    }

    patient* patient_manager::peek_next_patient()
    {
        if(this->regular_queue.empty())
            throw std::runtime_error("The regular queue is empty.");

        return this->regular_queue.front();
    }
    emergency_patient* patient_manager::peek_next_emergency_patient()
    {
        if(this->heap.empty())
            throw std::runtime_error("The emergency queue is empty.");

        return this->heap.front();
    }
}