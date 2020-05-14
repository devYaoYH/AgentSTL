#pragma once

#include "Heuristic.h"
#include "../game/NPuzzle.h"
#include <unordered_set>
#include <algorithm>

class NPuzzleHeuristic: public Heuristic{
    protected:
        // In private, we (programmers) know that NPuzzle uses TileState and NPuzzle
        virtual double score(const TileState* ts, const NPuzzle *np) const;
    public:
        NPuzzleHeuristic();
        virtual ~NPuzzleHeuristic();
        // Public consistent interface wrapper for internal score function
        virtual double score(const State* s, const Game* g) const override;
};
