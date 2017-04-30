//
// Created by wahba on 25/02/2017.
//
/*
 * this approach is good, allows reasonable sim speed and functions
 */

//include Propeller GCC libraries first

//#include "simpletools.h"        //somehow it links like its always there.
#include <propeller.h>
#include <sys/thread.h>
#include <simpletools.h>
//followed by globaly shared definitions and hub data
#include "Cores/defs.h"
//finally include simulation code
#include "simulation/EnergyToMove.h"
//#include "Cores/simulation.h"
#include "PWM_Read/pulse_width_reader.h"

libpropeller::PulseWidthReader pwmIn;

void gen_Square(void *arg __attribute__((unused)));

void pistonSum(void *arg __attribute__((unused)));

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

int pistonCounter[4];
uint_fast8_t totalNumberOfPistons = 0;

int main() {
    //initialization procedure------------------------------------------------------------------------------------------
    //make sure cores will not start unless specified, all execflag - 0
    //pin 3 initialization pin to signal ECU reset
    _DIRA |= 1 << 3;
    _OUTA |= 1 << 3;

    waitcnt(CNT+CLKFREQ/100);
    for (unsigned int x = 0; x < sizeof(execFlag) / sizeof(execFlag[0]); x++) {
        execFlag[x] = 0;
    }
    //piston select and instantiation
    DIRA |= 0 << 8;
    DIRA |= 0 << 10;
    DIRA |= 0 << 12;
    DIRA |= 0 << 14;

    pistonCounter[0] = input(8);
    pistonCounter[1] = input(10);
    pistonCounter[2] = input(12);
    pistonCounter[3] = input(14);

    //count the number of pistons seleted
    for (int x = 0; x < 4; x++){
        if (pistonCounter[x] == 1){
            totalNumberOfPistons++;
        }
    }
    printf("__\nSystemBooting\n");

    //this is a list of allowed number of simulation points
    Byte simPoints_List[] = {8, 16, 32, 36, 40, 44, 48, 52, 60};
    //gets the number of integers in array then -1
    Byte simPoints_Select = (sizeof(simPoints_List) / sizeof(simPoints_List[0])) - 1;
    //selects the number from list
    simPoints = simPoints_List[simPoints_Select];

    //locally describing the parameters of global struct
    engineDescription.reactionCoefficient = 50;
    engineDescription.threshold = 20;
    engineDescription.exhaustLoss = 4;
    engineDescription.no_pistons = totalNumberOfPistons;
    engineDescription.simulationPoints = simPoints;
    manager.setDescription(engineDescription);

    //pin list
    int pin[] = {21};
    //TODO: if needed communication pins need to be instantiated.
    pins = 0;                                   //clear all pins
    for (unsigned int x; x < sizeof(pin) / sizeof(pin[0]); x++) {
        pins |= 1 << pin[x];
    }

    Frequency = 1;                              //set frequency value
    //TODO: use COGSTART, same function auto calcs stack and creates the thread storage
    //TODO: Use pin Library

    /*
     * _start_cog_thread() - returns an int indicating the COG it started
     * so, we call it into an int. if core initilized the value will be the core it is running on
     * if not initilised it returns -1
     */
    int freqGen_thread = _start_cog_thread(squareGen_stack + STACK_SIZE, gen_Square, NULL, &squareGen_thread_data);
    while (execFlag[cogid()] == 0) {}           //forced wait till frequency thread initialised

    if (pistonCounter[0] == 1) {
        sim_thread0 = _start_cog_thread(sim1_stack + STACK_SIZE, simulationCore, NULL, &sim1_thread_data);
    }
    if (pistonCounter[1] == 1) {
        sim_thread1 = _start_cog_thread(sim2_stack + STACK_SIZE, simulationCore, NULL, &sim2_thread_data);
    }
    if (pistonCounter[2] == 1) {
        sim_thread2 = _start_cog_thread(sim3_stack + STACK_SIZE, simulationCore, NULL, &sim3_thread_data);
    }
    if (pistonCounter[3] == 1) {
        sim_thread3 = _start_cog_thread(sim4_stack + STACK_SIZE, simulationCore, NULL, &sim4_thread_data);
    }

    pistonSum_thread = _start_cog_thread(vectorSum_stack + STACK_SIZE, pistonSum, NULL, &vectorSum_thread_data);
    pwmIn.Start((1 << 5));
    //make sure serial tx pin is output for this core
    _DIRA |= 1 << 30;                           //needed to set the direction for the serial Tx
    int FMAX = 0;                               //Maximum frequency accumulator
    Byte maxSpeedInstanceCounter = 0;   //counter for simPoints < frequency

    //for now we assume the frequency is 60
    int PWM_percent_time;
    int frequencyConst = _clkfreq / 60;
    _OUTA ^= 1 << 3;
    //report internal stats Title --------------------------------------------------------------------------------------
    printf("Total Power, ");                    //Total Power in Engine
    printf("PWM pre-scaled, ");                 //PWM pre-scaled
    printf("PWM scaled, ");                     //PWM scaled to fuel value
    printf("Engine Load");                      //Load on Engine

    printf("Frequency, ");                      //Frequency

    printf("P1, P2, P3, P4, ");                 //Power in every piston
    printf("\n");

    int lastTime_Counter = CNT;
    int lastEncoder_State;
    int Encoder_State;
    int buttonPress_Deboucer = 0;
    int buttonState;
    int lastButton_State;

#define encoderA 25
#define encoderB 27
    printf("direction: %d \n", get_direction(13));
    //thread loop code!-------------------------------------------------------------------------------------------------
    forever {
        //load value Encoder detector
        Encoder_State = input(encoderA);
        if (Encoder_State != lastEncoder_State){
            if(input(encoderB)== Encoder_State){
                lastEncoder_State = Encoder_State;
                load_val+=10;
            } else {
                load_val-=10;
            }
        }
        lastEncoder_State = Encoder_State;

        buttonState = input(17);
        if (buttonState == 0){
            //printf("button pressed \n");
            buttonPress_Deboucer = buttonState;
        } else{
            if (buttonPress_Deboucer == 0 && lastButton_State == 0 && buttonState == 1){
                printf("**\nSystemStall BRAKES\n");
                load_val = 100000;
                buttonPress_Deboucer = 1;
            }
        }
        lastButton_State = buttonState;

        //if stalled reboot simulation
        if(Frequency == 0){
            printf("**\nSystemStall DOWN\n");
            reboot();
        }
        // if exceed maximum performance
        if(Frequency > 100){
            printf("**\nSystemStall UP\n");
            reboot();
        }


        if (execFlag[cogid()] == 1) {
            //the PWM control interface --------------------------------------------------------------------------------
            PWM_percent_time = ((pwmIn.getHighTime(0)) * 1000 / frequencyConst * 10) + 1;
            if (PWM_percent_time >= 150) {
                fuel_rat = (float) PWM_percent_time / 100;
            }

            //report internal states 4 times a second
            if (CNT-lastTime_Counter > _clkfreq/4){
                //report

                printf("%d,", PWM_percent_time);                //PWM pre-scaled
                printf("%d,",(int) fuel_rat);                   //PWM scaled to fuel value
                printf("%d,",(int) load_val);                   //Load on Engine

                printf("%d,", Frequency);                       //Frequency

                printf("%d, ", (int)simClassPointer[0]->getPower_out());
                printf("%d, ", (int)simClassPointer[1]->getPower_out());
                printf("%d, ", (int)simClassPointer[2]->getPower_out());
                printf("%d",   (int)simClassPointer[3]->getPower_out());
                printf("\n");
                lastTime_Counter = CNT;
            }
            execFlag[cogid()] = 0;
        }
    }
}


