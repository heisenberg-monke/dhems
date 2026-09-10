const form = document.getElementById("patient-form");
const result = document.getElementById("result");
const patientsElement = document.getElementById("patients");
const debugElement = document.getElementById("debug");

function setStatus(message, success = true) {
    result.textContent = message;
    result.className = `status ${success ? "success" : "error"}`;
}

function showDebug(data) {
    debugElement.textContent =
        typeof data === "string"
            ? data
            : JSON.stringify(data, null, 2);
}

function getPatientFromForm() {
    return {
        name: document.getElementById("name").value.trim(),
        phone: [
            document.getElementById("phone").value.trim()
        ],
        address: document.getElementById("address").value.trim(),
        bloodGroup: document.getElementById("bloodGroup").value.trim(),
        condition: document.getElementById("condition").value.trim(),
        priority: document.getElementById("priority").value,
        type: document.getElementById("type").value,
        gender: document.getElementById("gender").value,
        age: Number(document.getElementById("age").value)
    };
}

async function request(url, options = {}) {
    const response = await fetch(url, options);
    const text = await response.text();

    let data;

    try {
        data = text ? JSON.parse(text) : {};
    } catch {
        data = { raw: text };
    }

    showDebug({
        status: response.status,
        response: data
    });

    if (!response.ok) {
        throw new Error(data.error || data.raw || "Request failed");
    }

    return data;
}

form.addEventListener("submit", async (event) => {
    event.preventDefault();

    try {
        const patient = getPatientFromForm();

        const data = await request("/api/patients", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(patient)
        });

        setStatus(`Patient registered successfully. ID: ${data.id}`);
        form.reset();

        await loadPatients();
    } catch (error) {
        console.error(error);
        setStatus(`Error: ${error.message}`, false);
    }
});

async function loadPatients() {
    try {
        const data = await request("/api/patients");

        patientsElement.innerHTML = "";

        if (!data.patients || data.patients.length === 0) {
            patientsElement.innerHTML = "<p>No patients registered.</p>";
            return;
        }

        for (const patient of data.patients) {
            const card = document.createElement("div");
            card.className = "patient-card";

            card.innerHTML = `
                <strong>${escapeHtml(patient.name)}</strong>
                <div class="patient-meta">
                    ID: ${patient.id}<br>
                    Age: ${patient.age}<br>
                    Gender: ${escapeHtml(patient.gender)}<br>
                    Blood Group: ${escapeHtml(patient.bloodGroup)}<br>
                    Condition: ${escapeHtml(patient.condition)}<br>
                    Visit: ${escapeHtml(patient.type)}
                </div>
                <span class="badge">
                    ${escapeHtml(patient.priority)}
                </span>
            `;

            patientsElement.appendChild(card);
        }
    } catch (error) {
        patientsElement.innerHTML =
            `<p class="status error">${escapeHtml(error.message)}</p>`;
    }
}

async function getNextPatient(type) {
    try {
        const data = await request(`/api/patients/next?type=${type}`, {
            method: "POST"
        });

        setStatus(
            `Next ${type.toLowerCase()} patient: ${data.patient.name} (ID ${data.patient.id})`
        );

        await loadPatients();
    } catch (error) {
        setStatus(`Error: ${error.message}`, false);
    }
}

document.getElementById("refresh").addEventListener("click", loadPatients);

document.getElementById("next-appointment").addEventListener("click", () => {
    getNextPatient("APPOINTMENT");
});

document.getElementById("next-admission").addEventListener("click", () => {
    getNextPatient("ADMISSION");
});

document.getElementById("clear-form").addEventListener("click", () => {
    form.reset();
    result.textContent = "";
    result.className = "status";
});

function escapeHtml(value) {
    return String(value)
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
        .replaceAll("'", "&#039;");
}

loadPatients();