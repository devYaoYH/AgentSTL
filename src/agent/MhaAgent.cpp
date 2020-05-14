#include "Agent.h"
#include "MhaAgent.h"
#include <algorithm>
//#define NDEBUG
#include <cassert>

#define MAX(x, y) ((x)>(y)?(x):(y))
//#define DEBUG

HeuristicSearchAgent::HeuristicSearchAgent(Game *g, std::vector<Heuristic *> &heuristics):
Agent(g), _heuristics(heuristics){}

SerialImhaSearchAgent::SerialImhaSearchAgent(Game *g, std::vector<Heuristic *> &heuristics, double w):
HeuristicSearchAgent(g, heuristics), _w(w){}

int SerialImhaSearchAgent::expand(open_item &s){
    if(!s.second){
        return 0;
    }
    // Remove this item from all of the open sets
    #ifdef DEBUG
    /*for(auto it = _open.begin(); it != _open.end(); it++){
        if(it->size() != _open[0].size()){
            std::cerr << "Mismatching open size, open set ind: "
                << it-_open.begin() << " is size: " << it->size()
                << " but should be " << _open[0].size() << std::endl;
        }
    }*/
    #endif
    assert(s.second);
    auto state_iter_vec = _locs[s.second->_state];
    for(auto it = _open.begin(); it != _open.end(); it++){
        it->erase(state_iter_vec[it - _open.begin()]);
    }
    // We no longer need to track the associated state in _locs
    _locs.erase(s.second->_state);
    std::vector<std::pair<std::shared_ptr<State>,std::shared_ptr<Action>>> vsa;
    int ret_code = search_problem->get_successors(s.second->_state.get(), vsa);
    if (ret_code){
        std::cerr << "(SerialImhaSearchAgent::expand) get_successors failed with: " << ret_code << std::endl;
        return ret_code;
    }
    for(auto it = vsa.begin(); it != vsa.end(); it++){
        // Only consider adding this state if it isn't in _closed_a
        if(_closed_a.find(it->first) == _closed_a.end()){
            double cost = it->second->_cost;
            double g_candidate = s.second->_g + cost;
            auto locs_it = _locs.find(it->first);
            auto u_it = _closed_u.find(it->first);
            if(_locs.find(it->first) != _locs.end()){
                // This state is already in open
                auto open_item_iter = _locs.find(it->first)->second[0];
                // Need to know previous g value
                double curr_value = open_item_iter->second->_g;
                // Does this expansion improve g value?
                if(g_candidate < curr_value){
                    // `Decrease` the open_item in the first open set. We don't
                    // have to do anything in other sets because the heuristic
                    // values (thus, the ordering) don't change, and they just
                    // store pointers so we only have to update _g once

                    // Create copy of pointer to this state since we're going
                    // to destroy the other one when we erase it.
                    std::shared_ptr<AugmentedState> to_dec(_locs.find(it->first)->second[0]->second);
                    to_dec->_g = g_candidate;
                    to_dec->_action = it->second;
                    to_dec->_bp = s.second;
                    _open[0].erase(_locs.find(it->first)->second[0]);
                    open_item decreased(priority(to_dec), to_dec);
                    auto inserted = _open[0].insert(decreased);
                    // Update the iterator
                    _locs.find(it->first)->second[0] = inserted.first;
                }
            }else {
                // The state isn't in _open so we're going to have to insert
                // something
                std::shared_ptr<AugmentedState> a_to_insert;
                if(_closed_u.find(it->first) != _closed_u.end()){
                    u_it = _closed_u.find(it->first);
                    assert(*u_it->first == *u_it->second->_state);
                    assert(*u_it->second->_state == *it->first);
                    // We have already inadmissibly expanded this state but
                    // rediscovered it. Heuristic values still memoized
                    double curr_value = u_it->second->_g;
                    // If we found a better path, update _g and the _bp
                    if(g_candidate < curr_value){
                        u_it->second->_g = g_candidate;
                        u_it->second->_bp = s.second;
                        u_it->second->_action = it->second;
                    }
                    a_to_insert = u_it->second;
                }else{
                    // We've never seen this state so just make a new one
                    a_to_insert = std::make_shared<AugmentedState>(
                        it->first, it->second, g_candidate, s.second
                    );
                    // Compute the heuristic values for this new state
                    for(int i = 0; i < _open.size(); i++){
                        a_to_insert->_h_vals.push_back(
                            _heuristics[i]->score(
                                a_to_insert->_state.get(), search_problem
                            )
                        );
                    }
                }
                assert(*a_to_insert->_state == *it->first);
                assert(_locs.find(it->first) == _locs.end());
                _locs[it->first] = std::vector<open_set_iter_t>();
                // Need to insert into every open set
                for(auto o_it = _open.begin(); o_it != _open.end(); o_it++){
                    size_t ind = o_it - _open.begin();
                    open_item to_insert(a_to_insert->_h_vals[ind], a_to_insert);
                    if(ind == 0){
                        // If anchor search, add g value to priority
                        to_insert.first = priority(a_to_insert);
                    }
                    auto inserted = o_it->insert(to_insert);
                    _locs[it->first].push_back(inserted.first);
                }
            }
        }
    }
}

