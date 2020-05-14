#include "Agent.h"

size_t StatePointerHash::operator()(const std::shared_ptr<const State> &s) const{
    return s->hash();
}

size_t StateRawPointerHash::operator()(const State* const &s) const{
    return s->hash();
}
