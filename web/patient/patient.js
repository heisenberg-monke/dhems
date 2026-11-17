"use strict";

const PatientAPI = {
    async list() {
        return await (await fetch("/api/patients")).json();
    },

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

    async next() {
        return await (await fetch("/api/patients/next", { method: "POST" })).json();
    },

    async erase(id) {
        return await (await fetch(`/api/patients/${encodeURIComponent(id)}`, { method: "DELETE" })).json();
    }
};
