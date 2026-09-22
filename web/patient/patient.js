"use strict";

const PatientAPI = {
    async register(patient) {
        const response = await fetch("/api/patients", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(patient)
        });

        return await response.json();
    },

    async get(id) {
        const response = await fetch(`/api/patients/${id}`);

        return await response.json();
    },

    async update(id, patient) {
        const response = await fetch(`/api/patients/${id}`, {
            method: "PUT",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(patient)
        });

        return await response.json();
    },

    async next(type) {
        const response = await fetch(
            `/api/patients/next?type=${encodeURIComponent(type)}`,
            {
                method: "POST"
            }
        );

        return await response.json();
    }
};