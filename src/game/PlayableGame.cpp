#include "PlayableGame.h"

std::shared_ptr<State> PlayableGame::get_goal_state(){
    return game->get_goal_state();
}

bool PlayableGame::is_goal_state(const State* s){
    return game->is_goal_state(s);
}

int PlayableGame::get_successors(const State* s, std::vector<std::pair<std::shared_ptr<State>,std::shared_ptr<Action>>> &v){
    return game->get_successors(s, v);
}

int PlayableGame::get_actions(const State* s, std::vector<std::shared_ptr<Action>> &v){
    return game->get_actions(s, v);
}

void PlayableGame::display(std::ostream& os){
    game->display(os);
}

int PlayableGame::play(Action* a){
    return game->play(a);
}

bool PlayableGame::play_action(State* s, Action* a){
    return game->play_action(s, a);
}

////////////////////////////
// PlayableGame specifics //
////////////////////////////
std::shared_ptr<State> PlayableGame::get_state(){
    return play_state;
}

void PlayableGame::get_actions(std::vector<std::string> &v){
    // Query underlying game for actions
    std::vector<std::shared_ptr<Action>> va;
    game->get_actions(play_state.get(), va);
    // Update hashmap
    action_map.clear();
    for (std::shared_ptr<Action> a: va){
        action_map[a->_name] = a;
        v.push_back(a->_name);
    }
}

// Simply look into hashmap and return stored ptr
std::shared_ptr<Action> PlayableGame::get_action(std::string key){
    if (action_map.find(key) == action_map.end()) return nullptr;
    return action_map[key];
}

int PlayableGame::play(std::string key){
    // Look into hashmap
    std::shared_ptr<Action> a = get_action(key);
    if (a){
        // Modify own play_state, never touch underlying game->_state
        bool valid = game->play_action(play_state.get(), a.get());
        if (!valid){
            std::cerr << "Action: " << key << " is an Invalid action" << std::endl;
        }
        return 0;
    }
    else{
        return -1;
    }
}

PlayableGame::PlayableGame(Game* g):game(g){
    play_state = game->get_state();
}

PlayableGame::~PlayableGame(){
    // Release game
    game = 0;
}

std::ostream& operator<<(std::ostream& os, PlayableGame* g){
    if (!g->game){
        os << "Error: No game found" << std::endl;
        return os;
    }
    g->game->display(os);
    return os;
}