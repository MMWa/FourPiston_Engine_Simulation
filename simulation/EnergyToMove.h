//
// Created by wahba on 28/12/2016.
//

#ifndef ENGINESIM2_ENERGYTOMOVE_H
#define ENGINESIM2_ENERGYTOMOVE_H

#define PI2DEF 6.283

typedef float torque;
typedef float rps;

class EnergyToMove {
public:
    EnergyToMove(int no_piston):
            no_piston(no_piston),
            Power(0),
            Torque(0),
            RPS(0)
    {}

    float calculateRPS(float power, float torque) {
        Power = power;
        Torque = torque;
        RPS = (Power/Torque)/(PI2DEF);
        return RPS;
    }

    const uint_fast8_t getNo_piston() const {
        return no_piston;
    }

private:
    const uint_fast8_t  no_piston;
    float
            Power,
            Torque,
            RPS;
};


#endif //ENGINESIM2_ENERGYTOMOVE_H
