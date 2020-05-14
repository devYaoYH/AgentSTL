#include "Agent.h"
#include "ParallelMhaAgent.h"
#include <algorithm>
#define NDEBUG
#include <cassert>
#include <chrono>

//#define cilk_for for
//#define cilk_spawn 
//#define cilk_sync

//#define DEBUG

#define MAX(x, y) ((x)>(y)?(x):(y))

ParallelImhaSearchAgent::ParallelImhaSearchAgent(Game *g, std::vector<Heuristic *> &heuristics, double w):
HeuristicSearchAgent(g, heuristics), _w(w){}

int ParallelImhaSearchAgent::expand(open_item &s, int index){
    if(!s.second){
        // Open is temporarily empty (all states have been claimed for expansion) so
        // we have nothing to do. Just return 0
        return 0;
    }
    assert(s.second);
    _remove->push_back(s.second);
    std::vector<std::pair<std::shared_ptr<State>,std::shared_ptr<Action>>> vsa;
    
    auto& open0locs = *_openlocs[0];
    // Used to check open set membership since open0 is being mutated
    auto& open1locs = *_openlocs[index];

    int ret_code = search_problem->get_successors(s.second->_state.get(), vsa);
    if (ret_code){
        std::cerr << "(ParallelMhaAgent::expand) get_successors failed with: " << ret_code << std::endl;
        return ret_code;
    }
    for(auto it = vsa.begin(); it != vsa.end(); it++){
        // Only consider adding this state if it isn't in _closed_a
        if(_closed_a.find(it->first) == _closed_a.end()){
            // cost of the edge
            double cost = it->second->_cost;
            // potential updated g
            double g_candidate = s.second->_g + cost;
            if(open1locs.find(it->first) != open1locs.end()){
                // If we've seen this state before need to know previous g
                // value
                std::shared_ptr<AugmentedState> aug_state = open1locs[it->first]->second;
                const auto& state = it->first;
                //double curr_value = _g[state];
                double curr_value = aug_state->_g;
                // Does this expansion improve g value?
                if(g_candidate < curr_value){
                    // `Decrease` the open_item in the first open set. We don't
                    // have to do anything in other sets because the heuristic
                    // values (thus, the ordering) don't change, and they just
                    // store pointers so we only have to update _g once

                    // acquire lock then check again
                    _g_lock.lock();
                    g_candidate = s.second->_g + cost;
                    if (g_candidate < aug_state->_g) {
                        move_open++;
                        aug_state->_g = g_candidate;
                        aug_state->_action = it->second;
                        aug_state->_bp = s.second;
                        _open[0].erase(open0locs[it->first]);
                        open_item decreased(priority(aug_state), aug_state);
                        auto inserted = _open[0].insert(decreased);
                        // Update the iterator
                        open0locs[state] = inserted.first;
                    }
                    // unlock
                    _g_lock.unlock();
                }
            }
            else {
                // State isn't in open so we need to insert it
                std::shared_ptr<AugmentedState> a_to_insert(nullptr);
                auto u_it = _closed_u.find(it->first);
                if(u_it != _closed_u.end()){
                    // We've already seen this state and inadmissibly expanded it
                    double curr_value = u_it->second->_g;
                    if(g_candidate < curr_value){
                        // Acquire lock then check again
                        _g_lock.lock();
                        g_candidate = s.second->_g + cost;
                        if(g_candidate < u_it->second->_g){
                            reexpand++;
                            u_it->second->_g = g_candidate;
                            u_it->second->_bp = s.second;
                            u_it->second->_action = it->second;
                        }
                        _g_lock.unlock();
                    }
                    a_to_insert = u_it->second;
                    assert(a_to_insert->_h_vals.size() == _heuristics.size());
                }else{
                    a_to_insert = std::make_shared<AugmentedState>(
                        it->first, it->second, g_candidate, s.second
                    );
                    // Fill in heuristic placeholders, will update later
                    for(int i = 0; i < _heuristics.size(); i++){
                        a_to_insert->_h_vals.push_back(0);
                    }
                }
                assert(a_to_insert->_h_vals.size() == _heuristics.size());
                _openupdate->push_back(a_to_insert);
            }
        }
    }
    // At a minimum, _remove should contain the state we expanded
    assert(!_remove.get_value().empty());
}

