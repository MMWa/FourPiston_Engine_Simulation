//
// Created by wahba on 28/02/2017.
//

#include "pistonManager.h"


pistonManager::pistonManager() : ID(0) {}

int pistonManager::getID() {

    return ID++;
}


void pistonManager::incID() {

    ID++;
}

void pistonManager::setDescription(pistonParameters in) {
    in = description;
}

pistonParameters pistonManager::getDescription() {
    return description;
}
