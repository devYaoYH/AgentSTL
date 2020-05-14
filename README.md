# Agents STL-ish

The goal of this open repository is to implement commonly-used search agent algorithms in a problem-agnostic way such we can run these just like STL algorithms on a specified 'Game' class for different problems.

Currently, the list of implemented algorithms:
- weighted-A*
- [WIP] Minimax
- [WIP] Expectimax
- [WIP] Monte-Carlo-Tree-Search

Furthermore, presently, the following problems are available:
- NPuzzle: Sliding-Tile puzzle
- Sokoban: Box-pushing puzzle (credits to original game developer - Thinking Rabbit)

Lastly, I have also included a wrapper class `PlayableGame` such that any `Game` that specifies string-identified actions can be played via the console. See `sokoban_play.cpp` and `npuzzle_play.cpp` for samples.

*A python visualizer/GUI for `PlayableGame` is WIP*

## Infrastructure overview

In this project, I will use the following vernacular:
- Game: A description of rules surrounding state transitions
- State: Object completely describing some instance (in time) of a Game
- Agent: Object defining algorithms to 'play' a given Game from a given State. This 'play' we are currently interested in is searching for a pre-defined end-goal. (But perhaps some Agents are not interested in reaching an end-goal but merely optimizing some current state - leaves flexibility for future implementations)
- Search problems are derived children from Games. They are a type of Game we can play with our agents.

The `Game.h` interface exposes 3 abstract objects, `Action`, `State` and `Game`. In our implementation of various search strategies, we should only interact with these abstract class interfaces.

```cpp
// Action struct for use in games
struct Action{
    int _specifier;
    double _cost;
    Action(int spec, double cost):_specifier(spec), _cost(cost){}
};
```

Action is a struct which stores an int code that matches a corresponding enum declared in concrete Games along with an associated cost to making such an action. This struct is the minimum necessary information our Games will want. Although some concrete Games may want to implement their own derivation of Action which can be more complicated.

```cpp
// Virtual State Class
class State{
    friend std::ostream& operator<<(std::ostream& os, State* s);
    public:
        // Display
        State() = default;
        virtual ~State(){};
        virtual void display(std::ostream& os) = 0;
};
```

We don't know what's going to go on inside a Game's state, just that we need to describe a way to display it somehow, so `void display(std::ostream& os)` is the only necessary function to implement in derived States.

```cpp
// Virtual Game Class
class Game{
    friend std::ostream& operator<<(std::ostream& os, Game* g);
    protected:
        // Declare coord offsets for NESW
        int ADJ[4][2] = {{0,-1},{1,0},{0,1},{-1,0}};
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
```

Each Game object will at least contain it's current State (`_state`: pointer is owned internally by a Game object and exposed nowhere else). ADJ is for convenience (cardinal direction offsets).

Functions we need to implement for child classes derived from Game are described. The following list are functions pertinent to the implementation of various Agents:
- `std::shared_ptr<State> get_state()`
- `bool is_goal_state(State* s)`
    - Returns whether a given state is the goal
- `int get_successors(State* s, std::vector<std::pair<std::shared_ptr<State>,std::shared_ptr<Action>>> &v)`
- `int get_actions(State* s, std::vector<std::shared_ptr<Action>> &v)`
- `int play(Action* a)`
    - The previous 3 functions all Returns an int representing an error code (by convention, 0 means success).
    - An ERR_CODE enum should be defined in derived classes.

The consideration for this design is making each Game as isolated as possible. We don't know what Actions are possible (i.e. cannot create our own Actions), or what States are possible (No information of internal Game State representations). And an Agent should be abstract enough to function without knowledge of these things. Rather, we query a Game to generate available Actions and States given some starting State and run Heuristics on top of those States. Heuristics are not ignorant of internal Game States/Actions as each Heuristic at least needs to be described separately for each type of Game (but Agents should be general).

> Issue: What if a Game does not describe end-states? No goal possible, just optimization. (Or at least we know the rules but not what we're searching for)

## Heuristic Design

As mentioned earlier, Heuristics are not Game-agnostic, they *should* know what exact concrete Game type they are assessing. The abstract Heuristic class declares the only necessary function to be implemented in derived classes, `score`:

```cpp
class Heuristic{
    public:
        virtual double score(State* s, Game* g) = 0;
        virtual ~Heuristic(){};
};
```

To define Heuristics for concrete Games:

```cpp
#include "Heuristic.h"
#include "NPuzzle.h"

class NPuzzle_Heuristic: public Heuristic{
    private:
        // In private, we (programmers) know that NPuzzle uses TileState and NPuzzle
        double score(TileState* ts, NPuzzle *np);
    public:
        NPuzzle_Heuristic();
        virtual ~NPuzzle_Heuristic();
        // Public consistent interface wrapper for internal score function
        virtual double score(State* s, Game* g);
};
```

Game-specific Heuristics is not Game-agnostic and would require the inclusion of concrete Game header file to access a description of internal derived State, Action, and Game. Here, we override the `score(State* s, Game* g)` function but use it as a wrapper for dynamic_casting the given State and Game pointers into the corresponding TileState and NPuzzle specific to this Heuristic (should throw an error otherwise if we're given pointers to incorrectly-typed objects).

## Writing Agents to solve Generic 'Games'

Finally, to write Agents that interact with described Games:

```cpp
class Agent{
    public:
        virtual int solve(std::vector<std::shared_ptr<Action>>& va) = 0;
};
```

We should implement the solve function that writes to a vector of Actions.

> Extension: Perhaps we should define derived SearchAgent and have a solve(..., State* s) for running multiple solves of the same Game

Once again, Agents should be Game-agnostic, meaning only functions described in the abstract interface of Action, State, and Game should be available for use in Agents. Write Heuristics to assess game states and query the Game instance for state transitions.

> Issue: What about Actions on continuous space? Unlikely for a problem to generate all possible (State, Action) transitions. How Game-Agnostic could we be in that case? Likely will have to derive a new Action that has a continuous \_specifier instead? Should the Agent then know this? Also, why are Actions and States external to Games? Doesn't a Game require a specific class of Action and State that is used only for this Game and no other?

## Getting Sokoban level files

Website: https://www.sourcecode.se/sokoban/levels has a good repository of level files

python script `sokoban_reformat.py` runs through the text file downloaded from the above website to generate various .in files for each level described.