int ParallelImhaSearchAgent::solve(std::vector<std::shared_ptr<Action>>& va){
    reexpand = 0;
    move_open = 0;
    impossible_states_in_open = 0;
    iters = 0;
    parallel_ms = 0;
    typedef std::pair<double, std::shared_ptr<AugmentedState>> open_item;
    typedef decltype(_open.begin()->begin()) open_set_iter_t;
    auto a_goal = std::make_shared<AugmentedState>(
        nullptr, nullptr, 
        std::numeric_limits<double>::infinity(), nullptr
    );
    auto a_start = std::make_shared<AugmentedState>(
        search_problem->get_state(), nullptr, 0.0, nullptr);
    
    _goal_g = std::numeric_limits<double>::infinity();

    // Clear the sets so that they start empty
    _open = std::vector<std::set<open_item, OpenItemCompare>>(_heuristics.size());
    _closed_a.clear();
    _closed_u.clear();

    _openlocs.clear();
    for(int i = 0; i < _heuristics.size(); i++) {
        _openlocs.push_back(std::make_shared<loc_map>());
        a_start->_h_vals.push_back(
            _heuristics[i]->score(a_start->_state.get(), search_problem)
        );
    }
    for(auto loc_it = _openlocs.begin(); loc_it != _openlocs.end(); loc_it++) {
        (*loc_it)->clear();
    }
    // Profiling statistics
    size_t exp_a = 0, exp_u = 0, rate = 500;

    // Make sure va is empty to start
    va.clear();

    open_item start_pair(0.0, a_start);
    for(int i = 0; i <  _open.size(); i++){
        auto inserted = _open[i].insert(start_pair);
        (*_openlocs[i]).insert({a_start->_state, inserted.first});
    }

    std::list<std::shared_ptr<AugmentedState>> openupdate_list;
    std::list<std::shared_ptr<AugmentedState>> remove_list;
    std::vector<open_item> expand_vector;

    _max_p_closed_a = 0.0;

    while(!term_criterion(a_goal, -1)){
        if(_open.size() < 2){
            // Error in open, should have at least two entries (one admissible
            // and one inadmissible heuristic)
            return 2;
        }
        if(_open[0].empty()){
            // No more states to expand and haven't found goal: no soln.
            return 1;
        }

        // setup _openupdate and _remove
        openupdate_list.clear();
        remove_list.clear();

        _openupdate.move_in(openupdate_list);
        _remove.move_in(remove_list);

        
        // Do the anchor expansion
        open_item s_a = *(_open[0].begin());
        //std::cerr << "h_val: " << s_a.first << std::endl;
        s_a.second->_closed_a_flag = true;
        s_a.second->_closed_u_flag.test_and_set();
        s_a.second->_closed_u_danger_flag = true;
        _closed_a.insert(s_a.second->_state);
        _max_p_closed_a = MAX(_max_p_closed_a, priority(s_a.second));
        exp_a++;
        if(exp_a % rate == 0){
            std::cerr << "Performed " << exp_a << " admissible expansions" << std::endl;
            std::cerr << "Performed " << exp_a*_heuristics.size() << " total expansions" << std::endl;
            std::cerr << "Reexpansions that change _g: " << reexpand << std::endl;
            std::cerr << "Moves in open: " << move_open << std::endl;
            std::cerr << "Impossible states inserted in open: " << impossible_states_in_open << std::endl;
            std::cerr << "States in open: " << _open[0].size() << std::endl;
        }
        //auto p_start = std::chrono::high_resolution_clock::now();
        cilk_spawn expand(s_a, 1);    // Will eventually be spawned
        // Do inadmissible expansions
        // this will eventually be cilk_for
        int open_size = _open.size();

        cilk_for(int i = 1; i < open_size; i++){
            // Find the state with least priority that satisfies p_criterion
            open_item argmin;
            auto open_pq = _open[i];
            for(auto s_it = open_pq.begin(); s_it != open_pq.end(); s_it++){
                // Once we see something that satisfies p_criterion and is not
                // in closed_u, we know we should expand this one since we
                // traverse in non-decreasing order. We do an atomic compare
                // and swap so that no one else tries to inadmissibly expand this
                // state.
                //
                // test_and_set returns true if the flag was previously set
                if(p_criterion(s_it->second)
                    && !(s_it->second->_closed_u_flag.test_and_set()))
                {
                    // Benign race: people will only ever try to set this
                    // to true, we only test it after a sync has occurred
                    s_it->second->_closed_u_danger_flag = true;
                    argmin = *s_it;
                    break;
                }
            }
            expand(argmin, i);
            exp_u++;
        }

        cilk_sync;

        // do the updates to _open
        _openupdate.move_out(openupdate_list);
        _remove.move_out(remove_list);
        assert(remove_list.size() > 0);

        // this will be a cilk_for
        // Make open0 responsible for adding to the closed_sets. No one else is 
        // reading them here, so we just have one thread writing, hence, no race
        cilk_for(int i = 0; i < open_size; i++) { 
            auto& current_open = _open[i];
            auto& current_locs = *_openlocs[i];

            for(auto update_it = openupdate_list.begin(); update_it != openupdate_list.end(); update_it++) {
                std::shared_ptr<AugmentedState>& aug_state_ptr = *update_it;
                assert(aug_state_ptr);
                assert(aug_state_ptr->_h_vals.size() == _heuristics.size());
                open_item to_insert(aug_state_ptr->_h_vals[i], aug_state_ptr);
                if(i == 0) {
                    to_insert.first = priority(aug_state_ptr);
                }
                auto locs_iter = current_locs.find(aug_state_ptr->_state);
                if(locs_iter != current_locs.end()){
                    decltype(current_open.begin()) open_iter = locs_iter->second;
                    #ifdef DEBUG
                    assert(*aug_state_ptr->_state == *open_iter->second->_state);
                    #endif
                    // We have a duplicate state in open. Resolve with lower g
                    if(aug_state_ptr->_g < open_iter->second->_g){
                        // The aug state already in open is worse
                        // Lock, then check again
                        _g_lock.lock();
                        if(aug_state_ptr->_g < open_iter->second->_g){
                            // Update the contents of the AugmentedState
                            // We don't just use copy constructor because aug_state_ptr
                            // has 0's for all g values so we don't want to copy those
                            open_iter->second->_g = aug_state_ptr->_g;
                            open_iter->second->_bp = aug_state_ptr->_bp;
                            open_iter->second->_action = aug_state_ptr->_action;
                            if(aug_state_ptr->_closed_u_flag.test_and_set()){
                                open_iter->second->_closed_u_flag.test_and_set();
                                open_iter->second->_closed_u_danger_flag = true;
                            }
                            if(i == 0){
                                // If the anchor search, we have to actually move the pointer
                                const std::shared_ptr<AugmentedState> &tmp = open_iter->second;
                                current_open.erase(open_iter);
                                to_insert.second = tmp;
                                to_insert.first = priority(tmp);
                                auto inserted = current_open.insert(to_insert).first;
                                current_locs[aug_state_ptr->_state] = inserted;
                            }
                        }
                        _g_lock.unlock();
                    }
                } else {
                    if(!aug_state_ptr->_closed_u_danger_flag){
                        // We've never encountered this state before, 
                        // compute the appropriate heuristic
                        aug_state_ptr->_h_vals[i] = _heuristics[i]->score(
                            aug_state_ptr->_state.get(), search_problem
                        );
                    }
                    // When we grabed h val initially, it hadn't been computed yet
                    to_insert.first = aug_state_ptr->_h_vals[i];
                    if(i == 0){
                        to_insert.first = priority(aug_state_ptr);
                        if(aug_state_ptr->_h_vals[0] == std::numeric_limits<double>::infinity()){
                            impossible_states_in_open++;
                        }
                    }
                    auto inserted = current_open.insert(to_insert);
                    current_locs[aug_state_ptr->_state] = inserted.first;
                }
            }
               
            for(auto yeet_iter = remove_list.begin(); yeet_iter != remove_list.end(); yeet_iter++) {
                std::shared_ptr<AugmentedState> to_remove = *yeet_iter;
                auto yeet_iter_iter = current_locs[to_remove->_state];
                current_open.erase(yeet_iter_iter);
                current_locs.erase((*yeet_iter)->_state);
                // open 0 iteration responsible for updating _closed_a and _closed_u
                // and checking for goal state
                if(i == 0){
                    if(to_remove->_closed_u_flag.test_and_set()){
                        _closed_u[to_remove->_state] = to_remove;
                    }
                    if(search_problem->is_goal_state(to_remove->_state.get())){
                        if(to_remove->_g < a_goal->_g){
                            a_goal = to_remove;
                        }
                    }
                }
            }
        }
        //auto p_end = std::chrono::high_resolution_clock::now();
        //parallel_ms += std::chrono::duration_cast<std::chrono::milliseconds>(p_end - p_start).count();
        //iters++;
    }
    //std::cerr << "Parallel parts took " << parallel_ms << " milliseconds total" << std::endl;

    // Traceback the path to the start
    if(!(a_goal->_bp) || !search_problem->is_goal_state(a_goal->_state.get())){
        std::cerr << "Max p: " << _max_p_closed_a << std::endl;
        // Never found the goal, has no bp
        return 3;
    }

    // Final profiling values
    std::cerr << "(ParallelImhaSearchAgent::solve) Solved puzzle with " << exp_a
        << " admissible expansions and " << exp_u
        << " inadmissible expansions" << std::endl;
    std::shared_ptr<const AugmentedState> curr = a_goal;
    while(curr->_bp){
        va.push_back(curr->_action);
        curr = curr->_bp;
    }
    reverse(va.begin(), va.end());
    return 0;
}

bool ParallelImhaSearchAgent::term_criterion(const std::shared_ptr<const AugmentedState> &as, double UNUSED_REMOVE_EVENTUALLY){
    return as->_g <= _max_p_closed_a;
}

bool ParallelImhaSearchAgent::p_criterion(const std::shared_ptr<const AugmentedState> &as){
    // MHA*++
    return (as->_g + as->_h_vals[0]) <= _max_p_closed_a;
}

bool ParallelImhaSearchAgent::p_criterion_always_false(const std::shared_ptr<const AugmentedState> &as) {
    // MHA*++  
    return (as->_h_vals[0]) >= std::numeric_limits<double>::max();
}


double ParallelImhaSearchAgent::priority(const std::shared_ptr<const AugmentedState> &as){
    return as->_g + _w * as->_h_vals[0];
}

