#include "eVTOLSimulation.h"

std::default_random_engine rng((std::random_device())());
std::uniform_real_distribution<double> dist01(0.0, 1.0);
const std::vector<std::string> companyNames = {"Alpha", "Bravo", "Charlie", "Delta", "Echo"};

Vehicle::Vehicle(VehicleType t) : type(t) {}

double Vehicle::getFlightDuration() {
    return type.batteryCapacity / (type.cruiseSpeed * type.energyPerMile);
}

double Vehicle::getDistancePerFlight() {
    return type.cruiseSpeed * getFlightDuration();
}

bool Event::operator>(const Event& other) const {
    return time > other.time;
}

Simulation::Simulation() {
    loadVehicleTypes();
    createVehicles();
    activeChargers.resize(NUM_CHARGERS);
}

void Simulation::loadVehicleTypes() {
    vehicleTypes = {
        {ALPHA, 120, 320, 0.6, 1.6, 4, 0.25},
        {BRAVO, 100, 100, 0.2, 1.5, 5, 0.10},
        {CHARLIE, 160, 220, 0.8, 2.2, 3, 0.05},
        {DELTA, 90, 120, 0.62, 0.8, 2, 0.22},
        {ECHO, 30, 150, 0.3, 5.8, 2, 0.61}
    };
}

void Simulation::createVehicles() {
    std::uniform_int_distribution<int> dist(0, vehicleTypes.size() - 1);
    for (int i = 0; i < NUM_VEHICLES; ++i) {
        VehicleType vt = vehicleTypes[dist(rng)];
        auto v = std::make_shared<Vehicle>(vt);
        vehicles.push_back(v);
        scheduleFlight(v, 0.0);
    }
}

void Simulation::scheduleFlight(std::shared_ptr<Vehicle> v, double startTime) {
    double flightDuration = v->getFlightDuration();
    if (startTime + flightDuration > SIM_DURATION) return;

    eventQueue.push({startTime + flightDuration, [this, v, startTime, flightDuration]() {
        processFlightEnd(v, startTime, flightDuration);
    }});
}

void Simulation::processFlightEnd(std::shared_ptr<Vehicle> v, double startTime, double duration) {
    double endTime = startTime + duration;
    if (endTime > SIM_DURATION) return;

    double distance = v->getDistancePerFlight();
    Stats &s = stats[v->type.company];
    s.totalFlightTime += duration;
    s.totalDistance += distance;
    s.totalFlights++;
    s.passengerMiles += v->type.passengerCount * distance;

    if (dist01(rng) < v->type.faultProbPerHour * duration)
        s.totalFaults++;

    chargingQueue.push(v);
    tryCharging(endTime);
}

void Simulation::tryCharging(double currentTime) {
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

void Simulation::finishCharging(std::shared_ptr<Vehicle> v, int chargerIndex) {
    double now = eventQueue.top().time;
    Stats &s = stats[v->type.company];
    s.totalChargeTime += v->type.timeToCharge;
    s.totalCharges++;
    activeChargers[chargerIndex] = nullptr;
    scheduleFlight(v, now);
    tryCharging(now);
}

void Simulation::run() {
    while (!eventQueue.empty() && eventQueue.top().time <= SIM_DURATION) {
        auto e = eventQueue.top(); eventQueue.pop();
        e.action();
    }
    printStats();
}

void Simulation::printStats() {
    std::cout << std::fixed << std::setprecision(2);
    for (auto& [comp, stat] : stats) {
        std::cout << "\nStats for " << companyNames[comp] << ":\n";
        std::cout << "  Avg Flight Time: " << (stat.totalFlights ? stat.totalFlightTime / stat.totalFlights : 0) << " hr\n";
        std::cout << "  Avg Distance per Flight: " << (stat.totalFlights ? stat.totalDistance / stat.totalFlights : 0) << " miles\n";
        std::cout << "  Avg Charge Time: " << (stat.totalCharges ? stat.totalChargeTime / stat.totalCharges : 0) << " hr\n";
        std::cout << "  Total Faults: " << stat.totalFaults << "\n";
        std::cout << "  Total Passenger Miles: " << stat.passengerMiles << "\n";
    }

    // Additional Vehicle Type Count Summary
    std::map<Company, int> vehicleCount;
    for (const auto& v : vehicles) {
        vehicleCount[v->type.company]++;
    }

    std::cout << "\nVehicle Distribution:\n";
    for (const auto& [comp, count] : vehicleCount) {
        std::cout << "  " << companyNames[comp] << ": " << count << " vehicle(s)\n";
    }
}