/*
 * runs on a separate core
 * this function generates the pulse train output
 * on rising edge all simulation bearing cores will calculate one simulation point
 * if simulation points scalling is active the function
 * will skip the becessary rising edges to trigger the simulation cores
 */
void gen_Square(void *arg) {
    unsigned int hallPos = 0;
    unsigned int nextCnt;
    unsigned int waitTime;
    Byte waveEdge = 0;
    Byte tcMultiplyer = (60 / simPoints);
    const int pinMask = pins;   //speed optimization, const compiler will cache value into core.
    const int extra_hall_pin = 19;
    const int extra_hall_pin_mask = 1 << extra_hall_pin;
    //pin for teeth && hall pin
    const int full_hall_teeth_pin = 23;
    const int full_hall_teeth_pin_mask = 1 << full_hall_teeth_pin;
    
    Byte hall_state = 0;

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
    //simulation points can be scaled accordingly
    forever {
        //this the make or break code ?, i think
        tcMultiplyer = (60*2 / simPoints);
        //calculate the wait time from frequency
        //120: because (60 teeth * 2) one for +ive and -ive edge
        waitTime = _clkfreq / (Frequency * 120);

        //invert the pulse train output pin
        _OUTA ^= pinMask;
        //we keep track of whether on +ve or -ve edge
        //if currently at zero then do.....
        if (waveEdge == 0) {
            //turn on all the execution flags
            if (execFlag[pistonSum_thread] == 0) {
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
                hall_state = 1;
                OUTA |= extra_hall_pin_mask;
            }

            //if at 60
            if (hallPos == 60) {
                //invert hall pin
                OUTA ^= extra_hall_pin_mask;
                hall_state = 0;
                //reset
                hallPos = 0;
            }
            
            //hall state check for full pin
            if (hall_state == 0){
                _OUTA ^= full_hall_teeth_pin_mask;
            }
            

        } else {
            //hall state check for full pin
            if (hall_state == 0){
                _OUTA ^= full_hall_teeth_pin_mask;
            }
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


/*
 * runs on a separate core
 * this core will wait for all simulation cores to finish calculations
 * there after it will sum the total power and turn it to corresponding frequency
 * after the frequency is set the total power is divided by four and redistributed
 * among the pistons. This simulates the fact that all the pistons are connected
 * and is used as a stop for an issue where numbers start to accumulate disproportionately
 */
void pistonSum(void *arg) {
    EnergyToMove calcRPS(totalNumberOfPistons);
    Byte temp_powerTotal;
    const Byte pistonCount = calcRPS.getNo_piston();
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
                //useful only in debug or report
                powerTotal = temp_powerTotal * pistonCount;

            }
        }
    }
}

