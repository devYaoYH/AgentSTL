#include "SokobanHeuristic.h"

// Used to store board coordinates
typedef std::pair<int, int> pii;
// Used to store set of object locations
typedef std::unordered_set<pii, PairHash> set_pii;
// Used to memoize distances to locations via bfs step-distance
typedef std::unordered_map<pii, int, PairHash> dist_map;

SokobanHeuristic::SokobanHeuristic(){}

SokobanHeuristic::~SokobanHeuristic(){}

double SokobanHeuristic::score(const State* s, const Game* g) const{
    const BoardState* bs = dynamic_cast<const BoardState*>(s);
    const Sokoban* sok = dynamic_cast<const Sokoban*>(g);
    if (!sok || !bs){
        std::cerr << "() Error, state || game argument is not of type BoardState, Sokoban" << std::endl;
        return std::nan("");
    }
    return score(bs, sok);
}

// BFS distance to nearest goal
double SokobanHeuristic::score(const BoardState* bs, const Sokoban* sok) const{
    double score = 0.0;
    set_pii occupied_goals;
    // Compute bfs distance from each _box location to nearest _goal
    for (const std::pair<pii, int>& box_pii: bs->get_boxes()){
        pii p = box_pii.first;
        if (bs->is_goal(p)) continue;   // Discount if already on goal
        pii g_loc;
        double cur_box_score = bfs_to_goal(bs, p, g_loc, occupied_goals, &SokobanHeuristic::end_goal, &SokobanHeuristic::expand_admissible);
        // Only accumulate score if we can get all boxes into goals
        // otherwise, skip
        if (cur_box_score == std::numeric_limits<double>::infinity()) return cur_box_score;
        score += cur_box_score;
        // occupied_goals.insert(g_loc);    //Makes search inadmissible
    }
    // If player is standing on a goal, implies we've pushed some box out of the way, penalize
    pii _player_loc = bs->get_player_loc();
    if (bs->is_goal(_player_loc)){
        for (int i=0;i<4;++i){
            int ax = ADJ[i][0] + _player_loc.first;
            int ay = ADJ[i][1] + _player_loc.second;
            pii adj_loc = pii(ax, ay);
            if (bs->is_box(adj_loc) && !bs->is_goal(adj_loc)){
                // We pushed a box out of the way
                score += 1.0;
                break;
            }
        }
    }
    return score;
}

bool SokobanHeuristic::expand_admissible(const BoardState* bs, const pii& t_loc, const pii& f_loc) const{
    return bs->is_valid(t_loc) && !bs->is_wall(t_loc) && 
        bs->is_valid(f_loc) && !bs->is_wall(f_loc);
}

bool SokobanHeuristic::end_goal(const BoardState* bs, const pii& t_loc) const{
    return bs->is_goal(t_loc);
}

double SokobanHeuristic::bfs_to_goal(const BoardState* bs, const pii& s_loc) const{
    pii tmp;
    return bfs_to_goal(bs, s_loc, tmp);
}

double SokobanHeuristic::bfs_to_goal(const BoardState* bs, const pii& s_loc, pii& goal_loc) const{
    set_pii tmp;
    return bfs_to_goal(bs, s_loc, goal_loc, tmp, &SokobanHeuristic::end_goal, &SokobanHeuristic::expand_admissible);
}
