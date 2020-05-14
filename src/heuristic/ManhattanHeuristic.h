#pragma once

#include "Heuristic.h"
#include "../game/NPuzzle.h"
#include <unordered_set>
#include <algorithm>

class ManhattanHeuristic: public Heuristic{
    private:
        // In private, we (programmers) know that NPuzzle uses TileState and NPuzzle
        double score(const TileState* ts, const NPuzzle *np) const;
    public:
        ManhattanHeuristic();
        virtual ~ManhattanHeuristic();
        // Public consistent interface wrapper for internal score function
        virtual double score(const State* s, const Game* g) const;
};
