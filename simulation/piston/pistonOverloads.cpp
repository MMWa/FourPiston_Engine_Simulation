//
// Created by wahba on 27/02/2017.
//
#include "pistonReaction.h"

/// \brief this is just a demo engine every thing is pre-set
/// \return no return
pistonReaction::pistonReaction():
        reactionCoefficient(120), ReactionThreshold(30), exhaustLoss(6),
        cycleState(0), state(0), powerOut(0), simulationPos(0), pistonID(0),
        simulationPoints(60){}

/// all Constructor
pistonReaction::pistonReaction(float reactioncoefficient, float threshold):
        reactionCoefficient(reactioncoefficient), ReactionThreshold(threshold), exhaustLoss(6),
        cycleState(0), state(0), powerOut(0),
        pistonsNub(1), simulationPos(0), pistonID(0),
        simulationPoints(60) {}

pistonReaction::pistonReaction(float reactioncoefficient, float threshold, int no_Pistons, int piston_id):
        reactionCoefficient(reactioncoefficient), ReactionThreshold(threshold), exhaustLoss(6),
        cycleState(0), state(0), powerOut(0),
        pistonsNub(no_Pistons), simulationPos(0), pistonID(piston_id),
        simulationPoints(60)
{
    simulationPos = (simulationPoints/4)*piston_id;
}

pistonReaction::pistonReaction(float reactioncoefficient, float threshold, int pistonsnub, int piston_id, int simulation_points):
        reactionCoefficient(reactioncoefficient), ReactionThreshold(threshold), exhaustLoss(6),
        cycleState(0), state(0), powerOut(0),
        pistonsNub(pistonsnub), simulationPos((simulation_points/4)*piston_id), pistonID(piston_id),
        simulationPoints(simulation_points) {}

pistonReaction::pistonReaction(pistonParameters input, int piston_id):
        reactionCoefficient(input.reactionCoefficient), ReactionThreshold(input.threshold), exhaustLoss(input.exhaustLoss),
        cycleState(piston_id), state(0), powerOut(0),
        pistonsNub(input.no_pistons), simulationPos(0), pistonID(piston_id),
        simulationPoints(input.simulationPoints) {}

/*
 * ReacC = 120
 * RacT = 30
 *
 *
 *
 * */