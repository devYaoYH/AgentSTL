#include "NPuzzle.h"

#define ABS(x) ((x)>0?(x):(-(x)))

// Typedef a pair<int, int> object for Co-ordinate use
// pii p = pii(r,c); int y = p.first; int x = p.second;
typedef std::pair<int, int> pii;

NPuzzle::NPuzzle(int r, int c): _rows(r), _cols(c){
    // Initialize a single Start State (holds onto all Tile pointers for us)
    // In the entire lifetime of NPuzzle, there will only be r*c Tile objects
    this->_state = new TileState(r, c);
    // Copy construct (no deep copy of Tile objects)
    //this->_goal_state = new TileState(*dynamic_cast<TileState*>(this->_state));
    this->_goal_state = std::make_shared<TileState>(*dynamic_cast<TileState*>(this->_state));
}

NPuzzle::NPuzzle(const NPuzzle &np):_rows(np._rows), _cols(np._cols),
    _goal_state(np._goal_state)
{
    this->_state = new TileState(*dynamic_cast<TileState*>(np._state));
}

NPuzzle::~NPuzzle(){
    // _state is owned by Parent Game and deleted there
    //delete _goal_state;
}

bool NPuzzle::is_goal_state(const State* s){
    const TileState* ts = dynamic_cast<const TileState*>(s);
    // Assess whether is goal state
    if (!this->_goal_state || !ts){
        std::cerr << "Error, NPuzzle has no starting state OR input state is void" << std::endl;
        return false;
    }
    return *ts == *this->_goal_state;
}

std::shared_ptr<State> NPuzzle::get_state(){
    return std::make_shared<TileState>(*dynamic_cast<TileState*>(this->_state));
}

std::shared_ptr<State> NPuzzle::get_goal_state(){
    return std::make_shared<TileState>(*this->_goal_state);
}

bool NPuzzle::play_action(State* s, Action* a){
    TileState* cur_state = dynamic_cast<TileState*>(s);
    int x = cur_state->_empty_space.first;
    int y = cur_state->_empty_space.second;
    int nx = -1;
    int ny = -1;
    switch(a->_specifier){
        case NPuzzle::legal_actions::north:
            nx = x + ADJ[0][0]; ny = y + ADJ[0][1];
            break;
        case NPuzzle::legal_actions::east:
            nx = x + ADJ[1][0]; ny = y + ADJ[1][1];
            break;
        case NPuzzle::legal_actions::south:
            nx = x + ADJ[2][0]; ny = y + ADJ[2][1];
            break;
        case NPuzzle::legal_actions::west:
            nx = x + ADJ[3][0]; ny = y + ADJ[3][1];
            break;
    }
    // We can generalize cases if we stick to NESW (0,1,2,3) mapping
    // int nx = x + ADJ[a->_specifier][0];
    // int ny = y + ADJ[a->_specifier][1];
    // Valid Adj Cell exists (within bounds)
    if (nx >= 0 && ny >= 0 && nx < this->_cols && ny < this->_rows){
        std::shared_ptr<Tile> tmp_north = std::shared_ptr<Tile>(cur_state->_tiles[nx][ny]);
        // Swap the two pointers
        cur_state->_tiles[nx][ny] = cur_state->_tiles[x][y];
        cur_state->_tiles[x][y] = tmp_north;
        // Update our _empty_space
        cur_state->_empty_space.first = nx;
        cur_state->_empty_space.second = ny;
        return true;
    }
    return false;
}

//TODO: Finish implementing this with vector<shared_ptr<State>> stuff
void NPuzzle::scramble(int moves){
    // Run series of random moves
    //srand(time(NULL));
    srand(23);
    for (int i=0;i<moves;++i){
        std::vector<std::shared_ptr<Action>> va;
        // Grab next available actions
        int ret_code = this->get_actions(this->_state, va);
        if (ret_code){
            std::cerr << "(NPuzzle::scramble) Error: " << ret_code << std::endl;
            return;
        }
        // Randomly pick an action to play
        int idx = rand() % va.size();
        //std::cerr << "(NPuzzle::scramble) move: " << va[idx]->_specifier << std::endl;
        // Play this action on current state
        play_action(this->_state, va[idx].get());
        //std::cout << *this << std::endl;
    }
}

