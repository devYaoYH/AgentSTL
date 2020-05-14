#pragma once

#include "Agent.h"
#include "../game/Game.h"
#include "../heuristic/Heuristic.h"
#include <vector>
#include <memory>
#include <ctime>
#include <cstdlib>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <limits>
#include <queue>
#include <algorithm>

// #define DEBUG

class AstarSearchAgent: public Agent{
    private:
        // Custom wrapper state for priority queue/map
        struct AugmentedState{
            static int count;
            double _priority, _cost;            // Actual g() cost_to_come, g+h for priority
            std::shared_ptr<State> _state;      // Current state
            std::shared_ptr<State> _prev;       // Back pointer
            std::shared_ptr<Action> _action;    // Action taken from _prev->_state
            // Should never happen?
            AugmentedState():_priority(-1.0), _cost(-1.0), _state(nullptr), _prev(nullptr), _action(nullptr){count++;}
            AugmentedState(double priority, double cost, 
                            std::shared_ptr<State> state,
                            std::shared_ptr<State> prev,
                            std::shared_ptr<Action> action)
                           :_priority(priority), _cost(cost),
                            _state(state), _prev(prev), _action(action){count++;}
            virtual ~AugmentedState(){count--;}
            // Copy constructor
            AugmentedState(const AugmentedState& as)
                           :_priority(as._priority), _cost(as._cost),
                            _state(as._state), _prev(as._prev), _action(as._action){count++;}
            // Copy assignment
            AugmentedState& operator=(AugmentedState as){
                std::swap(this->_cost, as._cost);
                std::swap(this->_priority, as._priority);
                std::swap(this->_state, as._state);
                std::swap(this->_prev, as._prev);
                std::swap(this->_action, as._action);
                return *this;
            }
            // Get num living descendants
            int get_count(){return count;}
            // Overload comparators
            bool operator<(const AugmentedState& other) const{
                return _priority < other._priority;
            }
            bool operator>(const AugmentedState& other) const{
                return _priority > other._priority;
            }
        };

        Heuristic* search_heuristic;
        double _w;      // w-weighted A*
        int random(std::vector<std::shared_ptr<Action>>& va);
        int greedy_search(std::vector<std::shared_ptr<Action>>& va);
    public:
        // Heuristics are always tied to Search_Problems
        // we can implement this by overloading functions
        // in Heuristic with different pointer types
        AstarSearchAgent(Game* g, Heuristic* h);
        AstarSearchAgent(Game* g, Heuristic* h, double weight);
        // Set the weight
        void set_weight(double d);
        // Destructor (don't destroy heuristic)
        virtual ~AstarSearchAgent();
        // Solution
        virtual int solve(std::vector<std::shared_ptr<Action>>& va);
};
