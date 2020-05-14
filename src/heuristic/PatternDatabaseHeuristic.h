#pragma once

#include "Heuristic.h"
#include "../game/Game.h"
#include "../game/NPuzzle.h"
#include "NPuzzleHeuristic.h"
#include <ctime>
#include <cstdlib>
#include <memory>
#include <limits>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <algorithm>

class PatternDatabaseHeuristic: public NPuzzleHeuristic{
    private:
        // In private, we (programmers) know that PatternDatabaseHeuristic uses
        // TileState and NPuzzle
        double score(const TileState* ts, const NPuzzle *np) const override;
        std::unordered_map<TileState*, double, StateRawPointerHash, DerefRawCompare> _database;
    public:
        PatternDatabaseHeuristic(NPuzzle& np, int size, int lower, int upper);
        virtual ~PatternDatabaseHeuristic();
};

struct PairHash
{
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2> &pair) const
    {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};