// Mostly repeated with scramble, need to refactor TileState to fix so that
// we can get_action and play_action directly on a state.
TileState* NPuzzle::scramble_copy(int moves){
    // Run series of random moves
    srand(time(NULL));
    //srand(23);
    TileState* tmp_state = dynamic_cast<TileState*>(_state);
    if(!tmp_state){
        std::cerr << "(NPuzzle::scramble_copy) Error: _state is not a TileState*" << std::endl;
        return nullptr;
    }
    TileState* copy_state = new TileState(*tmp_state);
    Action* last_action = nullptr;
    for (int i=0;i<moves;++i){
        std::vector<std::shared_ptr<Action>> va;
        // Grab next available actions
        int ret_code = this->get_actions(copy_state, va);
        if (ret_code){
            std::cerr << "(NPuzzle::scramble) Error: " << ret_code << std::endl;
            return nullptr;
        }
        // Randomly pick an action to play
        int idx = rand() % va.size();
        // Don't undo the last action if possible
        if(last_action && ABS(va[idx]->_specifier - last_action->_specifier) == 2){
            idx = (idx + 1) % va.size();
        }
        //std::cerr << "(NPuzzle::scramble) move: " << va[idx]->_specifier << std::endl;
        // Play this action on current state
        play_action(copy_state, va[idx].get());
        //std::cout << *this << std::endl;
    }
    return copy_state;
}

int NPuzzle::get_successors(const State* s, std::vector<std::pair<std::shared_ptr<State>,std::shared_ptr<Action>>> &v){
    // Assert that we have the correct type of state
    const TileState* ts = dynamic_cast<const TileState*>(s);

    // If not of the right type, throw STATE_TYPE_ERROR
    if (!ts) return NPuzzle::ERR_CODE::STATE_TYPE_ERROR;

    // Run 'simulation' using our valid actions
    std::vector<std::shared_ptr<Action>> va;
    int ret_code = get_actions(s, va);
    if (ret_code){
        std::cerr << "(NPuzzle::get_successors) Error: " << ret_code << std::endl;
        return ret_code;
    }

    // Generate copies of states
    for (std::shared_ptr<Action> a: va){
        // Make copy of current state
        std::shared_ptr<State> new_state = std::make_shared<TileState>(*ts);
        // Play current state to next state
        play_action(new_state.get(), a.get());
        // Push new state into vector of (successors,action) pairs
        v.push_back(std::make_pair(new_state, a));
    }

    // Return success
    return NPuzzle::ERR_CODE::SUCCESS;
}

int NPuzzle::get_actions(const State* s, std::vector<std::shared_ptr<Action>> &v){
    // Assert that we have the correct type of state
    const TileState* ts = dynamic_cast<const TileState*>(s);

    // If not of the right type, throw STATE_TYPE_ERROR
    if (!ts) return NPuzzle::ERR_CODE::STATE_TYPE_ERROR;

    // Check for moves given state (NESW ordering)
    for (int i=0;i<4;++i){
        // Grab a standard cardinal action
        // Note we are NOT creating a new Action object here, just grabbing another pointer to it
        std::shared_ptr<Action> a = std::shared_ptr<Action>(this->actions[i]);
        // Test validity and push it into the actions vector
        int nx = ts->_empty_space.first + ADJ[i][0];
        int ny = ts->_empty_space.second + ADJ[i][1];
        if (nx >= 0 && ny >= 0 && nx < this->_cols && ny < this->_rows){
            v.push_back(a);
        }
    }

    return NPuzzle::ERR_CODE::SUCCESS;
}

void NPuzzle::display(std::ostream& os){
    if (!this->_state) return;  // Make sure we have a state first
    os << this->_state << std::endl;
}

// Interactive Functions (mutates _current_state)
int NPuzzle::play(Action* a){
    if (play_action(this->_state, a)){
        return NPuzzle::ERR_CODE::SUCCESS;
    }
    return NPuzzle::ERR_CODE::PLAY_FAILED;
}

// Getters (for dimensions)
pii NPuzzle::get_dims() const{
    return pii(this->_cols, this->_rows);
}

//////////
// Tile //
//////////
// Replicate another Tile
Tile::Tile(const Tile& t):_home_position(t._home_position), _real(t._real){}

// New Tile
Tile::Tile(pii h, bool r):_home_position(h), _real(r){} //TODO: is copy-constructor for std::pair deep?

// Copy Assignment no swap
Tile& Tile::operator=(Tile& other){
    std::cerr << "(Tile) Copy and NO SWAP" << std::endl;
    this->_home_position.first = other._home_position.first;
    this->_home_position.second = other._home_position.second;
    this->_real = other._real;
    return *this;
}

