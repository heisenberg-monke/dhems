# DHEMS

**Digital Hospital and Emergency Management System** is a C++23 web application for managing hospital patients and doctors, with an ambulance dispatch page backed by a generated city road network.

## Features

- Register, search, list, update, and discharge patients.
- Add, list, search, and remove doctors; assign doctors to admitted patients.
- Generate a seeded 21 × 21 city map with named neighborhoods, local streets, diagonal connections, and longer avenues.
- Select a destination and compare routes calculated by A* and Dijkstra. The page displays both paths and their calculation times.
- Save patient, doctor, and generated city map data under `data/` when the server shuts down.

## Requirements

- CMake 3.30 or newer
- A C++23 compiler
- Ninja (for the commands below)

The HTTP server and JSON library are included in `external/`.

## Build and run

From the repository root:

```sh
cmake -S . -B build -G Ninja
cmake --build build
./build/dhems
```

The server listens on `http://localhost:6769`. Open that address to use the patient, doctor, and ambulance pages.

By default, the city map uses a seed based on the current system time. Supply a seed to generate the same map again:

```sh
./build/dhems --seed 12345
```

Stop the server with Ctrl+C so it can save its data files.

## Ambulance map

Open **Call an ambulance** from the home page, choose a destination from the list or map, then request a route. The map shows the A* route as a solid red line and the Dijkstra route as a dashed blue line. Hover over a location to reveal its name. The route details list the stops and the page footer reports each algorithm’s calculation time in milliseconds.

## HTTP API

| Method | Endpoint | Purpose |
| --- | --- | --- |
| `GET` | `/api/patients` | List patients |
| `POST` | `/api/patients` | Register a patient |
| `GET` | `/api/patients/{id}` | Get a patient |
| `PUT` | `/api/patients/{id}` | Update/discharge a patient |
| `DELETE` | `/api/patients/{id}` | Remove a patient |
| `POST` | `/api/patients/next` | Appoint the next patient |
| `GET` | `/api/doctors` | List doctors |
| `POST` | `/api/doctors` | Add a doctor |
| `GET` | `/api/doctors/{id}` | Get a doctor |
| `DELETE` | `/api/doctors/{id}` | Remove a doctor |
| `POST` | `/api/doctors/release` | Release a doctor from a patient |
| `GET` | `/api/city-map` | Get the generated map |
| `POST` | `/api/ambulance` | Calculate both routes for `{ "locationId": <id> }` |

## Data files

Runtime data is stored in `data/patients.json`, `data/doctors.json`, and `data/city_map.json`. The data directory is created as needed. These files are ignored by Git.
