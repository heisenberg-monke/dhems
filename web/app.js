const form = document.getElementById("patient-form");
const result = document.getElementById("result");

form.addEventListener("submit", async (event) => {
    event.preventDefault();

    const patient = {
        name: document.getElementById("name").value,
        phone: [
            document.getElementById("phone").value
        ],
        address: document.getElementById("address").value,
        bloodGroup: document.getElementById("bloodGroup").value,
        condition: document.getElementById("condition").value,
        priority: document.getElementById("priority").value,
        gender: document.getElementById("gender").value,
        age: Number(document.getElementById("age").value)
    };

    try {
        const response = await fetch("/api/patients", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(patient)
        });

        const text = await response.text();

        console.log("HTTP:", response.status);
        console.log("Raw response:", text);

        const data = JSON.parse(text);

        console.log("Parsed response:", data);
        console.log("Server-generated ID:", data.id);

        if (!response.ok) {
            throw new Error(data.error || "Request failed");
        }

        result.textContent =
            `Patient added successfully. ID: ${data.id}`;

        form.reset();
    }
    catch (error) {
        console.error(error);
        result.textContent = `Error: ${error.message}`;
    }
});