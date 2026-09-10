function getValue(id) {
    return document.getElementById(id).value.trim();
}

function patientFromForm() {
    return {
        name: getValue("name"),

        phone: [
            getValue("phone")
        ],

        address: getValue("address"),
        bloodGroup: getValue("bloodGroup"),
        condition: getValue("condition"),

        priority: document.getElementById("priority").value,
        gender: document.getElementById("gender").value,
        type: document.getElementById("type").value,

        age: Number(document.getElementById("age").value)
    };
}

function showMessage(message, success = true) {
    const element = document.getElementById("message");

    if (!element)
        return;

    element.textContent = message;
    element.className = success ? "success" : "error";
}

async function request(url, options = {}) {
    const response = await fetch(url, options);

    const text = await response.text();

    let data;

    try {
        data = text ? JSON.parse(text) : {};
    }
    catch {
        data = {
            raw: text
        };
    }

    if (!response.ok) {
        throw new Error(
            data.error ||
            data.message ||
            data.raw ||
            `HTTP ${response.status}`
        );
    }

    return data;
}


/*
 * REGISTER
 */

const registerForm = document.getElementById("register-form");

if (registerForm) {
    registerForm.addEventListener("submit", async (event) => {
        event.preventDefault();

        try {
            const patient = patientFromForm();

            const data = await request("/api/patients", {
                method: "POST",

                headers: {
                    "Content-Type": "application/json"
                },

                body: JSON.stringify(patient)
            });

            showMessage(
                `Patient registered successfully. ID: ${data.id}`
            );

            registerForm.reset();
        }
        catch (error) {
            showMessage(error.message, false);
        }
    });
}


/*
 * UPDATE
 */

const updateForm = document.getElementById("update-form");
const loadPatientButton = document.getElementById("load-patient");

async function loadPatient() {
    const id = getValue("patient-id");

    if (!id) {
        showMessage("Enter a patient ID.", false);
        return;
    }

    try {
        const data = await request(`/api/patients/${id}`);

        const patient = data.patient || data;

        document.getElementById("name").value =
            patient.name ?? "";

        document.getElementById("phone").value =
            Array.isArray(patient.phone)
                ? patient.phone.join(", ")
                : patient.phone ?? "";

        document.getElementById("address").value =
            patient.address ?? "";

        document.getElementById("bloodGroup").value =
            patient.bloodGroup ?? "";

        document.getElementById("condition").value =
            patient.condition ?? "";

        document.getElementById("priority").value =
            patient.priority ?? "LOW";

        document.getElementById("gender").value =
            patient.gender ?? "MALE";

        document.getElementById("type").value =
            patient.type ?? "APPOINTMENT";

        document.getElementById("age").value =
            patient.age ?? "";

        showMessage(`Patient ${id} loaded.`);
    }
    catch (error) {
        showMessage(error.message, false);
    }
}

if (loadPatientButton) {
    loadPatientButton.addEventListener("click", loadPatient);
}

if (updateForm) {
    updateForm.addEventListener("submit", async (event) => {
        event.preventDefault();

        const id = getValue("patient-id");

        if (!id) {
            showMessage("Enter a patient ID.", false);
            return;
        }

        try {
            const patient = patientFromForm();

            const data = await request(`/api/patients/${id}`, {
                method: "PUT",

                headers: {
                    "Content-Type": "application/json"
                },

                body: JSON.stringify(patient)
            });

            showMessage(
                data.message ||
                `Patient ${id} updated successfully.`
            );
        }
        catch (error) {
            showMessage(error.message, false);
        }
    });
}


/*
 * DISPLAY
 */

const displayForm = document.getElementById("display-form");

if (displayForm) {
    displayForm.addEventListener("submit", async (event) => {
        event.preventDefault();

        const id = getValue("patient-id");

        try {
            const data = await request(`/api/patients/${id}`);

            const patient = data.patient || data;

            document.getElementById("display-id").textContent =
                patient.id ?? id;

            document.getElementById("display-name").textContent =
                patient.name ?? "";

            document.getElementById("display-age").textContent =
                patient.age ?? "";

            document.getElementById("display-gender").textContent =
                patient.gender ?? "";

            document.getElementById("display-phone").textContent =
                Array.isArray(patient.phone)
                    ? patient.phone.join(", ")
                    : patient.phone ?? "";

            document.getElementById("display-blood").textContent =
                patient.bloodGroup ?? "";

            document.getElementById("display-priority").textContent =
                patient.priority ?? "";

            document.getElementById("display-type").textContent =
                patient.type ?? "";

            document.getElementById("display-address").textContent =
                patient.address ?? "";

            document.getElementById("display-condition").textContent =
                patient.condition ?? "";

            document.getElementById("display-json").textContent =
                JSON.stringify(patient, null, 4);

            document
                .getElementById("patient-result")
                .classList.remove("hidden");

            showMessage(`Patient ${id} retrieved.`);
        }
        catch (error) {
            document
                .getElementById("patient-result")
                .classList.add("hidden");

            showMessage(error.message, false);
        }
    });
}


/*
 * NEXT PATIENT
 */

async function handleNextPatient(type) {
    try {
        const data = await request(
            `/api/patients/next?type=${encodeURIComponent(type)}`,
            {
                method: "POST"
            }
        );

        const patient = data.patient || data;

        document.getElementById("next-id").textContent =
            patient.id ?? "";

        document.getElementById("next-name").textContent =
            patient.name ?? "";

        document.getElementById("next-age").textContent =
            patient.age ?? "";

        document.getElementById("next-gender").textContent =
            patient.gender ?? "";

        document.getElementById("next-phone").textContent =
            Array.isArray(patient.phone)
                ? patient.phone.join(", ")
                : patient.phone ?? "";

        document.getElementById("next-blood").textContent =
            patient.bloodGroup ?? "";

        document.getElementById("next-priority").textContent =
            patient.priority ?? "";

        document.getElementById("next-type").textContent =
            patient.type ?? "";

        document.getElementById("next-address").textContent =
            patient.address ?? "";

        document.getElementById("next-condition").textContent =
            patient.condition ?? "";

        document.getElementById("next-json").textContent =
            JSON.stringify(patient, null, 4);

        document
            .getElementById("next-patient")
            .classList.remove("hidden");

        showMessage(
            `Handling next ${type.toLowerCase()} patient.`
        );
    }
    catch (error) {
        document
            .getElementById("next-patient")
            .classList.add("hidden");

        showMessage(error.message, false);
    }
}

const nextAppointment =
    document.getElementById("next-appointment");

if (nextAppointment) {
    nextAppointment.addEventListener("click", () => {
        handleNextPatient("APPOINTMENT");
    });
}

const nextAdmission =
    document.getElementById("next-admission");

if (nextAdmission) {
    nextAdmission.addEventListener("click", () => {
        handleNextPatient("ADMISSION");
    });
}