int SerialImhaSearchAgent::solve(std::vector<std::shared_ptr<Action>>& va){
    reexpand = 0;
    typedef std::pair<double, std::shared_ptr<AugmentedState>> open_item;
    typedef decltype(_open.begin()->begin()) open_set_iter_t;
    auto a_goal = std::make_shared<AugmentedState>(nullptr, nullptr,
        std::numeric_limits<double>::infinity(), nullptr);
    auto a_start = std::make_shared<AugmentedState>(
        search_problem->get_state(), nullptr, 0.0, nullptr);
    _max_p_closed_a = 0.0;

    // Clear the sets so that they start empty
    _open = std::vector<std::set<open_item, OpenItemCompare>>(_heuristics.size());
    _closed_u.clear();
    _closed_a.clear();
    _locs.clear();
    // Profiling statistics
    size_t exp_a = 0, exp_u = 0, rate = 500;

    // Make sure va is empty to start
    va.clear();

    // Compute heuristics for a_start. This is to guarantee that all states
    // will have _h_vals.size() == _heuristics.size() == _open.size()
    for(int i = 0; i < _heuristics.size(); i++){
        a_start->_h_vals.push_back(_heuristics[i]->score(
            a_start->_state.get(), search_problem
        ));
    }

    open_item start_pair(0, a_start);
    for(auto it = _open.begin(); it != _open.end(); it++){
        size_t ind = it - _open.begin();
        auto inserted = it->insert(start_pair);
        _locs[a_start->_state].push_back(inserted.first);
    }

    while(!term_criterion(a_goal, _max_p_closed_a)){
        if(_open.empty()){
            // Error in open, should have at least one entry
            return 2;
        }
        if(_open[0].empty()){
            // No more states to expand and haven't found goal: no soln.
            return 1;
        }
        // Do the inadmissible expansions - skip the 0'th heuristic
        for(auto o_it = _open.begin() + 1; o_it != _open.end(); o_it++){
            // Find the state with least priority that satisfies p_criterion
            open_item argmin;
            for(auto s_it = o_it->begin(); s_it != o_it->end(); s_it++){
                // Once we see something that satisfies p_criterion and is not
                // in closed_u, we know we should expand this one since we
                // traverse in non-decreasing order
                // Aidan: but if it is in closed_u, we should permanently yeet it
                // Patrick: not necessarily, it hasn't been expanded by anchor yet
                if(p_criterion(s_it->second)
                    && _closed_u.find(s_it->second->_state) == _closed_u.end())
                {
                    argmin = *s_it;
                    break;
                }
            }
            // s_it now points to the argmin pair
            expand(argmin);
            exp_u++;
            //assert(_closed_u.find(argmin.second->_state) == _closed_u.end());
            if(argmin.second){
                _closed_u[argmin.second->_state] = argmin.second;
                // Track the best goal state we have expanded (inadmissibly)
                if(search_problem->is_goal_state(argmin.second->_state.get())){
                    if(argmin.second->_g < a_goal->_g){
                        a_goal = argmin.second;
                    }
                }
            }
        }
        // Do the anchor expansion
        open_item s_a = *(_open[0].begin());
        if(_closed_u.find(s_a.second->_state) != _closed_u.end()){
            reexpand++;
        }
        assert(_closed_a.find(s_a.second->_state) == _closed_a.end());
        expand(s_a);
        exp_a++;
        _closed_a.insert(s_a.second->_state);
        // Track the best goal state we have expanded (admissibly)
        if(search_problem->is_goal_state(s_a.second->_state.get())){
            if(s_a.second->_g < a_goal->_g){
                a_goal = s_a.second;
            }
        }
        _max_p_closed_a = MAX(_max_p_closed_a, priority(s_a.second));
        // Output profiling variables occasionally
        if(exp_a % rate == 0){
            std::cerr << "Performed " << exp_a
                << " admissible expansions" << std::endl;
            std::cerr << "Performed " << exp_u
                << " inadmissible expansions" << std::endl;
            std::cerr << "Reexpansions: " << reexpand << std::endl;
        }
    }

    // Traceback the path to the start
    if(!(a_goal->_bp) && !search_problem->is_goal_state(a_goal->_state.get())){
        // Never found the goal, has no bp
        return 3;
    }

    // Final profiling values
    std::cerr << "(SerialImhaSearchAgent::solve) Solved puzzle with " << exp_a
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

bool SerialImhaSearchAgent::term_criterion(const std::shared_ptr<const AugmentedState> &as, double comp){
    return as->_g <= comp;
}

bool SerialImhaSearchAgent::p_criterion(const std::shared_ptr<const AugmentedState> &as){
    return (as->_g + as->_h_vals[0]) <= _max_p_closed_a;
}

double SerialImhaSearchAgent::priority(const std::shared_ptr<const AugmentedState> &as){
    return as->_g + _w * as->_h_vals[0];
}

void SerialImhaSearchAgent::fill_locs_end(std::vector<open_set_iter_t> &vec){
    vec.clear();
    for(auto it = _open.begin(); it != _open.end(); it++){
        vec.push_back(it->end());
    }
}
