#pragma once

#include "Heuristic.h"
#include "SokobanHeuristic.h"
#include "../game/Game.h"
#include "../game/Sokoban.h"
#include <unordered_set>
#include <algorithm>
#include <limits>

class SokobanGoalPathHeuristic: public SokobanHeuristic{
    protected:
		typedef std::pair<int, int> pii;
    	typedef std::unordered_set<pii, PairHash> set_pii;
    	const int ADJ[4][2] = {{0,-1},{1,0},{0,1},{-1,0}};
        // In private, we (programmers) know that NPuzzle uses TileState and NPuzzle
        virtual double score(const BoardState* bs, const Sokoban *sok) const override;
        bool expand_avoid_boxes(const BoardState* bs, const pii& t_loc, const pii& f_loc) const;
        bool end_goal_unoccupied(const BoardState* bs, const pii& t_loc) const;
    public:
        SokobanGoalPathHeuristic();
        virtual ~SokobanGoalPathHeuristic();
};
