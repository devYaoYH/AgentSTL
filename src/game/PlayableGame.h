#pragma once

#include "Game.h"
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <unordered_map>

// Wraps around a pointer to underlying actual Game, making this playable via CLI
// Should work for all instances of Game that implements Game interface and give names to actions
class PlayableGame: public Game{
    friend std::ostream& operator<<(std::ostream& os, PlayableGame* g);
    protected:
        // Mapping from string to Action
        typedef std::pair<std::string, std::shared_ptr<Action>> PlayAction;
        // Hashmap of Action mappings
        std::unordered_map<std::string, std::shared_ptr<Action>> action_map;
        // Actual Game
        Game* game;
        // Playing State
        std::shared_ptr<State> play_state;
    public:
        ////////////////////////////////////////
        // Differ these calls to game pointer //
        ////////////////////////////////////////
        // Get goal state (if any)
        virtual std::shared_ptr<State> get_goal_state() override;
        // Check if state passed to the game is Goal
        virtual bool is_goal_state(const State* s) override;
        // Get successors stored into vector of States given starting state
        virtual int get_successors(const State* s, std::vector<std::pair<std::shared_ptr<State>,std::shared_ptr<Action>>> &v) override;
        // Get legal moves stored into vector of Actions given starting state
        virtual int get_actions(const State* s, std::vector<std::shared_ptr<Action>> &v) override;
        // Display current game state
        virtual void display(std::ostream& os) override;
        // Play a game action
        virtual int play(Action* a) override;
        // Play an action on state
        virtual bool play_action(State* s, Action* a) override;
        //////////////////////////
        // Interactive commands //
        //////////////////////////
        // Get current game state (to play) -> Constant function (no dynamic changes)
        virtual std::shared_ptr<State> get_state() override;
        // Wrap around the get_actions call to actual game and returns strings to match for
        // also updates internal action_map
        virtual void get_actions(std::vector<std::string> &v);
        // Getter for string to action* ptr mapping stored in hashmap action_map
        virtual std::shared_ptr<Action> get_action(std::string key);
        // Play action with string descriptor
        virtual int play(std::string key);
        // Constructor | Destructors
        PlayableGame(Game* g);
        virtual ~PlayableGame();
};

std::ostream& operator<<(std::ostream& os, PlayableGame* g);