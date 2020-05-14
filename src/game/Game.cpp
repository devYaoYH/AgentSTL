#include "Game.h"

std::ostream& operator<<(std::ostream& os, State* s){
    s->display(os);
    return os;
}

std::ostream& operator<<(std::ostream& os, const State* s){
    s->display(os);
    return os;
}

std::ostream& operator<<(std::ostream& os, Game* g){
    g->display(os);
    return os;
}