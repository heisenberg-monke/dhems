"use strict";

const svg = document.getElementById("map");
const destinationSelect = document.getElementById("destination");
const callButton = document.getElementById("call");
const message = document.getElementById("message");
const routePanel = document.getElementById("route-panel");
const mapSeed = document.getElementById("map-seed");
const ns = "http://www.w3.org/2000/svg";
let cityMap;
let selectedId = null;
let nodeElements = new Map();

function svgElement(tag, attrs = {}) {
    const el = document.createElementNS(ns, tag);
    for (const [key, value] of Object.entries(attrs)) el.setAttribute(key, value);
    return el;
}

function selectLocation(id) {
    selectedId = Number(id);
    destinationSelect.value = String(selectedId);
    callButton.disabled = false;
    for (const [nodeId, element] of nodeElements) {
        const selected = nodeId === selectedId;
        element.classList.toggle("selected", selected);
        const circle = element.querySelector("circle");
        circle.setAttribute("r", selected ? "19" : (nodeId === cityMap.hospital ? "15" : "11"));
    }
    routePanel.hidden = true;
    svg.querySelectorAll(".route").forEach(element => element.remove());
    message.textContent = `Destination selected: ${cityMap.nodes.find(node => node.id === selectedId).name}`;
}

function fitMapToContent() {
    const box = svg.getBBox();
    const pad = 42;
    svg.setAttribute("viewBox", `${box.x - pad} ${box.y - pad} ${box.width + pad * 2} ${box.height + pad * 2}`);
}

function renderMap(data) {
    cityMap = data;
    const byId = new Map(data.nodes.map(node => [node.id, node]));
    const roads = svgElement("g");
    for (const [fromId, toId] of data.roads) {
        const from = byId.get(fromId), to = byId.get(toId);
        roads.appendChild(svgElement("line", { x1: from.x, y1: from.y, x2: to.x, y2: to.y, class: "road", "data-from": fromId, "data-to": toId }));
    }
    svg.appendChild(roads);
    for (const node of data.nodes) {
        const group = svgElement("g", { class: `node${node.id === data.hospital ? " hospital" : ""}`, transform: `translate(${node.x} ${node.y})`, tabindex: node.id === data.hospital ? "-1" : "0", role: node.id === data.hospital ? "img" : "button", "aria-label": node.name });
        group.appendChild(svgElement("circle", { r: node.id === data.hospital ? 15 : 11 }));
        const label = svgElement("text", { x: 14, y: -14 });
        label.textContent = node.name;
        group.appendChild(label);
        if (node.id !== data.hospital) {
            group.addEventListener("click", () => selectLocation(node.id));
            group.addEventListener("keydown", event => { if (event.key === "Enter" || event.key === " ") { event.preventDefault(); selectLocation(node.id); } });
            const option = document.createElement("option"); option.value = node.id; option.textContent = node.name; destinationSelect.appendChild(option);
        }
        nodeElements.set(node.id, group);
        svg.appendChild(group);
    }
    fitMapToContent();
    mapSeed.textContent = `Seed: ${data.seed}`;
    message.textContent = `Ambulance starts at ${byId.get(data.hospital).name}. Choose a destination on the map or from the list.`;
}

async function initialize() {
    try {
        const response = await fetch("/api/city-map");
        if (!response.ok) throw new Error("Could not load city map.");
        renderMap(await response.json());
    } catch (error) { message.className = "error"; message.textContent = error.message; mapSeed.textContent = "Seed: unavailable"; }
}

destinationSelect.addEventListener("change", () => {
    if (destinationSelect.value) selectLocation(destinationSelect.value);
    else { selectedId = null; callButton.disabled = true; }
});

callButton.addEventListener("click", async () => {
    if (selectedId === null) return;
    callButton.disabled = true;
    message.textContent = "Finding the fastest road route…";
    try {
        const response = await fetch("/api/ambulance", { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify({ locationId: selectedId }) });
        const result = await response.json();
        if (!response.ok) throw new Error(result.error || "Ambulance request failed.");
        svg.querySelectorAll(".route").forEach(element => element.remove());
        for (let i = 1; i < result.route.length; ++i) {
            const a = result.route[i - 1], b = result.route[i];
            const line = svgElement("line", { x1: a.x, y1: a.y, x2: b.x, y2: b.y, class: "route" });
            svg.insertBefore(line, svg.querySelector(".node"));
        }
        document.getElementById("route-summary").textContent = `Ambulance dispatched from ${result.hospital.name} to ${result.destination.name}. Route length: ${result.distance.toFixed(0)} map units.`;
        const steps = document.getElementById("route-steps"); steps.replaceChildren();
        result.route.forEach(node => { const item = document.createElement("li"); item.textContent = node.name; steps.appendChild(item); });
        routePanel.hidden = false;
        message.textContent = "Ambulance route calculated.";
    } catch (error) { message.className = "error"; message.textContent = error.message; }
    finally { callButton.disabled = false; }
});

initialize();
