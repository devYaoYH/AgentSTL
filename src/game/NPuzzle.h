#pragma once

#include "Game.h"
#include <ctime>
#include <cstdlib>
#include <memory>

// Typedef a pair<int, int> object for Co-ordinate use
// pii p = pii(r,c); int y = p.first; int x = p.second;
typedef std::pair<int, int> pii;

// Forward declare
class Tile;
class TileState;
class NPuzzle;

class Tile{
    friend std::ostream& operator<<(std::ostream& os, Tile& t);
    friend TileState;
    private:
        // std::pair<int, int> object to store (x,y) locations
        pii _home_position;
        // flag to say if this is a real tile or a placeholder
        bool _real;
    public:
        // Copy Constructor
        Tile(const Tile& t);
        // Initialize Constructor
        // pass by reference vector (copy into self)
        Tile(pii h, bool r);
        // Default Destructor, only primitives used (safe)
        ~Tile() = default;
        // Assignment (copy over stuff no swap)
        Tile& operator=(Tile& other);
        // Comparators
        bool operator==(const Tile& other) const;
        bool operator!=(const Tile& other) const;
        // Display
        void display(std::ostream& os) const;
};

class TileState: public State{
    friend std::ostream& operator<<(std::ostream& os, const TileState& s);
    friend NPuzzle;
    private:
        // 2D vector of Tile* pointers
        std::vector<std::vector<std::shared_ptr<Tile>>> _tiles;
        pii _empty_space;
        int _rows, _cols;
    public:
        // Copy Constructor
        TileState(const TileState& ts);
        // Initialize Constructor with size of board
        TileState(int r, int c);
        // Destructor, free Tiles
        virtual ~TileState();
        // Assignment (copy over stuff no swap)
        TileState& operator=(TileState& other);
        // Comparators
        bool operator==(const State& other) const override;
        bool operator!=(const State& other) const override;
        // Display
        virtual void display(std::ostream& os) const override;
        // Getters (don't expose our Tile internals)
        pii get_tile_home_position(int x, int y) const;
        bool get_tile_real(int x, int y) const;
        // Hash function for set membership
        size_t hash() const override;
        void scramble(int moves);
};

class NPuzzle: public Game{
    friend std::ostream& operator<<(std::ostream& os, NPuzzle& g);
    private:
        int _rows,_cols;
	std::shared_ptr<TileState> _goal_state;
        // Static 'Standard' actions
        std::shared_ptr<Action> actions[4] = {
            std::make_shared<Action>(legal_actions::north,1.0,"N"),
            std::make_shared<Action>(legal_actions::east,1.0,"E"),
            std::make_shared<Action>(legal_actions::south,1.0,"S"),
            std::make_shared<Action>(legal_actions::west,1.0,"W")
        };
    public:
        // Legal actions to be taken
        enum legal_actions{
            north   = 0,
            east    = 1,
            south   = 2,
            west    = 3
        };
        // Make visible some error codes (to return from get_successors || get_actions)
        enum ERR_CODE{
            SUCCESS                 = 0x0,
            STATE_TYPE_ERROR        = 0x1,
            STATE_NO_SUCCESSORS     = 0x2,
            STATE_NO_ACTIONS        = 0x4,
            PLAY_FAILED             = 0x8
        };

        // Constructor specifying row and columns in N-Puzzle
        NPuzzle(int r, int c);
        //NPuzzle(const State &np);
        NPuzzle(const NPuzzle &np);
        // Scramble with random moves of 'fake' tile (default 1000)
        void scramble(int moves=1000);
        // Scramble a copy of the current state and return a pointer to the copy
        // Should abstract this out into something you can call on a state so
        // that we don't repeat code from scramble. Requires modifying
        // get_actions and play_action so that they can be called directly
        // on a state = a bit more involved than just redefining one function.
        TileState* scramble_copy(int moves);
        // Grab the current state
        virtual std::shared_ptr<State> get_state() override;
        // Grab the goal state
        virtual std::shared_ptr<State> get_goal_state() override;
        // Assess whether given state is goal
        virtual bool is_goal_state(const State* s) override;
        // Populate a vector of successor states given state
        // @return ERR_CODE
        virtual int get_successors(const State* s, std::vector<std::pair<std::shared_ptr<State>,std::shared_ptr<Action>>> &v) override;
        // Populate a vector of actions given state
        // @return ERR_CODE
        virtual int get_actions(const State* s, std::vector<std::shared_ptr<Action>> &v) override;
        // Prints the current state of the game
        virtual void display(std::ostream& os) override;
        virtual ~NPuzzle();
        // Getters
        pii get_dims() const;

        ////////////////////////////////
        // Makes the Game interactive //
        ////////////////////////////////
        // Do action on the game
        // @return ERR_CODE
        virtual int play(Action* a) override;
        // Play an action on a single state by MUTATING passed state
        //@return true or false depending on whether action is valid for this state
        virtual bool play_action(State* s, Action* a) override;
};

// Display functions
std::ostream& operator<<(std::ostream& os, Tile& t);
std::ostream& operator<<(std::ostream& os, const TileState& s);
std::ostream& operator<<(std::ostream& os, NPuzzle& g);
