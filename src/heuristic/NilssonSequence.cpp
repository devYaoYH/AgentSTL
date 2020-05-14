#include "NilssonSequence.h"

// Typedef pair<int, int> object
typedef std::pair<int, int> pii;

NilssonSequence::NilssonSequence(){}

NilssonSequence::~NilssonSequence(){}

// Implement manhattan distance + NilssonSequence inadmissible heuristic
double NilssonSequence::score(const TileState* ts, const NPuzzle* np) const{
    double score = 0.0;
    // Get dimensions of NPuzzle board
    pii dims_xy = np->get_dims();
    // Run through each cell to compute manhattan distance offset
    for (int x=0;x<dims_xy.first;++x){
        for (int y=0;y<dims_xy.second;++y){
            score += (double) manhattan_distance<int>(pii(x,y), ts->get_tile_home_position(x,y));
        }
    }

    double sequence_score = 0.0;
    if(ts->get_tile_real(0, 0)){
        // Add one b/c this space should be empty
        sequence_score++;
    }
    for(int x = 0; x < dims_xy.first; ++x){
        for(int y = 0; y < dims_xy.second; ++y){
            pii nextPos = next(x, y, dims_xy);
            pii nextHome = next(ts->get_tile_home_position(x, y), dims_xy);
            if(ts->get_tile_home_position(nextPos.first, nextPos.second)
                != nextHome)
            {
                sequence_score += 2.0;
            }
        }
    }

    score += 3.0 * sequence_score;

    return score;
}

pii NilssonSequence::next(int x, int y, pii dims) const{
    pii ret(x, y);
    if(y % 2 == 0){
        if(x == dims.first - 1){
            if(y < dims.second - 1){
                ret.second++;
            }else{
                ret.first = 0;
                ret.second = 0;
            }
        }else{
            ret.first++;
        }
    }else{
        if(x == 0){
            if(y < dims.second - 1){
                ret.second++;
            }else{
                ret.first = 0;
                ret.second = 0;
            }
        }else{
            ret.first--;
        }
    }
    return ret;
}

pii NilssonSequence::next(pii loc, pii dims) const{
    return next(loc.first, loc.second, dims);
}
