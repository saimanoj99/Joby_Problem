#include <iostream>
#include <cmath>

enum Company { ALPHA, BRAVO, CHARLIE, DELTA, ECHO };

struct VehicleType {
    Company company;
    double cruiseSpeed;
    double batteryCapacity;
    double timeToCharge;
    double energyPerMile;
    int passengerCount;
    double faultProbPerHour;
};

class Vehicle {
public:
    VehicleType type;
    Vehicle(VehicleType t) : type(t) {}

    double getFlightDuration() const {
        return type.batteryCapacity / (type.cruiseSpeed * type.energyPerMile);
    }

    double getDistancePerFlight() const {
        return type.cruiseSpeed * getFlightDuration();
    }
};

void testFlightDuration() {
    VehicleType bravo = {BRAVO, 100, 100, 0.2, 1.5, 5, 0.10};
    Vehicle v(bravo);
    double expected = 0.666;
    double actual = v.getFlightDuration();

    if (std::abs(actual - expected) < 0.01) {
        std::cout << "Flight Duration Test Passed\n";
    } else {
        std::cout << "Flight Duration Test Failed\n";
    }
}

void testDistancePerFlight() {
    VehicleType delta = {DELTA, 90, 120, 0.62, 0.8, 2, 0.22};
    Vehicle v(delta);
    double expected = 150.0;
    double actual = v.getDistancePerFlight();

    if (std::abs(actual - expected) < 0.01) {
        std::cout << "Distance Per Flight Test Passed\n";
    } else {
        std::cout << "Distance Per Flight Test Failed\n";
    }
}

int main() {
    testFlightDuration();
    testDistancePerFlight();
    return 0;
}
