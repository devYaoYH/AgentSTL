#include "AstarSearchAgent.h"

int AstarSearchAgent::AugmentedState::count = 0;

AstarSearchAgent::AstarSearchAgent(Game* g, Heuristic* h): Agent(g), search_heuristic(h), _w(1.0){}

AstarSearchAgent::AstarSearchAgent(Game* g, Heuristic* h, double weight): Agent(g), search_heuristic(h), _w(weight){}

AstarSearchAgent::~AstarSearchAgent(){
    // !IMPORTANT Release control of pointers
    search_problem = nullptr;
    search_heuristic = nullptr;
}

void AstarSearchAgent::set_weight(double d){
    this->_w = d;
}

int AstarSearchAgent::random(std::vector<std::shared_ptr<Action>>& va){
    //TODO: actual algo (this is just to test infrastructure)
    // Make 10 random (valid) moves
    // Make a copy of the starting state and hold onto a shared pointer
    std::shared_ptr<State> cur_state = std::shared_ptr<State>(search_problem->get_state());
    std::cout << std::endl << "Current Game State:" << std::endl;
    std::cout << search_problem << std::endl;
    srand(time(NULL));
    for (int i=0;i<10000;++i){
        std::vector<std::pair<std::shared_ptr<State>,std::shared_ptr<Action>>> vsa;
        int ret_code = search_problem->get_successors(cur_state.get(), vsa);
        if (ret_code){
            std::cerr << "(AstarSearchAgent::solve) get_successors failed with: " << ret_code << std::endl;
            return ret_code;
        }
        if (vsa.size() < 1){
            // gg... no more moves
            return 1;
        }
        // Randomly pick a (successor,action) pair to play
        int idx = rand() % vsa.size();

        //DEBUG moves made by Agent
        #ifdef DEBUG
        std::cerr << "(AstarSearchAgent::solve) move: " << vsa[idx].second->_specifier << std::endl;
        #endif

        // Grab the chosen (state, action) pair
        std::pair<std::shared_ptr<State>,std::shared_ptr<Action>> sa = vsa[idx];
        // Set next action to query for successors
        cur_state = sa.first;
        // Add corresponding action to output vector
        va.push_back(sa.second);
    }
    return 0;
}

