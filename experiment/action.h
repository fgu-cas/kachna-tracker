#ifndef ACTIONS
#define ACTIONS

class Action {
public:
    enum Type {
        SHOCK,
        STATE,
        COUNTER,
        LIGHT,
        SOUND
    } type;
};

#endif // ACTIONS
