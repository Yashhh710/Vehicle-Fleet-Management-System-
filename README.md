# 🚖 FleetRoute — Smart Fleet & Trip Dispatch System

[![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)]()
[![Status](https://img.shields.io/badge/Status-Active-brightgreen.svg)]()

---

## 1 Project Title

**FleetRoute — Smart Fleet Management & Trip Dispatch System using DSA**

---

## 2 Problem Statement

Urban transportation networks face a critical challenge: efficiently dispatching vehicles to passengers across a city graph while minimising travel distance and managing trip priorities. Manual assignment is error-prone and slow. This project addresses the problem by modelling a city as a weighted undirected graph and applying core Data Structures & Algorithms to automate vehicle dispatch, route optimisation, and fleet tracking — simulating a real-world cab/logistics dispatch backend.

---

## 3 Objectives

- Model a city road network as a weighted graph loaded from an external file.
- Implement **Dijkstra's algorithm** to find the shortest path between any two cities.
- Use a **min-heap priority queue** to manage trips by urgency (priority-based dispatching).
- Maintain a **fleet of vehicles**, track their locations, availability, and trip counts.
- Provide **rollback functionality** using a stack that restores the full prior state of a vehicle after an undone dispatch.
- Persist all fleet and trip data across sessions using file I/O (`data.txt`).
- Display a clean, column-aligned vehicle dashboard.

---

## 4 System Overview / Architecture

```
┌─────────────────────────────────────────────────┐
│                  User (CLI Menu)                │
└───────────────────────┬─────────────────────────┘
                        │
          ┌─────────────▼──────────────┐
          │       Main Controller      │
          │  (menu loop, input/output) │
          └──┬──────────┬──────────────┘
             │          │
   ┌──────────▼──┐  ┌───▼──────────────┐
   │  City Graph │  │   Fleet Manager  │
   │  (Adjacency │  │  (vector<Vehicle>│
   │   List)     │  │   + hash map)    │
   └──────┬──────┘  └───────┬──────────┘
          │                 │
   ┌──────▼──────┐   ┌──────▼──────────┐
   │  Dijkstra   │   │  Trip Queue     │
   │ (min-heap   │   │ (priority_queue │
   │  shortest   │   │  min-heap)      │
   │  path)      │   └──────┬──────────┘
   └─────────────┘          │
                     ┌──────▼──────────┐
                     │  Rollback Stack │
                     │ (VehicleSnapshot│
                     │  stack)         │
                     └─────────────────┘
                     
   ┌──────────────────────────────────────┐
   │         File I/O Layer               │
   │  cities.txt (graph)  data.txt (state)│
   └──────────────────────────────────────┘
```

**Flow for Trip Dispatch:**
1. Highest-priority trip is popped from the min-heap.
2. Dijkstra runs from the trip's pickup city.
3. The nearest available vehicle is selected.
4. A `VehicleSnapshot` is pushed onto the rollback stack.
5. Vehicle state is updated (location, trip count, availability).
6. Results are displayed and saved to `data.txt`.

---

## 5 Data Structures and Algorithms Used

| Component | DSA Used | Purpose |
|-----------|----------|---------|
| City Graph | Adjacency List (`vector<vector<pair<int,int>>>`) | Represent roads between cities with distances |
| Shortest Path | **Dijkstra's Algorithm** with Min-Heap | Find nearest vehicle and trip route distance |
| Trip Scheduling | **Priority Queue** (min-heap) | Dispatch highest-priority trip first |
| Rollback | **Stack** (`stack<VehicleSnapshot>`) | Undo last dispatch, restore full vehicle state |
| Fleet Lookup | **Hash Map** (`unordered_map<int,int>`) | O(1) vehicle lookup by ID |
| Vehicle Sorting | **Sort** (`std::sort` with lambda) | Display vehicles sorted by trip count |
| Vehicle Search | **Binary Search** | Search vehicle by ID in sorted ID list |
| Persistence | File I/O (ifstream / ofstream) | Save and reload fleet + trip state across runs |

---

## 6 Implementation Approach

### Graph Loading (`loadCities`)
Cities and edges are read from `cities.txt` in two sections (`CITIES`, `EDGES`). The graph is built as an undirected adjacency list.

### Dijkstra's Algorithm
Standard single-source shortest path using a `priority_queue<pair<int,int>, vector<...>, greater<>>` (min-heap). Returns a distance array from the source city to all other cities.

### Trip Dispatch (`dispatchTrip`)
- Pops the top (highest-priority = lowest int value) trip from the priority queue.
- Runs Dijkstra once from the pickup city.
- Iterates all vehicles to find the one with minimum `dist[vehicle.location]`.
- Saves a `VehicleSnapshot` before modifying state.
- Updates vehicle: location ← dropoff, trips++, available = true.

### Rollback (`rollback`)
Pops the top `VehicleSnapshot` and restores the vehicle's `location`, `trips`, and `available` fields to their pre-dispatch values.

### Persistence (`saveData` / `loadData`)
On every state-changing operation, `saveData()` rewrites `data.txt` with the current fleet and pending trips. `loadData()` is called once at startup to restore the previous session.

---

## 7 Time and Space Complexity Analysis

| Operation | Time Complexity | Space Complexity |
|-----------|----------------|-----------------|
| Load cities/edges | O(V + E) | O(V + E) |
| Dijkstra | O((V + E) log V) | O(V) |
| Add trip to queue | O(log T) | O(T) |
| Dispatch trip | O((V + E) log V + F) | O(V + F) |
| Rollback | O(1) | O(D) — depth of rollback stack |
| Search vehicle | O(F log F) | O(F) |
| Show vehicles | O(F log F) | O(F) |
| Save data | O(F + T) | O(1) |

> V = cities, E = edges, F = fleet size, T = trip queue size, D = dispatches since last save

---

## 8 Execution Steps

### Prerequisites
- A C++ compiler supporting C++11 or later (g++, clang++)
- Terminal / command prompt

### 1. Clone the Repository
```bash
git clone https://github.com/<your-username>/FleetRoute.git
cd FleetRoute
```

### 2. Compile
```bash
g++ code.cpp -o fleet
# or with explicit C++17:
g++ -std=c++17 code.cpp -o fleet
```

### 3. Ensure Data Files Are Present
Place `cities.txt` and `data.txt` in the **same directory** as the compiled binary.  
If `cities.txt` is missing, the program auto-generates a default one.

### 4. Run
```bash
./fleet        # Linux / macOS
fleet.exe      # Windows
```

### 5. Navigate the Menu
```
===== Fleet Management =====
1. Add Vehicle
2. Show Vehicles
3. Add Trip
4. Dispatch Trip
5. Shortest Path
6. Rollback Last Route
7. Search Vehicle
8. Show Cities
0. Exit
```

---

## 9 Sample Inputs and Outputs

### Show Cities (Option 8)
```
Available Cities:
  0. Mumbai_CST
  1. Dadar
  2. Bandra
  3. Andheri
  4. Borivali
  5. Thane
  6. Navi_Mumbai
  7. Panvel
  8. Khopoli
  9. Pune
  10. Nashik
  11. Aurangabad
  12. Nagpur
  13. Kolhapur
  14. Solapur
```

### Add Vehicle (Option 1)
```
Vehicle Name: Tesla_Model3
Location index: 0
Vehicle added: [8] Tesla_Model3 at Mumbai_CST
```

### Add Trip (Option 3)
```
Passenger: Sharma
Pickup index: 0
Dropoff index: 9
Priority (1=High, 2=Med, 3=Low): 1
Trip queued: Sharma | Mumbai_CST -> Pune | Priority 1
```

### Dispatch Trip (Option 4)
```
--- Trip Assigned ---
Vehicle   : Toyota_Innova
Passenger : Sharma
Route     : Mumbai_CST -> Pune
Distance  : 118 km
```

### Show Vehicles (Option 2)
```
ID    Name              Location        Status  Trips
--------------------------------------------------------
4     Mercedes_E        Pune            Free    8
2     BMW_M5            Andheri         Free    6
1     Toyota_Innova     Mumbai_CST      Free    4
6     Hyundai_Creta     Borivali        Free    3
3     Honda_City        Thane           Free    2
5     Tata_Nexon        Navi_Mumbai     Free    1
7     Mahindra_XUV      Panvel          Free    0
```

### Shortest Path (Option 5)
```
From index: 0
To index:   9
Shortest: Mumbai_CST -> Pune = 118 km
```

### Rollback (Option 6)
```
Rolled back: [V1] Mumbai_CST -> Pune
Vehicle [1] restored to Mumbai_CST | Trips: 3
```

---

## 10 Screenshots

> - Program startup and menu
<img width="1067" height="180" alt="Image" src="https://github.com/user-attachments/assets/f6a38b90-2486-429a-b74a-f69860589ce7" />            
> - `Show Vehicles` output (aligned table)
<img width="1045" height="143" alt="Image" src="https://github.com/user-attachments/assets/9c43e41a-826c-4349-b9d0-7a26c13e2fc7" />        
> - `Dispatch Trip` output
<img width="1060" height="82" alt="Image" src="https://github.com/user-attachments/assets/2853e091-a453-4345-927a-3036bc5545d9" />             
> - `Shortest Path` between two cities
<img width="1063" height="281" alt="Image" src="https://github.com/user-attachments/assets/ca8b8d00-d5ee-4460-bfb6-8f4c9e4453bc" />          
> - `Rollback` restoring vehicle state
<img width="1078" height="196" alt="Image" src="https://github.com/user-attachments/assets/7dfbd84c-0326-4b31-8866-72784b3f9e5d" />         

---

## 11 Results and Observations

- **Dijkstra** correctly finds shortest paths across the Maharashtra road network. Mumbai CST → Pune resolves to 118 km via the Panvel–Khopoli route.
- **Priority-based dispatch** ensures high-priority passengers are always served first, regardless of insertion order.
- **Rollback** fully restores vehicle state (location, trip count, availability) — not just a log entry.
- **Column-aligned display** using `setw()` from `<iomanip>` renders cleanly regardless of city name length.
- **File persistence** allows the system to survive restarts; fleet and trip state reload correctly on every run.
- Binary search on sorted vehicle IDs provides efficient lookup even as the fleet grows.

---

## 12 Conclusion

FleetRoute demonstrates how classical DSA concepts translate directly into a practical real-world system. Dijkstra's algorithm handles dynamic routing, a min-heap priority queue enforces SLA-based trip ordering, a stack enables reliable undo, and a hash map delivers O(1) fleet lookups. The project highlights that efficient software is not about complex frameworks — it is about choosing the right data structure for each problem. Future enhancements could include multi-stop routing, real-time traffic weights, and a GUI front-end.

---

## Project Structure

```
FleetRoute/
├── code.cpp          # Main C++ source
├── cities.txt        # City graph (nodes + weighted edges)
├── data.txt          # Persistent fleet and trip state
├── README.md         # This file
└── screenshots/      # Terminal output screenshots
    ├── menu.png
    ├── vehicles.png
    ├── dispatch.png
    └── rollback.png
```

---

## Author

**Yash Tambade**  
B.Tech Computer Science
DSA Final Project
