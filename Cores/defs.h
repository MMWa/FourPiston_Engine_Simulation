//
// Created by wahba on 09/03/2017.
//

#ifndef PROPELLERPROJECTV4_1_NON_DOC_DEFS_H
#define PROPELLERPROJECTV4_1_NON_DOC_DEFS_H

//forever definition allows for easy to read and write code
//infinite loops are a must for each core
//a precompile definition is appropriate

#define forever for(;;)
#define STACK_SIZE 16
#define CORE_NUMBER 8
#define minSteadyStateFuel 2.5
#define timeToExecute execFlag[cogid()] == 1
#define MAX_RPS 30          //suggested have maximum RPS defined
//#define simPoints 60       //takes values greater than 20 - ie, 24, 28, 32, 36, 40, 60 - relevant in dev


/*
 * HUBDATA: makes sure values are stored in hub not external memory or one of the cores
 * this makes execution faster, because data will flow as follows:
 *
 *          hub <->destination_core
 *              <->destination_core
 *
 * instead of the normal expected flow:
 *          source_core<->hub <->destination_core
 *                            <->destination_core
 *
 * this is especially useful for values such as Frequency and execFlag,
 * where a fresh value needs to be read multiple times per core every simulation cycle.
 *
 * volatile: indicates to the compiler not to cache the variable, thus reading a new value every execution
 */

HUBDATA volatile int Frequency;

//TODO: move from hub to copy to each core, this never changes.
int pins;     //will make obsulute
HUBDATA volatile uint_fast8_t execFlag[CORE_NUMBER];
float volatile powerTotal;
//HUBDATA volatile float fuel_rat = 8.0644; //4.4 = perfect fuel
HUBDATA volatile float fuel_rat = minSteadyStateFuel*2; //4.4 = perfect fuel
HUBDATA volatile float load_val = 20 * 4;

#endif //PROPELLERPROJECTV4_1_NON_DOC_DEFS_H
