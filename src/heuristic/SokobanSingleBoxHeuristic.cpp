#include "SokobanSingleBoxHeuristic.h"

SokobanSingleBoxHeuristic::SokobanSingleBoxHeuristic(std::vector<int>& seq){
    std::cerr << "Creating solution sequence: ";
    for (int i: seq){
        _box_seq.push_back(i);
        std::cerr << i << ",";
    }
    std::cerr << std::endl;
}

// Return when place to expand isn't a box
bool SokobanSingleBoxHeuristic::expand_avoid_boxes(const BoardState* bs, const pii& t_loc, const pii& f_loc) const{
    return bs->is_valid(t_loc) && !bs->is_wall(t_loc) && !bs->is_box(t_loc) &&
        bs->is_valid(f_loc) && !bs->is_wall(f_loc) && !bs->is_box(f_loc);
}

// We end BFS and return path when we hit upon a stored box location
bool SokobanSingleBoxHeuristic::end_goal_unoccupied(const BoardState* bs, const pii& t_loc) const{
    return bs->is_goal(t_loc) && !bs->is_box(t_loc);
}

// BFS distance to nearest goal
double SokobanSingleBoxHeuristic::score(const BoardState* bs, const Sokoban* sok) const{
    double score = 0.0;
    set_pii occupied_goals;
    for (int id: _box_seq){
        // Compute bfs distance from chosen box
        pii chosen_box = bs->get_box(id);
        if (bs->is_goal(chosen_box)) continue;   // Check the next box in sequence if this one fits
        pii g_loc;
        double cur_box_score = bfs_to_goal<SokobanSingleBoxHeuristic>(bs, chosen_box, g_loc, occupied_goals, 
            &SokobanSingleBoxHeuristic::end_goal_unoccupied, 
            &SokobanSingleBoxHeuristic::expand_avoid_boxes);
        // Return not possible if we cannot fit a box into goal in this ordering
        return cur_box_score;
        // if (cur_box_score == std::numeric_limits<double>::infinity()) return cur_box_score;
        // score += cur_box_score;
    }
    // Return lowest cost that we can get this box into goal (currently)
    return score;
}

SokobanSingleBoxHeuristicBuilder::SokobanSingleBoxHeuristicBuilder(Sokoban* sok):_sok_game(sok){}

void SokobanSingleBoxHeuristicBuilder::build(std::vector<Heuristic*>& hs, int extra_seq_num=0){
    // Grab starting state from _sok_game
    std::shared_ptr<State> init_state = _sok_game->get_state();
    BoardState* init_s = dynamic_cast<BoardState*>(init_state.get());
    std::vector<int> init_box_seq;
    // Push a SingleBoxHeuristic for each box
    for (const std::pair<pii, int>& box_pii: init_s->get_boxes()){
        init_box_seq.push_back(box_pii.second);
    }
    for (int i=0;i<init_box_seq.size();++i){
        int id = init_box_seq[i];
        std::vector<int> cur_seq;
        cur_seq.push_back(id);
        for (int j=0;j<init_box_seq.size();++j){
            if (i == j) continue;
            cur_seq.push_back(init_box_seq[j]);
        }
        std::random_shuffle(cur_seq.begin()+1, cur_seq.end());
        hs.push_back(new SokobanSingleBoxHeuristic(cur_seq));
    }
    for (int i=0;i<extra_seq_num;++i){
        std::random_shuffle(init_box_seq.begin(), init_box_seq.end());
        hs.push_back(new SokobanSingleBoxHeuristic(init_box_seq));
    }
}
