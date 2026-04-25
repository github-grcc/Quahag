#ifndef INPUTSTATE_H
#define INPUTSTATE_H

struct InputState {
    bool moveLeft{false};
    bool moveRight{false};
    bool jump{false};
    bool attack{false};
};

#endif // INPUTSTATE_H
