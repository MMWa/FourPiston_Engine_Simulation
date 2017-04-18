//
// Created by wahba on 29/12/2016.
//

#ifndef ENGINESIM2_PISTON_REACTION_H
#define ENGINESIM2_PISTON_REACTION_H

#include <stdint.h>

struct pistonParameters{
    float
            reactionCoefficient,
            threshold,
            exhaustLoss,
            volume;
    int
            simulationPoints,
            no_pistons;
};

class pistonReaction {
public:
    pistonReaction();
    pistonReaction(float reactioncoefficient, float threshold);
    pistonReaction(float reactioncoefficient, float threshold, int no_Pistons, int piston_id);
    pistonReaction(float reactioncoefficient, float threshold, int pistonsnub, int piston_id, int simulation_points);
    pistonReaction(pistonParameters input, int piston_id);

    float Engine_tick(float);
    void Power_up(float input);
    void Load(float input);

    int getSimulation_Pos() const;

    int getCycle_state() const;
    int getState() const;
    int getTrain_value() const;

    float getRPS() const;
    void setRPS(float RPS);

    void serialize();


private:
    //globally managed
    uint_fast8_t
            cycleState,
            state,
            simulationPos,
            pistonID,
            simulationPoints;
    const float reactionCoefficient;
    float
            volume,
            fuelVolume,
            Ratio,
            powerOut,
            ReactionThreshold,
            exhaustLoss,
            RPS;


public:
    float getPower_out() const;
    void setCycle_state(uint_fast8_t cycle_state);
    void setSimulationPoints(uint_fast8_t);
    void setPower(float input);

private:
    //partially managed
    int pistonsNub;
    const float Intake(float ratio);
    const float Compression();
    const float Power();
    const float Exhaust();
    void tick_Progression(uint_fast8_t future);
};

#endif //ENGINESIM2_PISTON_REACTION_H