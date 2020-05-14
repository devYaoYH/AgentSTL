#pragma once

#include "Heuristic.h"
#include "SokobanHeuristic.h"
#include "../game/Game.h"
#include "../game/Sokoban.h"
#include <unordered_set>
#include <algorithm>
#include <vector>
#include <limits>
#include <ctime>
#include <cstdlib>

// #define DEBUG

class SokobanSingleBoxHeuristic: public SokobanHeuristic{
    protected:
        typedef std::pair<int, int> pii;
        typedef std::unordered_set<pii, PairHash> set_pii;
        const int ADJ[4][2] = {{0,-1},{1,0},{0,1},{-1,0}};
        std::vector<int> _box_seq;
        // In private, we (programmers) know that NPuzzle uses TileState and NPuzzle
        virtual double score(const BoardState* bs, const Sokoban* sok) const override;
        bool expand_avoid_boxes(const BoardState* bs, const pii& t_loc, const pii& f_loc) const;
        bool end_goal_unoccupied(const BoardState* bs, const pii& t_loc) const;
    public:
        // Initializes wrapper with state to check distance to
        SokobanSingleBoxHeuristic(std::vector<int>& seq);
        virtual ~SokobanSingleBoxHeuristic(){}
};

class SokobanSingleBoxHeuristicBuilder{
    protected:
        // Instance of sokoban game we are generating a SingleBox heuristic for
        Sokoban* _sok_game;
    public:
        // Initializes a builder with Sokoban* ptr
        SokobanSingleBoxHeuristicBuilder(Sokoban* sok);
        // Returns a pointer to created sokoban single box heuristic (imagining there's only a single box)
        // and that all other boxes are walls
        // does not own pointer, delegate task of destruction to place-of-return (in this case, pushes a bunch
        // of pointers into the given vector)
        void build(std::vector<Heuristic*>& hs, int extra_seq_num);
        virtual ~SokobanSingleBoxHeuristicBuilder(){_sok_game = 0;}
};
