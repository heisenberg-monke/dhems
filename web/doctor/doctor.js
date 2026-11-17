"use strict";

const DoctorAPI = {
    async list() {
        const response = await fetch("/api/doctors");
        if (!response.ok) throw new Error(`Doctor list failed (${response.status}).`);
        return response.json();
    },
    async get(id) {
        const response = await fetch(`/api/doctors/${encodeURIComponent(id)}`);
        const result = await response.json();
        if (!response.ok) throw new Error(result.error || `Doctor lookup failed (${response.status}).`);
        return result;
    },
    async register(doctor) {
        const response = await fetch("/api/doctors", { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify(doctor) });
        const result = await response.json();
        if (!response.ok) throw new Error(result.error || `Registration failed (${response.status}).`);
        return result;
    },
    async erase(id) {
        const response = await fetch(`/api/doctors/${encodeURIComponent(id)}`, { method: "DELETE" });
        const result = await response.json();
        if (!response.ok) throw new Error(result.error || `Erase failed (${response.status}).`);
        return result;
    }
};

const departmentNames = ["Ortho", "Gyno", "Cardio", "Neuro"];
const tbody = document.getElementById("doctors");
const listMessage = document.getElementById("list-message");

function addDoctorRow(doctor) {
    const row = document.createElement("tr");
    for (const value of [doctor.id, doctor.name, departmentNames[doctor.department] || "Unknown", doctor.busy ? "Busy" : "Available", doctor.patientName || "Unassigned"]) {
        const cell = document.createElement("td");
        cell.textContent = value;
        row.appendChild(cell);
    }
    const action = document.createElement("td");
    if (!doctor.busy && !doctor.patientID) {
        const erase = document.createElement("button");
        erase.type = "button";
        erase.className = "button danger";
        erase.textContent = "Erase";
        erase.addEventListener("click", async () => {
            try { await DoctorAPI.erase(doctor.id); await loadDoctors(); }
            catch (error) { listMessage.className = "error"; listMessage.textContent = error.message; }
        });
        action.appendChild(erase);
    } else {
        action.textContent = "Assigned";
    }
    row.appendChild(action);
    tbody.appendChild(row);
}

async function loadDoctors() {
    tbody.replaceChildren();
    try {
        const doctors = await DoctorAPI.list();
        listMessage.textContent = doctors.length ? "" : "No doctors have been registered.";
        doctors.sort((a, b) => a.id - b.id).forEach(addDoctorRow);
    } catch (error) {
        listMessage.className = "error";
        listMessage.textContent = error.message;
    }
}

document.getElementById("doctor-form").addEventListener("submit", async event => {
    event.preventDefault();
    const form = event.currentTarget;
    const data = new FormData(form);
    const message = document.getElementById("form-message");
    try {
        const result = await DoctorAPI.register({ name: data.get("name"), department: Number(data.get("department")) });
        message.className = "success";
        message.textContent = `Doctor registered. ID: ${result.id}`;
        form.reset();
        await loadDoctors();
    } catch (error) {
        message.className = "error";
        message.textContent = error.message;
    }
});

document.getElementById("search-form").addEventListener("submit", async event => {
    event.preventDefault();
    const id = new FormData(event.currentTarget).get("id");
    const output = document.getElementById("search-message");
    try {
        const doctor = await DoctorAPI.get(id);
        output.className = "success";
        output.textContent = `${doctor.name} · ${departmentNames[doctor.department] || "Unknown"} · ${doctor.busy ? `Assigned to ${doctor.patientName}` : "Available"}`;
    } catch (error) {
        output.className = "error";
        output.textContent = error.message;
    }
});

loadDoctors();
