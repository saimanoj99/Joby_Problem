
#ifndef EVTOL_SIMULATION_H
#define EVTOL_SIMULATION_H

#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <random>
#include <functional>
#include <string>
#include <memory>
#include <iomanip>

constexpr double SIM_DURATION = 3.0;
constexpr int NUM_VEHICLES = 20;
constexpr int NUM_CHARGERS = 3;

enum Company { ALPHA, BRAVO, CHARLIE, DELTA, ECHO };
extern const std::vector<std::string> companyNames;

struct VehicleType {
    Company company;
    double cruiseSpeed;
    double batteryCapacity;
    double timeToCharge;
    double energyPerMile;
    int passengerCount;
    double faultProbPerHour;
};

struct Stats {
    double totalFlightTime = 0;
    double totalDistance = 0;
    double totalChargeTime = 0;
    double passengerMiles = 0;
    int totalFlights = 0;
    int totalCharges = 0;
    int totalFaults = 0;
};

class Vehicle {
public:
    VehicleType type;
    double nextAvailableTime = 0.0;
    Vehicle(VehicleType t);
    double getFlightDuration();
    double getDistancePerFlight();
};

struct Event {
    double time;
    std::function<void()> action;
    bool operator>(const Event& other) const;
};

class Simulation {
public:
    Simulation();
    void run();

private:
    void loadVehicleTypes();
    void createVehicles();
    void scheduleFlight(std::shared_ptr<Vehicle> v, double startTime);
    void processFlightEnd(std::shared_ptr<Vehicle> v, double startTime, double duration);
    void tryCharging(double currentTime);
    void finishCharging(std::shared_ptr<Vehicle> v, int chargerIndex);
    void printStats();

    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> eventQueue;
    std::queue<std::shared_ptr<Vehicle>> chargingQueue;
    std::vector<std::shared_ptr<Vehicle>> activeChargers;
    std::vector<VehicleType> vehicleTypes;
    std::vector<std::shared_ptr<Vehicle>> vehicles;
    std::map<Company, Stats> stats;
};

#endif
