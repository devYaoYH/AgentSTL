#pragma once

// https://cse.iitk.ac.in/users/cs365/2009/ppt/13jan_Aman.pdf

#include "Heuristic.h"
#include "../game/NPuzzle.h"
#include "NPuzzleHeuristic.h"
#include <unordered_set>
#include <algorithm>

class NilssonSequence: public NPuzzleHeuristic{
    private:
        // In private, we (programmers) know that NPuzzle uses TileState and NPuzzle
        double score(const TileState* ts, const NPuzzle *np) const override;
        pii next(int x, int y, pii dims) const;
        pii next(pii loc, pii dims) const;
    public:
        NilssonSequence();
        virtual ~NilssonSequence();
};
