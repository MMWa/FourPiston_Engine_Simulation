//
// Created by wahba on 25/02/2017.
//
/*
 * this approach is good, allows reasonable sim speed and functions
 */

//include Propeller GCC libraries first

//#include "simpletools.h"        //somehow it links like its always there.
#include <tinystream>
#include <propeller.h>
#include <sys/thread.h>
//followed by globaly shared definitions and hub data
#include "Cores/defs.h"
//finally include simulation code
#include "simulation/EnergyToMove.h"
#include "simulation/piston/pistonReaction.h"
#include "simulation/pistonManager/pistonManager.h"
//#include "Cores/simulation.h"
#include "PWM_Read/pulse_width_reader.h"

libpropeller::PulseWidthReader pwmIn;


static int sim_thread0;
static int sim_thread1;
static int sim_thread2;
static int sim_thread3;
static int vectorSum_thread;

void gen_Square(void *arg __attribute__((unused)));
void read_PWM(void *arg __attribute__((unused)));
void vectoredSum(void *arg __attribute__((unused)));
void simulationCore(void *arg __attribute__((unused)));

static int squareGen_stack[STACK_SIZE];
static int vectorSum_stack[STACK_SIZE];
//static int PWM_in_stack[STACK_SIZE];
static int sim1_stack[STACK_SIZE];
static int sim2_stack[STACK_SIZE];
static int sim3_stack[STACK_SIZE];
static int sim4_stack[STACK_SIZE];

static _thread_state_t squareGen_thread_data;
static _thread_state_t vectorSum_thread_data;
//static _thread_state_t PWM_in_thread_data;
static _thread_state_t sim1_thread_data;
static _thread_state_t sim2_thread_data;
static _thread_state_t sim3_thread_data;
static _thread_state_t sim4_thread_data;

pistonParameters engineDescription;
pistonManager manager;
int countAcc;
uint_fast8_t simClassPointerCounter = 0;
pistonReaction (*simClassPointer[4]) = {};
uint_fast8_t simPoints;

