#include "PlayerAgent.h"

PlayerAgent::PlayerAgent(PlayableGame* g):Agent(g), game(g){}

PlayerAgent::~PlayerAgent(){
	// PlayerAgent is now in charge of this PlayableGame
	// Each wrapper instance is only used once
	delete game;
}

// Grab player's action input
bool PlayerAgent::get_action(const State* s, std::string& key){
	// Display state
	std::cout << s;
	// Get a list of playable actions for given state
	std::vector<std::string> available_actions;
	game->get_actions(available_actions);
	// Display list of actions
	for (std::string act: available_actions){
		std::cout << act << ",";
	}
	std::cout << "[END]" << std::endl;
	// Get user input
	std::cout << "Enter next action: ";
	std::cin >> key; std::cin.ignore();
	// Test whether user intends to continue
	if (key.compare("END") == 0){
		return false;
	}
	return true;
}

// Interactive CLI process
int PlayerAgent::solve(std::vector<std::shared_ptr<Action>>& va){
	// Grab current game state
	std::shared_ptr<State> state = game->get_state();
	std::string action;
	while (!game->is_goal_state(state.get()) && get_action(state.get(), action)){
		// Execute player input action
		int play_code = game->play(action);
		if (! play_code){
			// Add action to va (vector of actions played)
			va.push_back(game->get_action(action));
			// Update game state post action
			state = game->get_state();
		}
		else{
			// Error was thrown during execution of play
			std::cerr << "(PlayerAgent) Error during play of action: " << action << " Code: " << play_code << std::endl;
			continue;
		}
	}
	if (action.compare("END") == 0){
		return 1;
	}
	if (game->is_goal_state(state.get())){
		std::cout << "CONGRATULATIONS: YOU WON!" << std::endl;
	}
	return 0;
}