#include "NPuzzleHeuristic.h"

// Typedef pair<int, int> object
typedef std::pair<int, int> pii;

NPuzzleHeuristic::NPuzzleHeuristic(){}

NPuzzleHeuristic::~NPuzzleHeuristic(){}

double NPuzzleHeuristic::score(const State* s, const Game* g) const{
    const TileState* ts = dynamic_cast<const TileState*>(s);
    const NPuzzle* np = dynamic_cast<const NPuzzle*>(g);
    if (!ts || !np){
        std::cerr << "() Error, state || game argument is not of type TileState, NPuzzle" << std::endl;
        return std::nan("");
    }
    return score(ts, np);
}

// Implement manhattan distance + linear conflicts Consistent Heuristic
double NPuzzleHeuristic::score(const TileState* ts, const NPuzzle* np) const{
    double score = 0.0;
    // Get dimensions of NPuzzle board
    pii dims_xy = np->get_dims();
    // Run through each cell to compute manhattan distance offset
    for (int x=0;x<dims_xy.first;++x){
        for (int y=0;y<dims_xy.second;++y){
            score += (double) manhattan_distance<int>(pii(x,y), ts->get_tile_home_position(x,y));
        }
    }

    // return score;

    int num_conflicts = 0;

    // Run through each column to grab number of linear conflicts
    for (int x=0;x<dims_xy.first;++x){
        // Create set to store conflicting pairs in this row
        std::unordered_set<pii, pair_hash> conflicts;
        for (int y=0;y<dims_xy.second;++y){
            // Look at whether this cell could have conflicts
            pii cur_home_pos = ts->get_tile_home_position(x, y);
            if (cur_home_pos.first == x){
                // Look from current y till goal y
                int min_y = std::min(cur_home_pos.second, y);
                int max_y = std::max(cur_home_pos.second, y);
                for (int dy=min_y;dy<=max_y;++dy){
                    if (dy == y) continue;
                    bool is_real = ts->get_tile_real(x, dy);
                    if (!is_real) continue;
                    pii int_tile_home_pos = ts->get_tile_home_position(x, dy);
                    if (int_tile_home_pos.first == x){
                        conflicts.insert(pii(std::min(dy,y), std::max(dy,y)));
                    }
                }
            }
        }
        num_conflicts += conflicts.size();
    }

    // Run through each row to grab number of linear conflicts
    for (int y=0;y<dims_xy.second;++y){
        // Create set to store conflicting pairs in this row
        std::unordered_set<pii, pair_hash> conflicts;
        for (int x=0;x<dims_xy.first;++x){
            // Look at whether this cell could have conflicts
            pii cur_home_pos = ts->get_tile_home_position(x, y);
            if (cur_home_pos.second == y){
                // Look from current x till goal x
                int min_x = std::min(cur_home_pos.first, x);
                int max_x = std::max(cur_home_pos.first, x);
                for (int dx=min_x;dx<=max_x;++dx){
                    if (dx == x) continue;
                    bool is_real = ts->get_tile_real(dx, y);
                    if (!is_real) continue;
                    pii int_tile_home_pos = ts->get_tile_home_position(dx, y);
                    if (int_tile_home_pos.second == y){
                        conflicts.insert(pii(std::min(dx,x), std::max(dx,x)));
                    }
                }
            }
        }
        num_conflicts += conflicts.size();
    }

    score += num_conflicts*2;

    return score;
}
