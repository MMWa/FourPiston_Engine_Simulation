//
// Created by wahba on 29/12/2016.
//

#include "pistonReaction.h"

/// \brief
/// Main simulation function, manages the simulation exection proccess
///
/// Checks if engine is On, Checks if at Stall, Simulates cycle
/// \return Change in power within the Cyclinder
//manages the simulation exection proccess
float pistonReaction::Engine_tick(float ratio) {
    if(state) {
        switch (cycleState) {
            case 0:
                return powerOut -= Intake(ratio);
            case 1:
                return powerOut -= Compression();
            case 2:
                return powerOut += Power();
            case 3:
                return powerOut -= Exhaust();
        }
    } else {
        return powerOut;
    }
}

void pistonReaction::setCycle_state(uint_fast8_t cycle_state) {
    pistonReaction::cycleState = cycle_state;
}

/// Feeds in the first the start power needed to sustain a reaction
/// \param input
void pistonReaction::Power_up(float input) {
    if (input > ReactionThreshold){
        powerOut += input;
        state = 1;
    }
}
/// \brief subtracts the torque from the power in piston
/// \param input torque in Newtons?
void pistonReaction::Load(float input) {
    powerOut -= input/pistonsNub;
    //return input;
}
/// \brief changes the position in the cycles based on the number of simulation points done
/// \param future
inline void pistonReaction::tick_Progression(uint_fast8_t future) {
    simulationPos++;
    if (simulationPos == simulationPoints/4){
        simulationPos = 0;
        cycleState = future;
    }
}

//function modes-------------------------------------------------------------------------------
/// \brief Simulate cylinder at Intake
/// \param ratio
/// \return 0 for now, TODO: add a function
inline const float pistonReaction::Intake(float ratio){
    Ratio = ratio;
    tick_Progression(cycleState+1);
    return RPS/2;
}

/// \brief Simulate cylinder at Compresion
/// \return energy lost during compressing the mix
inline const float pistonReaction::Compression(){
    tick_Progression(cycleState+1);
    return ReactionThreshold;
}

/// \brief Simulate cylinder at Power
/// \return the energy released from the combustion
const float pistonReaction::Power(){
    tick_Progression(cycleState+1);
    if (RPS >= 65){
        //wasting energy
        return (reactionCoefficient * Ratio)/(RPS/2);
    }
    return reactionCoefficient * Ratio;
}

/// \brief Simulate cylinder at Exhaust
/// \return Energy lost from pushing the mix out
inline const float pistonReaction::Exhaust(){
    tick_Progression(0);
    //TODO: indicate some burn/unburn ratio
    return exhaustLoss*(RPS/2);
}

void pistonReaction::setSimulationPoints(uint_fast8_t newVal){
    simulationPoints = newVal;
}

void pistonReaction::setPower(float input) {
    powerOut = input;
}

//Getters
int pistonReaction::getState() const {
    return state;
}

int pistonReaction::getCycle_state() const {
    return cycleState;
}

int pistonReaction::getTrain_value() const {
    return simulationPos*cycleState;
}

void pistonReaction::setRPS(float RPS) {
    pistonReaction::RPS = RPS;
}

float pistonReaction::getRPS() const {
    return RPS;
}

int pistonReaction::getSimulation_Pos() const {
    return simulationPos;
}

float pistonReaction::getPower_out() const {
    return powerOut;
}
