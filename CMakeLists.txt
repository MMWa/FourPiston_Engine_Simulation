cmake_minimum_required(VERSION 3.3)
find_package(PropWare 2.0.0 REQUIRED)

project(PropellerprojectV4_1_non_doc)

set(CMAKE_CXX_STANDARD 11)
set(BOARD QUICKSTART)
set(MODEL cmm)
#set(CMAKE_CXX_FLAGS "-O2")
set(LINK_TINY ON)
set(SOURCE_FILES
        main.cpp
        Cores/defs.h
        Cores/simulation.h
        Cores/realTime.h
        simulation/piston/pistonReaction.h
        simulation/piston/pistonOverloads.cpp
        simulation/piston/pistonReaction.cpp
        simulation/EnergyToMove.h
        simulation/pistonManager/pistonManager.h
        simulation/pistonManager/pistonManager.cpp
        PWM_Read/pulse_width_reader.S
        PWM_Read/pulse_width_reader.h
        )

create_simple_executable(PropellerprojectV4_1_non_doc ${SOURCE_FILES})