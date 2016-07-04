#ifndef PARAMS_H
#define PARAMS_H

struct Shock {
    int length;
    int delay;
    int in_delay;
    int refractory;
    int radius;
    int distance;
    int angle;
};

struct Arena {
    int x;
    int y;
    double size;
    int radius;
};


#endif // PARAMS_H