int AstarSearchAgent::greedy_search(std::vector<std::shared_ptr<Action>>& va){
    typedef std::pair<std::shared_ptr<State>, std::shared_ptr<Action>> pair_sa;
    typedef std::unordered_map<std::shared_ptr<State>, std::shared_ptr<AugmentedState>, StatePointerHash, DerefCompare> aug_map;
    typedef std::pair<double, std::shared_ptr<AugmentedState>> open_item;
    typedef std::set<open_item> aug_pq;
    typedef aug_pq::iterator pq_iter;
    typedef std::unordered_map<std::shared_ptr<State>, pq_iter, StatePointerHash, DerefCompare> pq_iter_map;
    typedef std::pair<std::shared_ptr<State>, pq_iter> pq_iter_pair;
    typedef std::unordered_set<std::shared_ptr<State>, StatePointerHash> closed_set;
    aug_map visited;
    aug_pq pq;
    pq_iter_map pq_map;
    closed_set closed;

    // Track number of states traversed
    int num_states = 0;

    // Track max number of living AugmentedStates
    int living_augStates = 0;

    // Grab the start state
    std::shared_ptr<State> init_state = std::shared_ptr<State>(search_problem->get_state());

    // Add to pq
    std::shared_ptr<AugmentedState> init_augState = std::make_shared<AugmentedState>(0.0, 0.0, init_state, nullptr, nullptr);
    std::pair<pq_iter, bool> insert_pq = pq.insert(open_item(init_augState->_priority, init_augState));
    if (insert_pq.second) pq_map.insert(pq_iter_pair(init_state, insert_pq.first));
    visited[init_state] = init_augState;

    while(!pq.empty()){
        // Grab top
        std::shared_ptr<AugmentedState> curState = pq.begin()->second;
        pq.erase(pq_map[curState->_state]);
        pq_map.erase(curState->_state);
        double cur_cost = curState->_cost;

        living_augStates = std::max(living_augStates, curState->get_count());

        if (search_problem->is_goal_state(curState->_state.get())){
            // Traceback
            va.push_back(curState->_action);
            std::shared_ptr<State> cur_prev = curState->_prev;
            while(cur_prev){
                std::shared_ptr<Action> prev_action = visited[cur_prev]->_action;
                cur_prev = visited[cur_prev]->_prev;
                if (cur_prev){
                    va.push_back(prev_action);
                }
            }
            // Reverse va
            std::reverse(va.begin(), va.end());
            std::cout << "Astar visited: " << num_states << " States" << std::endl;
            std::cout << "Astar created: " << living_augStates << " AugmentedStates" << std::endl;
            return 0;
        }
        if (visited.find(curState->_state) != visited.end()){
            // Skip this already expanded state and cur_cost is higher or same
            if (cur_cost > visited[curState->_state]->_cost) continue;
            else if (cur_cost == visited[curState->_state]->_cost &&
                    closed.find(curState->_state) != closed.end()){
                //visited[curState._state] = curState;
                continue;
            }
            else if (cur_cost < visited[curState->_state]->_cost &&
                    closed.find(curState->_state) != closed.end()){
                visited[curState->_state] = curState;
            }
        }

        // Update closed
        closed.insert(curState->_state);
        //visited[curState._state] = curState;

        // // Update stored state in map
        // visited[curState._state] = curState;

        #ifdef DEBUG
        std::cerr << "Expanding State: " << std::endl;
        std::cerr << curState._state;
        // iterate through current hashmap
        aug_map::iterator it;
        for (it=visited.begin();it!=visited.end();it++){
            std::cerr << it->first << ":" << &it->second << std::endl;
        }
        #endif

        // Track number of states traversed
        num_states++;

        // Ping every 10K states
        if (num_states%10000 == 0) std::cout << "Astar visited: " << num_states << " States" << std::endl;
        if (living_augStates%10000 == 0) std::cout << "Astar created: " << living_augStates << " AugmentedStates" << std::endl;

        // Expand state
        std::vector<pair_sa> vsa;
        int expand_code = search_problem->get_successors(curState->_state.get(), vsa);
        if (expand_code){
            std::cerr << "(AstarSearchAgent::greedy_search) get_successors failed with " << expand_code << std::endl;
            return expand_code;
        }
        if (vsa.size() < 1){
            // gg... no more moves
            continue;
        }
        for (pair_sa state_action: vsa){
            double cur_h = search_heuristic->score(state_action.first.get(), search_problem);
            double cur_c = state_action.second->_cost;
            double cur_cost_to_come = curState->_cost + cur_c;
            #ifdef DEBUG
            // iterate through current hashmap
            std::cerr << "Playing out action: " << std::endl;
            std::cerr << state_action.first;
            #endif
            // std::cerr << "HashMap Contents:" << std::endl;
            // for (it=visited.begin();it!=visited.end();it++){
            //     std::cerr << it->first << ":" << &it->second << std::endl;
            // }
            // #endif
            if (visited.find(state_action.first) != visited.end()){
                // Look to see if our lowest priority expansion is lower than current
                if (cur_cost_to_come < visited[state_action.first]->_cost){
                    // Only add if our cost to come could possibly be less
                    std::shared_ptr<AugmentedState> new_state = std::make_shared<AugmentedState>(cur_cost_to_come + cur_h*this->_w, cur_cost_to_come, state_action.first, curState->_state, state_action.second);
                    // Handle pq
                    if (pq_map.find(new_state->_state) != pq_map.end()){
                        pq.erase(pq_map[new_state->_state]);
                        pq_map.erase(new_state->_state);
                    }
                    std::pair<pq_iter, bool> insert_pq = pq.insert(open_item(new_state->_priority, new_state));
                    if (insert_pq.second) pq_map.insert(pq_iter_pair(new_state->_state, insert_pq.first));
                    // Maybe push into visited as well?
                    // Update stored state in map
                    visited[new_state->_state] = new_state;
                }
            }
            else{
                std::shared_ptr<AugmentedState> new_state = std::make_shared<AugmentedState>(cur_cost_to_come + cur_h*this->_w, cur_cost_to_come, state_action.first, curState->_state, state_action.second);
                std::pair<pq_iter, bool> insert_pq = pq.insert(open_item(new_state->_priority, new_state));
                if (insert_pq.second) pq_map.insert(pq_iter_pair(new_state->_state, insert_pq.first));
                visited[new_state->_state] = new_state;
            }
        }
    }
    std::cerr << "(AstarSearchAgent::greedy_search) No solution path found..." << std::endl;
    return -1;  // ERR no path
}

int AstarSearchAgent::solve(std::vector<std::shared_ptr<Action>>& va){
    return greedy_search(va);
}
