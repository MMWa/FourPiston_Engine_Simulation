//
// Created by wahba on 05/03/2017.
//
//function modes
/// \brief Simulate cylinder at Intake
/// \param ratio
/// \return 0 for now, TODO: add a function
float pistonReaction::Intake(float ratio){
    Ratio = ratio;
    tick_Progression(cycleState+1);
    return 0;
}

/// \brief Simulate cylinder at Compresion
/// \return energy lost during compressing the mix
float pistonReaction::Compression(){
    float energy_lost =-ReactionThreshold;
    tick_Progression(cycleState+1);
    return energy_lost;
}

/// \brief Simulate cylinder at Power
/// \return the energy released from the combustion
float pistonReaction::Power(){
    tick_Progression(cycleState+1);
    //TODO: what about volume?
    //just a limiting thing
    if (RPS >= 25){
        return 1;
    }
    return reactionCoefficient * Ratio;
}

/// \brief Simulate cylinder at Exhaust
/// \return Energy lost from pushing the mix out
float pistonReaction::Exhaust(){
    float energy_lost =-exhaustLoss;
    tick_Progression(0);
    //TODO: indicate some burn/unburn ratio
    return energy_lost;
}