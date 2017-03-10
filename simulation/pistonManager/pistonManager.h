//
// Created by wahba on 28/02/2017.
//

#ifndef PROPELLERPROJECTV3_PISTONMANAGER_H
#define PROPELLERPROJECTV3_PISTONMANAGER_H


#include "../piston/pistonReaction.h"

class pistonManager {
public:
    pistonManager();

    int getID();
    void incrementID();

    void setDescription(pistonParameters in);
    pistonParameters getDescription();
private:
    int ID;
    pistonParameters description;


};


#endif //PROPELLERPROJECTV3_PISTONMANAGER_H
