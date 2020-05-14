#pragma once

#include "Heuristic.h"
#include "SokobanHeuristic.h"
#include "../game/Game.h"
#include "../game/Sokoban.h"
#include <unordered_set>
#include <algorithm>
#include <limits>
#include <ctime>
#include <cstdlib>

// #define DEBUG

class SokobanPatternDatabaseHeuristic: public SokobanHeuristic{
    protected:
        typedef std::pair<int, int> pii;
        typedef std::unordered_set<pii, PairHash> set_pii;
        const int ADJ[4][2] = {{0,-1},{1,0},{0,1},{-1,0}};
        BoardState _success_state;
        double _cost;
        // In private, we (programmers) know that NPuzzle uses TileState and NPuzzle
        virtual double score(const BoardState* bs, const Sokoban* sok) const override;
        bool expand_admissible(const BoardState* bs, const pii& t_loc, const pii& f_loc) const;
        bool end_match_box(const BoardState* bs, const pii& t_loc) const;
    public:
        // Initializes wrapper with state to check distance to
        SokobanPatternDatabaseHeuristic(const BoardState* bs, double cost_to_goal);
        virtual ~SokobanPatternDatabaseHeuristic(){}
};

class SokobanPatternDatabaseHeuristicBuilder{
    protected:
        // Instance of sokoban game we are generating a database for
        Sokoban* _sok_game;
    public:
        // Initializes a builder with Sokoban* ptr
        SokobanPatternDatabaseHeuristicBuilder(Sokoban* sok);
        // Returns a pointer to created sokoban pattern database interface wrapper
        // does not own pointer, delegate task of destruction to place-of-return
        SokobanPatternDatabaseHeuristic* build(int seed=0);
        virtual ~SokobanPatternDatabaseHeuristicBuilder(){_sok_game = 0;}
};
