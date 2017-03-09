//
// Created by wahba on 27/02/2017.
//

#include <propeller.h>
#include "pistonReaction.h"

int pistonReaction::cogID() {
    return cogid();
}

void pistonReaction::setSimulationPoints(uint_fast8_t newVal){
    simulationPoints = newVal;
}

void pistonReaction::setPower(float input) {
    powerOut = input;
}