int main() {
    //initialization procedure------------------------------------------------------------------------------------------
    //make sure cores will not start unless specified, all execflag - 0
    for (unsigned int x = 0; x < sizeof(execFlag) / sizeof(execFlag[0]); x++) {
        execFlag[x] = 0;
    }

    //this is a list of allowed number of simulaiton points
    uint_fast8_t simPoints_List[] = { 28, 32, 36, 40, 44, 48, 52, 60};
    //gets the number of integers in array then -1
    uint_fast8_t simPoints_Select = (sizeof(simPoints_List) / sizeof(simPoints_List[0])) - 1;
    //selects the number from list
    simPoints = simPoints_List[simPoints_Select];

    //locally describing the parameters of global struct
    engineDescription.reactionCoefficient = 40;
    engineDescription.threshold = 20;
    engineDescription.exhaustLoss = 4;
    engineDescription.no_pistons = 4;
    engineDescription.simulationPoints = simPoints;
    manager.setDescription(engineDescription);

    //pin list
    int pin[] = {23, 21};
    //TODO: if needed communication pins need to be instantiated.
    pins = 0;                                   //clear all pins
    for (unsigned int x; x < sizeof(pin) / sizeof(pin[0]); x++) {
        pins |= 1 << pin[x];
    }

    Frequency = 1;                              //set frequency value
    //TODO: use COGSTART, same function auto calcs stack and creates the thread storage
    //TODO: consider Pthreads?

    int freqGen_thread = _start_cog_thread(squareGen_stack + STACK_SIZE, gen_Square, NULL, &squareGen_thread_data);
    while (execFlag[cogid()] == 0) {}           //forced wait till frequency thread initialised

    sim_thread0 = _start_cog_thread(sim1_stack + STACK_SIZE, simulationCore, NULL, &sim1_thread_data);
    sim_thread1 = _start_cog_thread(sim2_stack + STACK_SIZE, simulationCore, NULL, &sim2_thread_data);
    sim_thread2 = _start_cog_thread(sim3_stack + STACK_SIZE, simulationCore, NULL, &sim3_thread_data);
    sim_thread3 = _start_cog_thread(sim4_stack + STACK_SIZE, simulationCore, NULL, &sim4_thread_data);

    vectorSum_thread = _start_cog_thread(vectorSum_stack + STACK_SIZE, vectoredSum, NULL, &vectorSum_thread_data);
    pwmIn.Start((1 << 5));

    _DIRA |= 1 << 30;                           //needed to set the direction for the serial Tx
    int FMAX = 0;                               //Maximum frequency accumulator
    uint_fast8_t maxSpeedInstanceCounter = 0;   //counter for simPoints < frequency



    int PWM_percent_time;
    int frequencyConst = _clkfreq/600;

    //thread loop code!-------------------------------------------------------------------------------------------------
    forever {
        //maximum frequency accumulator
        if (FMAX < Frequency * 60) {
            FMAX = Frequency * 60;
        }

        if (execFlag[cogid()] == 1) {

            //report internal stats -------------------------------------------------

            printf("at f: %d, %d      Power_T: %d  , deltaT: %d      ", (Frequency), (execFlag[vectorSum_thread]),
                   (int) powerTotal, (_clkfreq / countAcc) / simPoints);
            //printf("delta T: %d", (int) (_clkfreq / countAcc) / simPoints);
            //printf(" simulation points: %d  |", simPoints_List[simPoints_Select]);



            //-----------------------------------------------------------------------
            //sets the greater than max problem, tries to adjust for it
            //this condition detects if we are at the lowest number of simulation points
            //then checks if one simulation point could happen at the desired frequency
            //if not then we select a smaller number of simulation points, if possible
            if (simPoints_Select != 0) {
                if (((int) (_clkfreq / countAcc) / simPoints) <= Frequency) {
                    maxSpeedInstanceCounter++;
                    //printf("Max Speed Hit!");
                    //check if speed problem happened 5 times and we are not at fastest
                    //this allows us to avoid things like random spikes from the 52 sec problem
                    if (maxSpeedInstanceCounter > 10) {
                        //decrement the points int variale and use it to select an object in array
                        simPoints = simPoints_List[--simPoints_Select];

                        //change the number of simulation points in all cores
                        for (unsigned  int x = 0; x < sizeof(simClassPointer) / sizeof(simClassPointer[0]); x++) {
                            simClassPointer[x]->setSimulationPoints(simPoints);
                        }
                        maxSpeedInstanceCounter = 0;
                    }
                }
            }

            //the PWM control interface

            PWM_percent_time =((pwmIn.getHighTime(0))*1000/frequencyConst)+1;
            printf("PWM_in: %d     |, ", PWM_percent_time);
            if(PWM_percent_time >= 220){
                fuel_rat = (float)PWM_percent_time/100;
            }


            //print the power in all cores for debugging
            for (uint_fast8_t i = 0; i < 4; i++) {
                printf("%d, ",  simClassPointer[i]->getCycle_state());
            }
            printf("%d ", simPoints);
            printf("\n");
            execFlag[cogid()] = 0;
        }
    }
}

