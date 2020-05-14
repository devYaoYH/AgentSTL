#pragma once

#include "Game.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <cassert>
#include <iostream>
#include <sstream>
#include <limits>

// We appear to need a hash for pair (stl doesn't have one o.O)
struct PairHash
{
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2> &pair) const
    {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};

// Used to store board coordinates
typedef std::pair<int, int> pii;
// Used to store set of object locations
typedef std::unordered_set<pii, PairHash> set_pii;
// Used to memoize distances to locations via bfs step-distance
typedef std::unordered_map<pii, int, PairHash> dist_map;

// Forward declare our class
class Sokoban;

struct PositionAction: Action{
    // Where to move to
    pii _move_loc;
    // Direction to push box to (if any)
    int _dir;
    PositionAction(int spec, double cost):Action(spec,cost),_move_loc(pii(-1,-1)),_dir(-1){}
    PositionAction(int spec, double cost, std::string name):Action(spec,cost,name),_move_loc(pii(-1,-1)),_dir(-1){}
    PositionAction(int spec, double cost, std::string name, int dir):Action(spec,cost,name),_move_loc(pii(-1,-1)),_dir(dir){}
    PositionAction(int spec, double cost, std::string name, pii loc):Action(spec,cost,name),_move_loc(loc),_dir(-1){}
    PositionAction(int spec, double cost, std::string name, pii loc, int dir):Action(spec,cost,name),_move_loc(loc),_dir(dir){}
    virtual ~PositionAction(){};
};

// State for us to store our Sokoban board
// Note: this is a 'compressed' state (it is not a 2D char array)
class BoardState: public State{
    friend std::ostream& operator<<(std::ostream& os, BoardState& s);
    friend void bfs(BoardState& bs, set_pii& v);
    friend void bfs(BoardState& bs, pii& loc, set_pii& v);
    friend void bfs(BoardState& bs, pii& loc, dist_map& map, bool stop_box);
    friend Sokoban;
    // Used to store set of object locations
    typedef std::unordered_set<pii, PairHash> set_pii;
    // Used to store traversible locations and cost to reach
    typedef std::unordered_map<pii, int, PairHash> dist_map;
    // Describe what we need to know to re-create state of Sokoban board
    private:
        // Dimensions of board
        int _rows, _cols;
        // Player location
        pii _player_loc;
        // Sets of objects
        set_pii _walls,_goals;
        // Map of box with matching id
        dist_map _boxes;
        // Traversible player locations (used for efficient state equality checking)
        dist_map _traversible;
    public:
        // Copy Constructor
        BoardState(const BoardState& bs);
        // Initialize Constructor with empty sets ready for insertion
        BoardState(int r, int c);
        // Destructor (do nothing)
        virtual ~BoardState(){};
        // Assignment (copy over stuff no swap)
        BoardState& operator=(BoardState& other);
        // Comparators
        bool operator==(const State& other) const override;
        bool operator!=(const State& other) const override;
        // Display
        virtual void display(std::ostream& os) const override;
        // Getter for player location
        pii get_player_loc() const;
        // Getter for dimensions
        pii get_dims() const;
        // Getter for boxes
        const dist_map& get_boxes() const;
        // Getter for goals
        const set_pii& get_goals() const;
        // Getters to test for existence of stuff
        bool is_wall(int x, int y) const;
        bool is_box(int x, int y) const;
        bool is_goal(int x, int y) const;
        bool is_wall(const pii& loc) const;
        bool is_box(const pii& loc) const;
        bool is_goal(const pii& loc) const;
        // Test if a given location is valid
        bool is_valid(int x, int y) const;
        bool is_valid(const pii& loc) const;
        // Test if is traversible and get cost
        bool is_traversible(int x, int y) const;
        bool is_traversible(const pii& loc) const;
        int get_dist(const pii& loc) const;    // From _player_loc
        pii get_box(int id) const;  // O(b), expensive?
        // Hash function for set membership
        size_t hash() const override;
};

