#Simulation Classes
These classes are the building blocks of the piston simulation, they are hardware independant therfore they can be used on different microcontrollers.

##pistonReaction
Main simulation class takes a fuel input and calculates how much energy is in the piston depending on the current stroke and load.
###pistonParameters
A struck that can be easilty populated to feed into the constructor for easy instantiation.

##EnergyToMove
Converts a value of power and load to crank shaft revolutions per second (RPS).

##pistonManager
A class that could be used as a helper to during multi-piston instantiation.
