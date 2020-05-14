#pragma once

#include "Heuristic.h"
#include "../game/Game.h"
#include "../game/Sokoban.h"
#include <unordered_set>
#include <algorithm>
#include <limits>
#include <functional>

class SokobanHeuristic: public Heuristic{
    protected:
		typedef std::pair<int, int> pii;
    	typedef std::unordered_set<pii, PairHash> set_pii;
        typedef bool expandFunc(const BoardState*, const pii&, const pii&) const;
        typedef bool endFunc(const BoardState*, const pii&) const;
    	const int ADJ[4][2] = {{0,-1},{1,0},{0,1},{-1,0}};
        // In private, we (programmers) know that NPuzzle uses TileState and NPuzzle
        virtual double score(const BoardState* bs, const Sokoban *sok) const;
        // Test for whether an expansion is valid (player @ f_loc, moving box to t_loc)
        bool expand_admissible(const BoardState* bs, const pii& t_loc, const pii& f_loc) const;
        // Test whether we should terminate bfs once we have box at t_loc
        bool end_goal(const BoardState* bs, const pii& t_loc) const;
        double bfs_to_goal(const BoardState* bs, const pii& s_loc) const;
        double bfs_to_goal(const BoardState* bs, const pii& s_loc, pii& goal_loc) const;
        // Only one that contains bfs logic
        template<class T>
        double bfs_to_goal(const BoardState* bs, const pii& s_loc, pii& goal_loc, set_pii& occ_cells, endFunc T::* end, expandFunc T::* expand) const{
            typedef std::pair<pii, int> pii_dist;
            pii loc = pii(s_loc.first, s_loc.second);
            set_pii v;
            // Grab locations from occ_cells and pre-insert to block out certain goals
            for (const pii& p: occ_cells) v.insert(p);
            // Setup queue for BFS
            std::queue<pii_dist> q;
            q.push(pii_dist(loc, 0));
            v.insert(loc);
            while (!q.empty()){
                pii_dist q_front = q.front(); q.pop();
                pii cur = q_front.first;
                int dist = q_front.second;
                // Stop expanding if we are on a goal
                if ((static_cast<const T*>(this)->*end)(bs, cur)){
                    goal_loc = cur;
                    return (double)dist;
                }
                // Expand from current location
                for (int i=0;i<4;++i){
                    int ax = ADJ[i][0] + cur.first;
                    int ay = ADJ[i][1] + cur.second;
                    pii next_loc = pii(ax, ay);
                    int next_dist = dist + 1;
                    // Furthermore, the location we would be pushed from should be empty
                    pii from_loc = pii(cur.first - ADJ[i][0], cur.second - ADJ[i][1]);
                    // Expand if not in v, is_valid and is not wall
                    if (v.find(next_loc) == v.end() && (static_cast<const T*>(this)->*expand)(bs, next_loc, from_loc)){
                        q.push(pii_dist(next_loc, next_dist));
                        v.insert(next_loc);
                    }
                }
            }
            return std::numeric_limits<double>::infinity();
        }
    public:
        SokobanHeuristic();
        virtual ~SokobanHeuristic();
        // Public consistent interface wrapper for internal score function
        virtual double score(const State* s, const Game* g) const override;
};