class Sokoban: public Game{
    friend std::ostream& operator<<(std::ostream& os, Sokoban& g);
    typedef std::unordered_map<pii, int, PairHash> dist_map;
    typedef std::unordered_map<pii, dist_map, PairHash> loc_dist_map;
    typedef std::unordered_set<pii, PairHash> set_pii;
    private:
        int _rows, _cols;
        BoardState* _goal_state;    // We need to generate this given starting board layout
        // Memoize distance[pii][pii] = int
        loc_dist_map distance;
        // Whether to prune 1-step travel only moves (boxes don't move)
        // technically, we are in the same effective state but makes easier for human player to follow
        bool _prune;    // Whether or not to prune should not be changed past the constructor
                        // thus, we have no getter/setter for this
    public:
        // Legal actions to be taken
        enum action_types{
            step_move   = 0,
            push_move   = 1,
            debug_hash  = 2
        };
        // Make visible some error codes (to return from get_successors || get_actions)
        enum ERR_CODE{
            SUCCESS                 = 0x0,
            STATE_TYPE_ERROR        = 0x1,
            STATE_NO_SUCCESSORS     = 0x2,
            STATE_NO_ACTIONS        = 0x4,
            PLAY_FAILED             = 0x8,
            REV_ACTION_UNAVAILABLE  = 0x10
        };
        // Constructor specifying row and columns in game board
        Sokoban(int r, int c);
        Sokoban(int r, int c, bool p);
        // Further specify 2D char input array to build state from
        // 2D char array has the following format
        // .    : Empty cell
        // x    : Box
        // o    : Player starting location
        // #    : Wall
        // _    : Goal position for Box
        // @    : Goal position with Box
        // !    : Goal position with player
        //IMPT! remember to add ending newline to file!!!
        Sokoban(int r, int c, char** raw);
        Sokoban(int r, int c, char** raw, bool p);
        // Copy Constructor
        Sokoban(const Sokoban& sok);
        // Test for whether we have constructed a valid board
        bool valid();
        // Loading boards into an already created game
        // WARNING: will mutate _goal_state AND _state AND (loc_dist_map) distance
        bool load_board(int r, int c, char** raw);
        // Grab the current state (returns mutable)
        virtual std::shared_ptr<State> get_state() override;
        // Grab the goal state (returns mutable)
        virtual std::shared_ptr<State> get_goal_state() override;
        // Assess whether given state is goal
        virtual bool is_goal_state(const State* s) override;
        // Populate a vector of successor states given state
        // @return ERR_CODE
        virtual int get_successors(const State* s, std::vector<std::pair<std::shared_ptr<State>,std::shared_ptr<Action>>>& v) override;
        // Populate a vector of actions given state
        // @return ERR_CODE
        virtual int get_actions(const State* s, std::vector<std::shared_ptr<Action>>& v) override;
        // Prints the current state of the game
        virtual void display(std::ostream& os) override;
        virtual ~Sokoban();
        // Getters
        // return (x,y)
        pii get_dims() const;

        ////////////////////////////////
        // Specific Sokoban functions //
        ////////////////////////////////
        // Since game is not undirected (returns 'forward' actions leading to this state)
        int get_rev_actions(const BoardState* bs, std::vector<std::shared_ptr<PositionAction>>& v);
        // Play a forwards action in reverse
        bool play_rev_action(BoardState* bs, PositionAction* pa);

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

// BFS function | Given BoardState, pii location, unordered_set<pii> visited
void bfs(BoardState& bs, set_pii& v);    // Implicitly takes bs._player_loc as loc
void bfs(BoardState& bs, pii& loc, set_pii& v);

// Given BoardState and location, write to some hashmap and memoize distances
void bfs(BoardState& bs, pii& loc, dist_map& map, bool stop_box=false);

// Display functions
std::ostream& operator<<(std::ostream& os, BoardState& s);
std::ostream& operator<<(std::ostream& os, Sokoban& g);
