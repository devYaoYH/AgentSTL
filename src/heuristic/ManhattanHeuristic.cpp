#include "ManhattanHeuristic.h"

// Typedef pair<int, int> object
typedef std::pair<int, int> pii;

ManhattanHeuristic::ManhattanHeuristic(){}

ManhattanHeuristic::~ManhattanHeuristic(){}

double ManhattanHeuristic::score(const State* s, const Game* g) const {
    const TileState* ts = dynamic_cast<const TileState*>(s);
    const NPuzzle* np = dynamic_cast<const NPuzzle*>(g);
    if (!ts || !np){
        std::cerr << "() Error, state || game argument is not of type TileState, NPuzzle" << std::endl;
        return std::nan("");
    }
    return score(ts, np);
}

// Implement manhattan distance + linear conflicts Consistent Heuristic
double ManhattanHeuristic::score(const TileState* ts, const NPuzzle* np) const {
    double score = 0.0;
    // Get dimensions of NPuzzle board
    pii dims_xy = np->get_dims();
    // Run through each cell to compute manhattan distance offset
    for (int x=0;x<dims_xy.first;++x){
        for (int y=0;y<dims_xy.second;++y){
            score += (double) manhattan_distance<int>(pii(x,y), ts->get_tile_home_position(x,y));
        }
    }
    return score;
}
