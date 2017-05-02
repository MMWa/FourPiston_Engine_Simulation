// quick code snippets involved in the report

#define forever for(;;)

forever {
    if (execFlag[cogid()] == 1) {
        //allows execution only once at a point
	//Calculations Area
        execFlag[cogid()] = 0;
    }
}


forever {
    //calculate the wait time from frequency
    //120: because (60 teeth * 2) one for +ve and -ve edge
    waitTime = _clkfreq / (Frequency * 120);

    //invert the pulse train output pin
    _OUTA ^= pinMask;
    //we keep track of whether on +ve or -ve edge
    //if currently at zero then do.....
    
    //turn on all the execution flags
    if (execFlag[pistonSum_thread] == 0) {
        for (unsigned int x = 0; x < execArraySize; x++) {
            execFlag[x] = 1;
        }
    }

    //We only update hall sensor on +ve edge
    //increment hall
    //Check if we are at hall point 58
    hallPos ++;
    if (hallPos >= 58) {
        //Or in hall pin
        OUTA |= extra_hall_pin_mask;
    }
    //if at 60
    if (hallPos == 60) {
        //invert hall pin
        OUTA ^= extra_hall_pin_mask;
        //reset
        hallPos = 0;
    }
    //indicate core done executing
    execFlag[cogid()] = 0;
    //freezes core till internal counter = nextCnt+waitTime saves value to nextCnt
    nextCnt = __builtin_propeller_waitcnt(nextCnt, waitTime);
}

