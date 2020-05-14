#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <memory>

// Action struct for use in games
struct Action{
    int _specifier;
    double _cost;
    std::string _name;  // Can ignore if we do not support making game Playable
    Action(int spec, double cost):_specifier(spec), _cost(cost), _name("NULL"){}
    Action(int spec, double cost, std::string name):_specifier(spec), _cost(cost), _name(name){}
    virtual ~Action(){};
};

// Virtual State Class
class State{
    friend std::ostream& operator<<(std::ostream& os, State* s);
    friend std::ostream& operator<<(std::ostream& os, const State* s);
    public:
        // Display
        State() = default;
        virtual ~State(){};
        virtual void display(std::ostream& os) const = 0;
        virtual bool operator==(const State &other) const = 0;
        virtual bool operator!=(const State &other) const = 0;
        virtual size_t hash() const = 0;
};

// Virtual Game Class
class Game{
    friend std::ostream& operator<<(std::ostream& os, Game* g);
    protected:
        // Declare coord offsets for NESW
        const int ADJ[4][2] = {{0,-1},{1,0},{0,1},{-1,0}};
        const char MOVE_DIR[5] = "NESW";
        State* _state;
    public:
        // Get current game state (to play) -> Constant function (no dynamic changes)
        virtual std::shared_ptr<State> get_state() = 0;
        // Get goal state (if any)
        virtual std::shared_ptr<State> get_goal_state() = 0;
        // Check if state passed to the game is Goal
        virtual bool is_goal_state(const State* s) = 0;
        // Get successors stored into vector of States given starting state
        virtual int get_successors(const State* s, std::vector<std::pair<std::shared_ptr<State>,std::shared_ptr<Action>>> &v) = 0;
        // Get legal moves stored into vector of Actions given starting state
        virtual int get_actions(const State* s, std::vector<std::shared_ptr<Action>> &v) = 0;
        // Display current game state
        virtual void display(std::ostream& os) = 0;
        // Play a game action
        virtual int play(Action* a) = 0;
        // Play a game action on top of a given state
        virtual bool play_action(State* s, Action* a) = 0;
        // Constructor | Destructors
        Game():_state(nullptr){};
        virtual ~Game(){delete _state;};
};

std::ostream& operator<<(std::ostream& os, State* s);
std::ostream& operator<<(std::ostream& os, const State* s);
std::ostream& operator<<(std::ostream& os, Game* g);

struct StatePointerHash{
    size_t operator()(const std::shared_ptr<const State> &s) const;
};

struct DerefCompare {
    template <typename T>
    size_t operator() (std::shared_ptr<T> const &a,
                       std::shared_ptr<T> const &b) const {
        //NOTE: If you dereference a State pointer, does it know to use the TileState operator==(TileState&) overload?
        return *a == *b;
    }
};

struct StateRawPointerHash{
    size_t operator()(const State* const &s) const;
};

struct DerefRawCompare{
    template <typename T>
    size_t operator() (const T* const &a,
                       const T* const &b) const {
        //NOTE: If you dereference a State pointer, does it know to use the TileState operator==(TileState&) overload?
        return *a == *b;
    }
};

