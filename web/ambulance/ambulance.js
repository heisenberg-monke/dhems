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
let labelElements = new Map();
let routeLayer;

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
        labelElements.get(nodeId)?.classList.toggle("visible", selected || nodeId === cityMap.hospital);
        const circle = element.querySelector("circle");
        circle.setAttribute("r", selected ? "19" : (nodeId === cityMap.hospital ? "15" : "11"));
    }
    routePanel.hidden = true;
    if (routeLayer) routeLayer.replaceChildren();
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
    routeLayer = svgElement("g", { class: "route-layer", "aria-hidden": "true" });
    svg.appendChild(routeLayer);
    nodeElements.clear();
    labelElements.clear();
    const nodesLayer = svgElement("g", { class: "nodes-layer" });
    for (const node of data.nodes) {
        const group = svgElement("g", { class: `node${node.id === data.hospital ? " hospital" : ""}`, transform: `translate(${node.x} ${node.y})`, tabindex: node.id === data.hospital ? "-1" : "0", role: node.id === data.hospital ? "img" : "button", "aria-label": node.name });
        group.appendChild(svgElement("circle", { r: node.id === data.hospital ? 15 : 11 }));
        const label = svgElement("text", { x: node.x + 14, y: node.y - 14, class: `map-label${node.id === data.hospital ? " visible" : ""}` });
        label.textContent = node.name;
        labelElements.set(node.id, label);
        group.addEventListener("pointerenter", () => label.classList.add("visible"));
        group.addEventListener("pointerleave", () => {
            if (node.id !== selectedId && node.id !== data.hospital) label.classList.remove("visible");
        });
        group.addEventListener("focus", () => label.classList.add("visible"));
        group.addEventListener("blur", () => {
            if (node.id !== selectedId && node.id !== data.hospital) label.classList.remove("visible");
        });
        if (node.id !== data.hospital) {
            group.addEventListener("click", () => selectLocation(node.id));
            group.addEventListener("keydown", event => { if (event.key === "Enter" || event.key === " ") { event.preventDefault(); selectLocation(node.id); } });
            const option = document.createElement("option"); option.value = node.id; option.textContent = node.name; destinationSelect.appendChild(option);
        }
        nodeElements.set(node.id, group);
        nodesLayer.appendChild(group);
    }
    svg.appendChild(nodesLayer);
    const labelsLayer = svgElement("g", { class: "labels-layer", "aria-hidden": "true" });
    for (const label of labelElements.values()) labelsLayer.appendChild(label);
    svg.appendChild(labelsLayer);
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
        if (!result.astar?.route || !result.dijkstra?.route) {
            throw new Error("The server is using an older ambulance API. Restart it with the latest build, then reload this page.");
        }
        routeLayer.replaceChildren();
        const drawRoute = (route, className) => {
            for (let i = 1; i < route.length; ++i) {
                const a = route[i - 1], b = route[i];
                routeLayer.appendChild(svgElement("line", {
                    x1: a.x, y1: a.y, x2: b.x, y2: b.y,
                    class: `route ${className}`, "data-from": a.id, "data-to": b.id
                }));
            }
        };
        drawRoute(result.astar.route, "astar-route");
        drawRoute(result.dijkstra.route, "dijkstra-route");
        document.getElementById("route-summary").textContent = `Routes from ${result.hospital.name} to ${result.destination.name}.`;
        const renderSteps = (id, route) => {
            const steps = document.getElementById(id); steps.replaceChildren();
            route.forEach(node => { const item = document.createElement("li"); item.textContent = node.name; steps.appendChild(item); });
        };
        renderSteps("astar-steps", result.astar.route);
        renderSteps("dijkstra-steps", result.dijkstra.route);
        const times = document.getElementById("algorithm-times");
        times.textContent = `Route calculation time — A*: ${result.astar.timeMs.toFixed(3)} ms; Dijkstra: ${result.dijkstra.timeMs.toFixed(3)} ms.`;
        times.hidden = false;
        routePanel.hidden = false;
        message.textContent = "Ambulance route calculated.";
    } catch (error) { message.className = "error"; message.textContent = error.message; }
    finally { callButton.disabled = false; }
});

initialize();
