#include "SokobanPatternDatabaseHeuristic.h"

SokobanPatternDatabaseHeuristic::SokobanPatternDatabaseHeuristic(const BoardState* bs, double cost_to_goal):_success_state(*bs),_cost(cost_to_goal){}

bool SokobanPatternDatabaseHeuristic::expand_admissible(const BoardState* bs, const pii& t_loc, const pii& f_loc) const{
    return bs->is_valid(t_loc) && !bs->is_wall(t_loc) && !bs->is_box(t_loc) &&
        bs->is_valid(f_loc) && !bs->is_wall(f_loc);
}

// We end BFS and return path when we hit upon a stored box location
bool SokobanPatternDatabaseHeuristic::end_match_box(const BoardState* bs, const pii& t_loc) const{
    return _success_state.is_box(t_loc);
}

double SokobanPatternDatabaseHeuristic::score(const BoardState* bs, const Sokoban* sok) const{
    // Test first if we can possibly reach this memoized state
    // if (!_success_state.is_traversible(bs->get_player_loc())) return std::numeric_limits<double>::infinity();
    // Scores the distance from given BoardState to stored board state
    double score = 0.0;//_success_state.get_dist(bs->get_player_loc());
    set_pii occupied_goals;
    // Compute bfs distance from each _box location to nearest _goal
    for (const std::pair<pii, int>& box_pii: bs->get_boxes()){
        pii p = box_pii.first;
        if (_success_state.is_box(p)) continue;
        pii g_loc;
        double cur_box_score = bfs_to_goal<SokobanPatternDatabaseHeuristic>(bs, p, g_loc, occupied_goals, 
            &SokobanPatternDatabaseHeuristic::end_match_box, 
            &SokobanPatternDatabaseHeuristic::expand_admissible);
        // Only accumulate score if we can get all boxes into goals
        // otherwise, skip
        if (cur_box_score == std::numeric_limits<double>::infinity()) return cur_box_score;
        score += cur_box_score;
        // Avoid matching two boxes from bs to same box in _success_state
        // occupied_goals.insert(g_loc);
    }
    // #ifdef DEBUG
    // std::cerr << &_success_state << bs << score + _cost << std::endl;
    // #endif
    return score + _cost;
}

SokobanPatternDatabaseHeuristicBuilder::SokobanPatternDatabaseHeuristicBuilder(Sokoban* sok):_sok_game(sok){}

SokobanPatternDatabaseHeuristic* SokobanPatternDatabaseHeuristicBuilder::build(int seed){
    if (seed == 0) srand(time(NULL));
    else srand(seed);
    // Grab the goal state from instanced sokoban* game
    std::shared_ptr<State> init_state = _sok_game->get_state();
    BoardState* init_s = dynamic_cast<BoardState*>(init_state.get());
    std::shared_ptr<State> goal_state = _sok_game->get_goal_state();
    BoardState* bs = dynamic_cast<BoardState*>(goal_state.get());   //Yiheng:   TERRIBLE!
    // Run a series of random actions in reverse on goal state
    double mutation_cost = 0.0;
    // scaling number of random moves to mutate goal position?
    // This needs to be a DFS search instead
    for (int i=0;i<(rand() % 10);++i){
        std::vector<std::shared_ptr<PositionAction>> current_rev_actions;
        _sok_game->get_rev_actions(bs, current_rev_actions);
        if (current_rev_actions.size() < 1) break;
        int idx = rand() % current_rev_actions.size();
        _sok_game->play_rev_action(bs, current_rev_actions[idx].get());
        mutation_cost += current_rev_actions[idx]->_cost;
    }
    if (!bs->is_traversible(init_s->get_player_loc())){
        // std::cerr << "Impossible Board:" << std::endl << bs;
        return nullptr;
    }
    // #ifdef DEBUG
    // std::cerr << bs;
    // #endif
    // Generate wrapper object around this state for comparisons
    return new SokobanPatternDatabaseHeuristic(bs, mutation_cost);
}