void gen_Square(void *arg) {
    unsigned int hallPos = 0;
    unsigned int nextCnt;
    unsigned int waitTime;
    uint_fast8_t waveEdge = 0;
    uint_fast8_t tcMultiplyer = (60/ simPoints);
    const int pinMask = pins;   //speed optimization, const compiler will cache value into core.

    const int extra_hall_pin = 19;
    const int extra_hall_pin_mask = 1 << extra_hall_pin;

    //precompute the array size and make const, it is used extensively in real time loop.
    //precomputing allows less cycles to calculate = faster code
    unsigned const int execArraySize = sizeof(execFlag) / sizeof(execFlag[0]);

    //direction must be set from within using cog
    DIRA |= pinMask;       //sets pinMask direction
    OUTA |= 1 >> 31;

    DIRA |= extra_hall_pin_mask;
    OUTA |= extra_hall_pin_mask;

    waitTime = _clkfreq / 1000;                     //presetting
    nextCnt = _CNT + waitTime;                      //presetting
    execFlag[0] = 1;

    //Force a wait till all simulation cores are done initializing
    //asynchronicity occurs because hub memory operations happen serially every (8 to 22)* 8 cycles
    //meaning that by the time each core is loaded and populated one core will be at a completely
    //different point in simulation time that the other.
    //Therefore, the below is done to make sure all cores are synchronized
    while ((execFlag[sim_thread0] == 0) && (execFlag[sim_thread1] == 0) && (execFlag[sim_thread2] == 0) &&
           (execFlag[sim_thread3] == 0)) {}

    //turn on all cores for first time to start simulating
    //the offset caused by starting from outside the loop can be negated,
    //the simulation starts at 1hz and every thing will be forced to alignment on next iteration.
    for (unsigned int x = 0; x < execArraySize; x++) {
        execFlag[x] = 1;
    }

    //real time tick code
    //--------------------------------------------------
    //todo: add time divider
    //      simulation points can be scaled accordingly
    forever {
        //fixme: what to do?
        //fixme: probe this
        //this the make or break code ?, i think
        //tcMultiplyer = (60*2 / simPoints) -1;
        //calculate the wait time from frequency
        //120: becasue (60 teeth * 2) one for +ive and -ive edge
        waitTime = _clkfreq / (Frequency * 120);

        //invert the pulse train output pin
        _OUTA ^= pinMask;
        //we keep track of whether on +ve or -ve edge
        //if currently at zero then do.....
        if (waveEdge == 0) {
            //turn on all the execution flags
            //only if we are done calculating the last cycle.
            if (execFlag[vectorSum_thread] == 0) {
                for (unsigned int x = 0; x < execArraySize; x++) {
                    execFlag[x] = 1;
                }
            }
            //needed to skip simulation points while still having 60 gear edges
            //fixme: check the tcmult ....
            waveEdge = tcMultiplyer;

            //we only update hall sensor on +ve edge
            //increment hall
            //, we check if we are at hall point 58
            hallPos += tcMultiplyer;
            if (hallPos >= 58) {
                //Or in hall pin
                OUTA |= extra_hall_pin_mask;
            }
            //if at 60
            if (hallPos == 60) {
                //invert hall pin
                OUTA ^= extra_hall_pin_mask;
                hallPos = 0;                //reset
                //execFlag[0] = 1;
            }
        } else {
            //skip execution point
            waveEdge--;
        }
        //indicate core done executing
        execFlag[cogid()] = 0;
        //freezes core till internal counter = nextCnt+waitTime saves value to nextCnt
        nextCnt = __builtin_propeller_waitcnt(nextCnt, waitTime);
    }
    //--------------------------------------------------
}

void vectoredSum(void *arg) {
    EnergyToMove calcRPS(4);
    int_fast16_t temp_powerTotal;
    const uint_fast8_t pistonCount = calcRPS.getNo_piston();
    forever {
        if (execFlag[cogid()] == 1) {
            if ((execFlag[sim_thread0] == 0) && (execFlag[sim_thread1] == 0) && (execFlag[sim_thread2] == 0) &&
                (execFlag[sim_thread3] == 0)) {

                temp_powerTotal = 0;
                for (unsigned int x = 0; x < pistonCount; x++) {
                    temp_powerTotal += simClassPointer[x]->getPower_out();
                }
                Frequency = (int) calcRPS.calculateRPS(temp_powerTotal, load_val);
                temp_powerTotal = temp_powerTotal / pistonCount;
                for (unsigned int x = 0; x < pistonCount; x++) {
                    simClassPointer[x]->setPower(temp_powerTotal);
                }
                execFlag[cogid()] = 0;
                powerTotal = temp_powerTotal;

            }
        }
    }
}

void simulationCore(void *arg) {
    int count1, count2;                                 //stopWatch: internal variables

    //getID returns an int, then increments said int
    //if we use the same manager the result is ID 0, 1, 2,...
    //the id is further fed into cycleState which indicates the stroke type
    int ID = manager.getID();

    //instantiate a class with the simulation inside.
    //takes a struct of engine parameters
    pistonReaction coreReaction(engineDescription, ID);

    //link class to pointer by reference to allow external core access.
    simClassPointer[simClassPointerCounter++] = &coreReaction;

    //through trial an error this number causes least stalls, with different ratio/load cobinations
    coreReaction.Power_up(1000);

    while (execFlag[0] == 0) {}                         //wait for main thread to go green
    forever {
        if (execFlag[cogid()] == 1) {
            count1 = CNT;                               //stopWatch: start time
            coreReaction.setRPS((float) Frequency);

            coreReaction.Engine_tick(fuel_rat);
            coreReaction.Load(load_val);

            execFlag[cogid()] = 0;
            //the stopWatch mechanism is used to dynamically change the number of simulation points
            count2 = CNT;                               //stopWatch: end time
            countAcc = count2 - count1;                 //stopWatch: get difference

        }
    }
}