// Display current tile
void Tile::display(std::ostream& os) const{
    os << " (" << this->_home_position.first << "," << this->_home_position.second << ") [" << this->_real << "] ";
}

// Equality
bool Tile::operator==(const Tile& other) const{
    return this->_home_position == other._home_position && this->_real == other._real;
}

// Inequality (using equality)
bool Tile::operator!=(const Tile& other) const{
    return !(*this == other);
}

///////////////
// TileState //
///////////////
// Replicate another TileState (deep)
TileState::TileState(const TileState& ts):_empty_space(ts._empty_space), _rows(ts._rows), _cols(ts._cols){
    for (int x=0;x<this->_cols;++x){
        std::vector<std::shared_ptr<Tile>> row;
        for (int y=0;y<this->_rows;++y){
            // Note: Not making another Tile Object, just grab a pointer to it
            row.push_back(std::shared_ptr<Tile>(ts._tiles[x][y]));
        }
        this->_tiles.push_back(row);
    }
}

// Create new Grid of Tiles
// This is the only place where we are initializing new Tile objects
TileState::TileState(int r, int c):_empty_space(pii(0,0)), _rows(r), _cols(c){
    for (int x=0;x<this->_cols;++x){
        std::vector<std::shared_ptr<Tile>> row;
        for (int y=0;y<this->_rows;++y){
            if (x != 0 || y != 0){ //Actual Tile
                std::shared_ptr<Tile> newTile = std::make_shared<Tile>(pii(x,y),true);
                row.push_back(newTile);
            }
            else{ //Placeholder Tile
                std::shared_ptr<Tile> fakeTile = std::make_shared<Tile>(pii(x,y),false);
                row.push_back(fakeTile);
            }
        }
        this->_tiles.push_back(row);
    }
}

// Copy Assignment (no swap)
TileState& TileState::operator=(TileState& other){
    std::cerr << "(TileState) Copy and NO SWAP" << std::endl;
    for (int x=0;x<this->_cols;++x){
        for (int y=0;y<this->_rows;++y){
            this->_tiles[x][y] = other._tiles[x][y];
        }
    }
    return *this;
}

// Display the current TileState
void TileState::display(std::ostream& os) const{
    for (int y=0;y<this->_rows;++y){
        for (int x=0;x<this->_cols;++x){
            os << *this->_tiles[x][y] << " ";
        }
        os << std::endl;
    }
    os << std::endl;
}

// Equality
bool TileState::operator==(const State& other) const{
    try{
        const TileState &tother = dynamic_cast<const TileState&>(other);
    	if (!(this->_rows == tother._rows && this->_cols == tother._cols)) return false;
    	if (this->_empty_space != tother._empty_space) return false;
    	// Traverse grid and equate all Tiles
    	for (int x=0;x<this->_cols;++x){
    		for (int y=0;y<this->_rows;++y){
    			if (*(this->_tiles[x][y]) != *(tother._tiles[x][y])) return false;
    		}
    	}
    	return true;
    }catch(std::bad_cast){
        // If it is not the same type of state they can't be equal
        return false;
    }
}

// Inequality using equality
bool TileState::operator!=(const State& other) const{
    return !(*this == other);
}

TileState::~TileState(){
    // Free up resources
    for (std::vector<std::shared_ptr<Tile>> vt: this->_tiles){
        vt.clear();
    }
    this->_tiles.clear();
}

// Getters
pii TileState::get_tile_home_position(int x, int y) const{
    return pii(this->_tiles[x][y]->_home_position.first, this->_tiles[x][y]->_home_position.second);
}

bool TileState::get_tile_real(int x, int y) const{
    return this->_tiles[x][y]->_real;
}

size_t TileState::hash() const{
    const size_t prime = 511;
    size_t hash = 0;
    for(int i = 0; i < _tiles.size(); i++){
        for(int j = 0; j < _tiles[i].size(); j++){
            pii hp = _tiles[i][j]->_home_position;
            hash = (hash + (i*hp.first + j*hp.second)) * prime;
        }
    }
    return hash;
}

/////////////
// DISPLAY //
/////////////
// Display functions
std::ostream& operator<<(std::ostream& os, Tile& t){
    t.display(os);
    return os;
}

std::ostream& operator<<(std::ostream& os, const TileState& s){
    s.display(os);
    return os;
}

std::ostream& operator<<(std::ostream& os, NPuzzle& g){
    g.display(os);
    return os;
}
