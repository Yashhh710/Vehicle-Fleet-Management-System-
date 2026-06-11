#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <unordered_map>
#include <climits>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

using namespace std;

// ── City / Graph (loaded from cities.txt) ───────────────────
vector<string> cities;
int N = 0;
vector<vector<pair<int,int>>> graph;

bool loadCities(const string& file = "cities.txt") {
    ifstream ifs(file);
    if (!ifs.is_open()) return false;

    string line;
    enum Section { NONE, CITY_SEC, EDGE_SEC } sec = NONE;

    while (getline(ifs, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (line == "CITIES") { sec = CITY_SEC; continue; }
        if (line == "EDGES")  { sec = EDGE_SEC; continue; }

        if (sec == CITY_SEC) {
            cities.push_back(line);
        } else if (sec == EDGE_SEC) {
            istringstream ss(line);
            int u, v, w;
            if (ss >> u >> v >> w) {
                int maxNode = max({u, v, (int)graph.size() - 1}) + 1;
                graph.resize(maxNode);
                graph[u].push_back({v, w});
                graph[v].push_back({u, w});
            }
        }
    }

    N = (int)cities.size();
    graph.resize(N);
    return true;
}

// ── Dijkstra ────────────────────────────────────────────────
vector<int> dijkstra(int src) {
    vector<int> dist(N, INT_MAX);
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
    dist[src] = 0;
    pq.push({0, src});

    while (!pq.empty()) {
        int d = pq.top().first;
        int u = pq.top().second;
        pq.pop();
        if (d > dist[u]) continue;
        for (int i = 0; i < (int)graph[u].size(); i++) {
            int v = graph[u][i].first;
            int w = graph[u][i].second;
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

// ── Structures ───────────────────────────────────────────────
struct Vehicle {
    int    id;
    string name;
    int    location;
    bool   available;
    int    trips;
};

struct Trip {
    int    id;
    int    pickup;
    int    dropoff;
    int    priority;
    string passenger;

    bool operator>(const Trip& o) const { return priority > o.priority; }
};

// Snapshot for rollback
struct VehicleSnapshot {
    int vehicleId;
    int prevLocation;
    int prevTrips;
    bool prevAvailable;
    string routeDesc;
};

// ── Global State ─────────────────────────────────────────────
vector<Vehicle>  fleet;
priority_queue<Trip, vector<Trip>, greater<Trip>> tripQueue;
stack<VehicleSnapshot> rollbackStack;   // FIX: stores full state, not just string
unordered_map<int,int> vehicleIndex;

int vid = 1;
int tid = 1;

const string DATA_FILE = "data.txt";

// ── Save / Load ──────────────────────────────────────────────
void saveData() {
    ofstream ofs(DATA_FILE);
    if (!ofs.is_open()) { cerr << "Error: cannot write " << DATA_FILE << "\n"; return; }

    ofs << "VEHICLES\n";
    for (const auto& v : fleet)
        ofs << "V," << v.id << "," << v.name << ","
            << v.location << "," << (v.available ? 1 : 0) << "," << v.trips << "\n";

    ofs << "TRIPS\n";
    auto tmp = tripQueue;
    while (!tmp.empty()) {
        const Trip& t = tmp.top();
        ofs << "T," << t.id << "," << t.passenger << ","
            << t.pickup << "," << t.dropoff << "," << t.priority << "\n";
        tmp.pop();
    }

    ofs << "META\n";
    ofs << "META," << vid << "," << tid << "\n";
}

void loadData() {
    ifstream ifs(DATA_FILE);
    if (!ifs.is_open()) return;

    fleet.clear();
    vehicleIndex.clear();
    while (!tripQueue.empty()) tripQueue.pop();

    string line;
    enum Section { NONE, VEH, TRP, META_SEC } sec = NONE;

    while (getline(ifs, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (line == "VEHICLES") { sec = VEH;      continue; }
        if (line == "TRIPS")    { sec = TRP;      continue; }
        if (line == "META")     { sec = META_SEC; continue; }

        istringstream ss(line);
        string tok;

        if (sec == VEH) {
            getline(ss, tok, ',');
            Vehicle v;
            getline(ss, tok, ','); v.id       = stoi(tok);
            getline(ss, v.name,  ',');
            getline(ss, tok, ','); v.location  = stoi(tok);
            getline(ss, tok, ','); v.available = (stoi(tok) == 1);
            getline(ss, tok, ','); v.trips     = stoi(tok);
            vehicleIndex[v.id] = (int)fleet.size();
            fleet.push_back(v);

        } else if (sec == TRP) {
            getline(ss, tok, ',');
            Trip t;
            getline(ss, tok, ',');  t.id       = stoi(tok);
            getline(ss, t.passenger, ',');
            getline(ss, tok, ',');  t.pickup   = stoi(tok);
            getline(ss, tok, ',');  t.dropoff  = stoi(tok);
            getline(ss, tok, ',');  t.priority = stoi(tok);
            tripQueue.push(t);

        } else if (sec == META_SEC) {
            getline(ss, tok, ',');
            getline(ss, tok, ','); vid = stoi(tok);
            if (getline(ss, tok, ',')) tid = stoi(tok);
        }
    }

    for (const auto& v : fleet)
        if (v.id >= vid) vid = v.id + 1;

    auto tmpQ = tripQueue;
    while (!tmpQ.empty()) {
        if (tmpQ.top().id >= tid) tid = tmpQ.top().id + 1;
        tmpQ.pop();
    }
}

// ── Add Vehicle ──────────────────────────────────────────────
void addVehicle(const string& name, int loc) {
    if (loc < 0 || loc >= N) { cout << "Invalid location index.\n"; return; }
    Vehicle v{ vid++, name, loc, true, 0 };
    vehicleIndex[v.id] = (int)fleet.size();
    fleet.push_back(v);
    cout << "Vehicle added: [" << v.id << "] " << name << " at " << cities[loc] << "\n";
    saveData();
}

// ── Show Vehicles ────────────────────────────────────────────
void showVehicles() {
    if (fleet.empty()) { cout << "No vehicles in fleet.\n"; return; }

    vector<Vehicle> sorted = fleet;
    sort(sorted.begin(), sorted.end(),
         [](const Vehicle& a, const Vehicle& b){ return a.trips > b.trips; });

    cout << "\n"
         << left << setw(6)  << "ID"
                 << setw(18) << "Name"
                 << setw(16) << "Location"
                 << setw(8)  << "Status"
                 << "Trips\n";
    cout << string(56, '-') << "\n";
    for (const auto& v : sorted)
        cout << left << setw(6)  << v.id
                     << setw(18) << v.name
                     << setw(16) << cities[v.location]
                     << setw(8)  << (v.available ? "Free" : "Busy")
                     << v.trips << "\n";
}

// ── Add Trip ─────────────────────────────────────────────────
void addTrip(const string& passenger, int pickup, int dropoff, int priority) {
    if (pickup < 0 || pickup >= N || dropoff < 0 || dropoff >= N) {
        cout << "Invalid pickup/dropoff index.\n"; return;
    }
    Trip t{ tid++, pickup, dropoff, priority, passenger };
    tripQueue.push(t);
    cout << "Trip queued: " << passenger << " | "
         << cities[pickup] << " -> " << cities[dropoff]
         << " | Priority " << priority << "\n";
    saveData();
}

// ── Dispatch Trip ────────────────────────────────────────────
// FIX 1: Removed redundant second dijkstra call — reuse dist[] for trip distance.
// FIX 2: Push full VehicleSnapshot onto rollbackStack instead of just a string.
void dispatchTrip() {
    if (tripQueue.empty()) { cout << "No pending trips.\n"; return; }

    Trip t = tripQueue.top(); tripQueue.pop();

    // Single Dijkstra from pickup — used for BOTH nearest vehicle AND trip distance
    vector<int> dist = dijkstra(t.pickup);

    int bestId   = -1;
    int bestDist = INT_MAX;
    for (const auto& v : fleet) {
        if (v.available && dist[v.location] < bestDist) {
            bestDist = dist[v.location];
            bestId   = v.id;
        }
    }

    if (bestId == -1) {
        cout << "No free vehicle available. Trip re-queued.\n";
        tripQueue.push(t);
        return;
    }

    int idx = vehicleIndex[bestId];

    // Save snapshot BEFORE modifying state (enables true rollback)
    VehicleSnapshot snap;
    snap.vehicleId    = bestId;
    snap.prevLocation = fleet[idx].location;
    snap.prevTrips    = fleet[idx].trips;
    snap.prevAvailable = fleet[idx].available;
    snap.routeDesc    = "[V" + to_string(bestId) + "] "
                      + cities[t.pickup] + " -> " + cities[t.dropoff];
    rollbackStack.push(snap);

    // Update vehicle state
    fleet[idx].available = false;
    fleet[idx].trips++;
    fleet[idx].location  = t.dropoff;

    string route = cities[t.pickup] + " -> " + cities[t.dropoff];
    cout << "\n--- Trip Assigned ---\n";
    cout << "Vehicle   : " << fleet[idx].name  << "\n";
    cout << "Passenger : " << t.passenger       << "\n";
    cout << "Route     : " << route             << "\n";
    // FIX 1: dist[t.dropoff] gives pickup->dropoff distance (no second dijkstra needed)
    cout << "Distance  : " << dist[t.dropoff]   << " km\n";

    fleet[idx].available = true;
    saveData();
}

// ── Shortest Path ────────────────────────────────────────────
void shortestPath(int src, int dst) {
    if (src < 0 || src >= N || dst < 0 || dst >= N) {
        cout << "Invalid city index.\n"; return;
    }
    vector<int> dist = dijkstra(src);
    if (dist[dst] == INT_MAX)
        cout << "No path exists.\n";
    else
        cout << "Shortest: " << cities[src] << " -> " << cities[dst]
             << " = " << dist[dst] << " km\n";
}

// ── Rollback ─────────────────────────────────────────────────
// FIX 2: Actually restores vehicle state — not just prints the route string.
void rollback() {
    if (rollbackStack.empty()) { cout << "Nothing to rollback.\n"; return; }

    VehicleSnapshot snap = rollbackStack.top();
    rollbackStack.pop();

    auto it = vehicleIndex.find(snap.vehicleId);
    if (it == vehicleIndex.end()) {
        cout << "Rollback failed: vehicle no longer exists.\n"; return;
    }

    int idx = it->second;
    fleet[idx].location  = snap.prevLocation;
    fleet[idx].trips     = snap.prevTrips;
    fleet[idx].available = snap.prevAvailable;

    cout << "Rolled back: " << snap.routeDesc << "\n";
    cout << "Vehicle [" << snap.vehicleId << "] restored to "
         << cities[snap.prevLocation] << " | Trips: " << snap.prevTrips << "\n";
    saveData();
}

// ── Search Vehicle ────────────────────────────────────────────
void searchVehicle(int id) {
    vector<int> ids;
    for (const auto& v : fleet) ids.push_back(v.id);
    sort(ids.begin(), ids.end());

    if (!binary_search(ids.begin(), ids.end(), id)) {
        cout << "Vehicle not found.\n"; return;
    }
    int idx = vehicleIndex[id];
    cout << "Found: [" << fleet[idx].id << "] " << fleet[idx].name
         << " | " << cities[fleet[idx].location]
         << " | " << (fleet[idx].available ? "Free" : "Busy")
         << " | Trips: " << fleet[idx].trips << "\n";
}

// ── Show Cities ───────────────────────────────────────────────
void showCities() {
    cout << "\nAvailable Cities:\n";
    for (int i = 0; i < N; i++)
        cout << "  " << i << ". " << cities[i] << "\n";
}

// ── Main ──────────────────────────────────────────────────────
int main() {
    if (!loadCities()) {
        ofstream def("cities.txt");
        def << "# City names (one per line)\n";
        def << "CITIES\n";
        def << "Hub\nAirport\nMall\nHospital\nUniversity\nStation\nPark\nTechZone\n";
        def << "# Edges: fromIndex toIndex distanceKm\n";
        def << "EDGES\n";
        def << "0 1 15\n0 2 8\n0 3 6\n1 4 12\n2 6 5\n";
        def << "3 4 9\n4 5 7\n5 6 4\n5 7 11\n6 7 8\n";
        def.close();
        cout << "cities.txt not found — created default file.\n";
        loadCities();
    }

    loadData();

    int choice;
    do {
        cout << "\n===== Fleet Management =====\n";
        cout << "1. Add Vehicle\n";
        cout << "2. Show Vehicles\n";
        cout << "3. Add Trip\n";
        cout << "4. Dispatch Trip\n";
        cout << "5. Shortest Path\n";
        cout << "6. Rollback Last Route\n";
        cout << "7. Search Vehicle\n";
        cout << "8. Show Cities\n";
        cout << "0. Exit\n";
        cout << "Choice: ";
        cin >> choice;

        switch (choice) {
        case 1: {
            string name; int loc;
            showCities();
            cout << "Vehicle Name: "; cin >> name;
            cout << "Location index: "; cin >> loc;
            addVehicle(name, loc);
            break;
        }
        case 2: showVehicles(); break;
        case 3: {
            string passenger; int pickup, dropoff, priority;
            showCities();
            cout << "Passenger: ";      cin >> passenger;
            cout << "Pickup index: ";   cin >> pickup;
            cout << "Dropoff index: ";  cin >> dropoff;
            cout << "Priority (1=High, 2=Med, 3=Low): "; cin >> priority;
            addTrip(passenger, pickup, dropoff, priority);
            break;
        }
        case 4: dispatchTrip(); break;
        case 5: {
            int src, dst;
            showCities();
            cout << "From index: "; cin >> src;
            cout << "To index:   "; cin >> dst;
            shortestPath(src, dst);
            break;
        }
        case 6: rollback(); break;
        case 7: { int id; cout << "Vehicle ID: "; cin >> id; searchVehicle(id); break; }
        case 8: showCities(); break;
        case 0: cout << "Goodbye!\n"; break;
        default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    return 0;
}