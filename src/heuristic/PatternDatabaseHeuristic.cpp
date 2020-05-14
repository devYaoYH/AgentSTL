#include "PatternDatabaseHeuristic.h"

// Typedef pair<int, int> object
typedef std::pair<int, int> pii;

PatternDatabaseHeuristic::PatternDatabaseHeuristic(NPuzzle& np, int size, int lower, int upper){
    //srand(time(NULL));
    srand(23);
    for(int i = 0; i < size; i++){
        int scramble_times = (lower + rand()%(upper-lower)) * np.get_dims().first * np.get_dims().second;
        TileState *tmp = np.scramble_copy(scramble_times);
        // If we don't already have this state, store it in the database
        if(tmp != nullptr && _database.find(tmp) == _database.end()){
            _database[tmp] = scramble_times;
        }else if(tmp != nullptr){
            delete tmp;
        }
    }
}

PatternDatabaseHeuristic::~PatternDatabaseHeuristic(){
    for(auto it = _database.begin(); it != _database.end(); it++){
        delete it->first;
    }
}

// Implement manhattan distance + linear conflicts Consistent Heuristic
double PatternDatabaseHeuristic::score(const TileState* ts, const NPuzzle* np) const{
    double most_similar_mdist = std::numeric_limits<double>::infinity();
    double most_similar_sdist = 0.0;
    // Get dimensions of NPuzzle board
    pii dims_xy = np->get_dims();
    // map home position to actual position so we don't have 4-nested loop
    // when we compute diff b/n this and the states in the database
    std::unordered_map<pii, pii, PairHash> m;
    for(int x = 0; x < dims_xy.first; x++){
        for(int y = 0; y < dims_xy.second; y++){
            m[ts->get_tile_home_position(x, y)] = pii(x, y);
        }
    }
    for(auto it = _database.begin(); it != _database.end(); it++){
        // Find manhattan distance between the given state and the database state
        TileState *dts = it->first;
        double mdist = 0.0;
        for(int x = 0; x < dims_xy.first; x++){
            for(int y = 0; y < dims_xy.second; y++){
                pii ts_pos = m[dts->get_tile_home_position(x, y)];
                mdist += (double)manhattan_distance<int>(pii(x, y), ts_pos);
            }
        }
        if(mdist < most_similar_mdist){
            most_similar_mdist = mdist;
            most_similar_sdist = it->second;
        }
    }
    return most_similar_mdist + most_similar_sdist;
}
