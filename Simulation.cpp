// eVTOL Simulation with Comments for Clarity
#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <random>
#include <functional>
#include <string>
#include <memory>
#include <iomanip>

// Simulation constants
constexpr double SIM_DURATION = 3.0; // Total simulation time in hours
constexpr int NUM_VEHICLES = 20;     // Total number of vehicles in the simulation
constexpr int NUM_CHARGERS = 3;      // Number of available chargers

// Random number generation setup
std::default_random_engine rng((std::random_device())());
std::uniform_real_distribution<double> dist01(0.0, 1.0);

// Enum to represent companies
enum Company { ALPHA, BRAVO, CHARLIE, DELTA, ECHO };
const std::vector<std::string> companyNames = {"Alpha", "Bravo", "Charlie", "Delta", "Echo"};

// Vehicle type information
struct VehicleType {
    Company company;
    double cruiseSpeed;
    double batteryCapacity;
    double timeToCharge;
    double energyPerMile;
    int passengerCount;
    double faultProbPerHour;
};

// Stats collected per company
struct Stats {
    double totalFlightTime = 0;
    double totalDistance = 0;
    double totalChargeTime = 0;
    double passengerMiles = 0;
    int totalFlights = 0;
    int totalCharges = 0;
    int totalFaults = 0;
};

// Vehicle class definition
class Vehicle {
public:
    VehicleType type;
    double nextAvailableTime = 0.0; // Not used in this version but could be useful

    Vehicle(VehicleType t) : type(t) {}

    // Calculate how long the vehicle can fly on a full charge
    double getFlightDuration() const {
        return type.batteryCapacity / (type.cruiseSpeed * type.energyPerMile);
    }

    // Calculate the distance the vehicle can fly on a full charge
    double getDistancePerFlight() const {
        return type.cruiseSpeed * getFlightDuration();
    }
};

// Event structure for simulation queue
struct Event {
    double time;                       // When the event occurs
    std::function<void()> action;     // What to do when it happens
    bool operator>(const Event& other) const { return time > other.time; }
};

// Main simulation class
class Simulation {
public:
    // Core simulation structures
    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> eventQueue;
    std::queue<std::shared_ptr<Vehicle>> chargingQueue;
    std::vector<std::shared_ptr<Vehicle>> activeChargers;
    std::vector<VehicleType> vehicleTypes;
    std::vector<std::shared_ptr<Vehicle>> vehicles;
    std::map<Company, Stats> stats;

    // Initialize simulation
    Simulation() {
        loadVehicleTypes();
        createVehicles();
        activeChargers.resize(NUM_CHARGERS);
    }

    // Load predefined vehicle types
    void loadVehicleTypes() {
        vehicleTypes = {
            {ALPHA, 120, 320, 0.6, 1.6, 4, 0.25},
            {BRAVO, 100, 100, 0.2, 1.5, 5, 0.10},
            {CHARLIE, 160, 220, 0.8, 2.2, 3, 0.05},
            {DELTA, 90, 120, 0.62, 0.8, 2, 0.22},
            {ECHO, 30, 150, 0.3, 5.8, 2, 0.61}
        };
    }

    // Randomly create 20 vehicles and schedule their first flight
    void createVehicles() {
        std::uniform_int_distribution<int> dist(0, vehicleTypes.size() - 1);
        for (int i = 0; i < NUM_VEHICLES; ++i) {
            VehicleType vt = vehicleTypes[dist(rng)];
            auto v = std::make_shared<Vehicle>(vt);
            vehicles.push_back(v);
            scheduleFlight(v, 0.0);
        }
    }

    // Schedule a flight if it can finish within the simulation time
    void scheduleFlight(std::shared_ptr<Vehicle> v, double startTime) {
        double flightDuration = v->getFlightDuration();
        if (startTime + flightDuration > SIM_DURATION) return;

        eventQueue.push({startTime + flightDuration, [this, v, startTime, flightDuration]() {
            processFlightEnd(v, startTime, flightDuration);
        }});
    }

    // Handle the end of a flight
    void processFlightEnd(std::shared_ptr<Vehicle> v, double startTime, double duration) {
        double endTime = startTime + duration;
        if (endTime > SIM_DURATION) return;

        // Update statistics
        double distance = v->getDistancePerFlight();
        Stats &s = stats[v->type.company];
        s.totalFlightTime += duration;
        s.totalDistance += distance;
        s.totalFlights++;
        s.passengerMiles += v->type.passengerCount * distance;

        // Randomly check for fault
        if (dist01(rng) < v->type.faultProbPerHour * duration)
            s.totalFaults++;

        // Add to charging queue
        chargingQueue.push(v);
        tryCharging(endTime);
    }

    // Assign vehicles from the queue to available chargers
    void tryCharging(double currentTime) {
        for (int i = 0; i < NUM_CHARGERS; ++i) {
            if (!activeChargers[i] && !chargingQueue.empty()) {
                auto v = chargingQueue.front(); chargingQueue.pop();
                double chargeEnd = currentTime + v->type.timeToCharge;
                if (chargeEnd > SIM_DURATION) continue;

                activeChargers[i] = v;
                eventQueue.push({chargeEnd, [this, v, i]() {
                    finishCharging(v, i);
                }});
            }
        }
    }

    // Finish charging and schedule next flight
    void finishCharging(std::shared_ptr<Vehicle> v, int chargerIndex) {
        double now = eventQueue.top().time;
        Stats &s = stats[v->type.company];
        s.totalChargeTime += v->type.timeToCharge;
        s.totalCharges++;
        activeChargers[chargerIndex] = nullptr;
        scheduleFlight(v, now);
        tryCharging(now);
    }

    // Run the simulation loop
    void run() {
        while (!eventQueue.empty() && eventQueue.top().time <= SIM_DURATION) {
            auto e = eventQueue.top(); eventQueue.pop();
            e.action();
        }
        printStats();
    }

    // Output the final statistics per company
    void printStats() {
        std::cout << std::fixed << std::setprecision(2);
        for (auto& [comp, stat] : stats) {
            std::cout << "\nStats for " << companyNames[comp] << ":\n";
            std::cout << "  Avg Flight Time: " << (stat.totalFlights ? stat.totalFlightTime / stat.totalFlights : 0) << " hr\n";
            std::cout << "  Avg Distance per Flight: " << (stat.totalFlights ? stat.totalDistance / stat.totalFlights : 0) << " miles\n";
            std::cout << "  Avg Charge Time: " << (stat.totalCharges ? stat.totalChargeTime / stat.totalCharges : 0) << " hr\n";
            std::cout << "  Total Faults: " << stat.totalFaults << "\n";
            std::cout << "  Total Passenger Miles: " << stat.passengerMiles << "\n";
        }
    }
};

// Entry point
int main() {
    Simulation sim;
    sim.run();
    return 0;
}