/*
 * runs on a separate core
 * this function initializes a simulation onto a core.
 * then an infinite frequency locked loop is started which runs the simulation
 * the loop is triggered by execFlag[n] == 1  this condition garantees that
 * the freqGen core is at the rising edge and a simulation is in order.
 */
void simulationCore(void *arg) {
    int count1, count2;                                 //stopWatch: internal variables
    //check if its a standard 2 piston
    if (corePinIndicator == 1 && totalNumberOfPistons == 2 && (pistonCounter[0] == 1 && pistonCounter[2] == 1)){
        corePinIndicator++;
    }
    corePinIndicator += 1;
    int pinMask = 0;
    pinMask = 1 << corePinIndicator;
    DIRA |= corePinIndicator;

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
    coreReaction.Power_up(2000);

    while (execFlag[0] == 0) {}                         //wait for main thread to go green
    forever {
        if (execFlag[cogid()] == 1) {
            //count1 = CNT;                               //stopWatch: start time
            coreReaction.setRPS((float) Frequency);

            coreReaction.Engine_tick(fuel_rat);
            coreReaction.Load(load_val);

            execFlag[cogid()] = 0;
            //the stopWatch mechanism is used to dynamically change the number of simulation points
            //count2 = CNT;                               //stopWatch: end time
            //countAcc[3] = count2 - count1;                 //stopWatch: get difference

        }
    }
}

