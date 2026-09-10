const API = "/api/patients";


// --------------------------------------------------
// REGISTER PATIENT
// --------------------------------------------------

const registerForm = document.getElementById("register-form");

if (registerForm) {
    registerForm.addEventListener("submit", async (event) => {
        event.preventDefault();

        const result = document.getElementById("result");

        const patient =
        {
            name: document.getElementById("name").value,

            phone:
                [
                    document.getElementById("phone").value
                ],

            address: document.getElementById("address").value,

            bloodGroup:
                document.getElementById("bloodGroup").value,

            condition:
                document.getElementById("condition").value,

            priority:
                document.getElementById("priority").value,

            gender:
                document.getElementById("gender").value,

            age:
                Number(document.getElementById("age").value),

            type:
                document.getElementById("type").value
        };

        try {
            const response = await fetch(API,
                {
                    method: "POST",

                    headers:
                    {
                        "Content-Type": "application/json"
                    },

                    body: JSON.stringify(patient)
                });

            const data = await response.json();

            if (!response.ok)
                throw new Error(data.error || "Failed to register patient.");

            result.textContent =
                data.id
                    ? `Patient registered successfully. ID: ${data.id}`
                    : "Patient registered successfully.";

            result.className = "success";

            registerForm.reset();
        }
        catch (error) {
            result.textContent = error.message;
            result.className = "error";
        }
    });
}


// --------------------------------------------------
// UPDATE PATIENT
// --------------------------------------------------

const loadForm = document.getElementById("load-patient-form");

if (loadForm) {
    loadForm.addEventListener("submit", async (event) => {
        event.preventDefault();

        const id =
            document.getElementById("patient-id").value;

        const result =
            document.getElementById("result");

        try {
            const response =
                await fetch(`${API}/${id}`);

            const patient =
                await response.json();

            if (!response.ok)
                throw new Error(
                    patient.error || "Patient not found."
                );

            document.getElementById("name").value =
                patient.name;

            document.getElementById("phone").value =
                patient.phone?.[0] || "";

            document.getElementById("address").value =
                patient.address;

            document.getElementById("bloodGroup").value =
                patient.bloodGroup;

            document.getElementById("condition").value =
                patient.condition;

            document.getElementById("priority").value =
                patient.priority;

            document.getElementById("gender").value =
                patient.gender;

            document.getElementById("age").value =
                patient.age;

            document.getElementById("type").value =
                patient.type;

            document.getElementById("update-form")
                .classList.remove("hidden");

            result.textContent =
                "Patient loaded successfully.";

            result.className = "success";
        }
        catch (error) {
            result.textContent = error.message;
            result.className = "error";
        }
    });
}


const updateForm = document.getElementById("update-form");

if (updateForm) {
    updateForm.addEventListener("submit", async (event) => {
        event.preventDefault();

        const id =
            document.getElementById("patient-id").value;

        const result =
            document.getElementById("result");

        const patient =
        {
            name: document.getElementById("name").value,

            phone:
                [
                    document.getElementById("phone").value
                ],

            address:
                document.getElementById("address").value,

            bloodGroup:
                document.getElementById("bloodGroup").value,

            condition:
                document.getElementById("condition").value,

            priority:
                document.getElementById("priority").value,

            gender:
                document.getElementById("gender").value,

            age:
                Number(document.getElementById("age").value),

            type:
                document.getElementById("type").value
        };

        try {
            const response =
                await fetch(`${API}/${id}`,
                    {
                        method: "PUT",

                        headers:
                        {
                            "Content-Type":
                                "application/json"
                        },

                        body:
                            JSON.stringify(patient)
                    });

            const data =
                await response.json();

            if (!response.ok)
                throw new Error(
                    data.error || "Failed to update patient."
                );

            result.textContent =
                "Patient updated successfully.";

            result.className = "success";
        }
        catch (error) {
            result.textContent = error.message;
            result.className = "error";
        }
    });
}


// --------------------------------------------------
// DISPLAY PATIENT
// --------------------------------------------------

const displayForm =
    document.getElementById("display-form");

if (displayForm) {
    displayForm.addEventListener("submit", async (event) => {
        event.preventDefault();

        const id =
            document.getElementById("patient-id").value;

        const result =
            document.getElementById("result");

        try {
            const response =
                await fetch(`${API}/${id}`);

            const patient =
                await response.json();

            if (!response.ok)
                throw new Error(
                    patient.error || "Patient not found."
                );

            document.getElementById("patient-details")
                .classList.remove("hidden");

            document.getElementById("display-id")
                .textContent = patient.id;

            document.getElementById("display-name")
                .textContent = patient.name;

            document.getElementById("display-phone")
                .textContent =
                patient.phone?.join(", ") || "";

            document.getElementById("display-address")
                .textContent = patient.address;

            document.getElementById("display-bloodGroup")
                .textContent = patient.bloodGroup;

            document.getElementById("display-condition")
                .textContent = patient.condition;

            document.getElementById("display-priority")
                .textContent = patient.priority;

            document.getElementById("display-gender")
                .textContent = patient.gender;

            document.getElementById("display-age")
                .textContent = patient.age;

            document.getElementById("display-type")
                .textContent = patient.type;

            document.getElementById("raw-json")
                .textContent =
                JSON.stringify(patient, null, 2);

            result.textContent =
                "Patient loaded successfully.";

            result.className = "success";
        }
        catch (error) {
            result.textContent = error.message;
            result.className = "error";
        }
    });
}


// --------------------------------------------------
// NEXT PATIENT
// --------------------------------------------------

async function getNextPatient(type) {
    const result =
        document.getElementById("result");

    const patientCard =
        document.getElementById("next-patient");

    try {
        const response =
            await fetch(
                `${API}/next?type=${encodeURIComponent(type)}`,
                {
                    method: "POST"
                }
            );

        const patient =
            await response.json();

        if (!response.ok)
            throw new Error(
                patient.error || "No patient available."
            );

        patientCard.classList.remove("hidden");

        document.getElementById("next-id")
            .textContent = patient.id;

        document.getElementById("next-name")
            .textContent = patient.name;

        document.getElementById("next-type")
            .textContent = patient.type;

        document.getElementById("next-priority")
            .textContent = patient.priority;

        document.getElementById("next-condition")
            .textContent = patient.condition;

        result.textContent =
            `Next ${type.toLowerCase()} patient selected.`;

        result.className = "success";
    }
    catch (error) {
        patientCard.classList.add("hidden");

        result.textContent = error.message;
        result.className = "error";
    }
}


const appointmentButton =
    document.getElementById("next-appointment");

if (appointmentButton) {
    appointmentButton.addEventListener("click", () => {
        getNextPatient("APPOINTMENT");
    });
}


const admissionButton =
    document.getElementById("next-admission");

if (admissionButton) {
    admissionButton.addEventListener("click", () => {
        getNextPatient("ADMISSION");
    });
}