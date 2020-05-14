#include "Sokoban.h"

// (x, y) coordinate pair
typedef std::pair<int, int> pii;
typedef std::unordered_set<pii, PairHash> set_pii;
typedef std::unordered_map<pii, int, PairHash> dist_map;
typedef std::unordered_map<pii, dist_map, PairHash> loc_dist_map;

/////////////
// Sokoban //
/////////////
Sokoban::Sokoban(int r, int c):_rows(r),_cols(c),_goal_state(nullptr),_prune(true){
    // Initialize our distance map
    for (int x=0;x<c;++x){
        for (int y=0;y<r;++y){
            // This should default initialize our dist_map?
            distance[pii(x,y)][pii(x,y)] = 0;
        }
    }
}

Sokoban::Sokoban(int r, int c, bool p):Sokoban(r, c){
    _prune = p;
}

Sokoban::Sokoban(int r, int c, char** raw):Sokoban(r, c){
    // In addition to setting up stuff, prepare _goal_state and _state with raw
    if (!load_board(r, c, raw)){
        std::cerr << "(Sokoban::constructor) Error: board is not valid" << std::endl;
        // Set our states to nullptr
        _state = nullptr;
        _goal_state = nullptr;
    }
}

Sokoban::Sokoban(int r, int c, char** raw, bool p):Sokoban(r, c, raw){
    _prune = p;
}

Sokoban::Sokoban(const Sokoban& sok):_rows(sok._rows),_cols(sok._cols),_goal_state(nullptr),_prune(sok._prune){
    // Initialize our distance map
    for (loc_dist_map::const_iterator fit = sok.distance.begin();fit != sok.distance.end();++fit){
        // Copy contents of each map
        for (dist_map::const_iterator sit = fit->second.begin();sit != fit->second.end();++sit){
            distance[fit->first][sit->first] = sit->second;
        }
    }
    _state = new BoardState(*dynamic_cast<BoardState*>(sok._state));
    _goal_state = new BoardState(*sok._goal_state);
}

bool Sokoban::valid(){
    return _state && _goal_state;
}

// Generate _goal_state and parse _state from char* raw
bool Sokoban::load_board(int r, int c, char** raw){
    BoardState* new_state = new BoardState(r, c);
    int conut = 0;  // Counter for allocating unique box_ids
    for (int y=0;y<r;++y){
        for (int x=0;x<c;++x){
            // Read into raw
            pii cur = pii(x, y);
            switch (raw[y][x]){
                case '.':
                    break;
                case 'x':
                    new_state->_boxes[cur] = conut++;
                    break;
                case 'o':
                    new_state->_player_loc = cur;
                    break;
                case '#':
                    new_state->_walls.insert(cur);
                    break;
                case '_':
                    new_state->_goals.insert(cur);
                    break;
                case '@':
                    new_state->_boxes[cur] = conut++;
                    new_state->_goals.insert(cur);
                    break;
                case '!':
                    new_state->_goals.insert(cur);
                    new_state->_player_loc = cur;
                    break;
                default:
                    std::cerr << "(Sokoban::load_board) unrecognized char: " << raw[y][x] << std::endl;
                    break;
            }
        }
    }
    
    // Check if input is formatted correctly
    if (new_state->_player_loc.first < 0 || new_state->_player_loc.second < 0 || new_state->_boxes.size() != new_state->_goals.size()){
        std::cerr << new_state;
        delete new_state;
        return false;
    }

    // Valid, we compute _traversible from initial player location
    bfs(*new_state, new_state->_player_loc, new_state->_traversible, true);

    // Construct _goal_state from _state
    _goal_state = new BoardState(r, c);
    conut = 0;
    for (const pii& p: new_state->_goals){
        _goal_state->_boxes[p] = conut++;
        _goal_state->_goals.insert(p);
    }
    for (const pii& p: new_state->_walls){
        _goal_state->_walls.insert(p);
    }
    // doesn't matter where we're putting the player
    // just that it is in a viable location
    for (int x=0;x<c;++x){
        if (_goal_state->is_valid(_goal_state->_player_loc)) break;
        for (int y=0;y<r;++y){
            pii cur = pii(x, y);
            if (!_goal_state->is_box(cur) && !_goal_state->is_wall(cur)){
                _goal_state->_player_loc = cur;
                break;
            }
        }
    }
    // Doesn't matter for the _goal_state whether we have populated _traversible either
    // since that is never checked for in is_goal_state

    // Finally, [expensive!] populate distance hashmap via BFS on every cell passable
    for (int x=0;x<c;++x){
        for (int y=0;y<r;++y){
            pii cur = pii(x, y);
            // Run bfs from this location if viable (not a wall)
            if (!_goal_state->is_wall(cur)){
                bfs(*_goal_state, cur, distance[cur]);
            }
        }
    }

    // Test our hashmap
    // #ifdef DEBUG
    // std::cout << new_state;
    // int x1,x2,y1,y2;
    // std::cout << "Test hashmap (x1,y1)->(x2,y2): ";
    // std::cin >> x1 >> y1 >> x2 >> y2;
    // std::cout << distance[pii(x1,y1)][pii(x2,y2)] << std::endl;
    // #endif

    // Transfer ownership of new_state into _state
    _state = new_state;
    new_state = 0;  // Make sure we release control of pointer from stack variable new_state

    return true;
}

std::shared_ptr<State> Sokoban::get_state(){
    return std::make_shared<BoardState>(*dynamic_cast<BoardState*>(_state));
}

std::shared_ptr<State> Sokoban::get_goal_state(){
    return std::make_shared<BoardState>(*_goal_state);
}

bool Sokoban::is_goal_state(const State* s){
    const BoardState* bs = dynamic_cast<const BoardState*>(s);
    if (!bs){
        return false;   // Failure to receive correctly-typed state for comparison
    }
    // Simply check if we have boxes in same locations as goal state
    if (bs->_boxes.size() != _goal_state->_boxes.size()) return false;
    for (const std::pair<pii, int>& box_pii: bs->_boxes){
        if (!_goal_state->is_box(box_pii.first)){
            return false;
        }
    }
    return true;
}

//Yiheng:   This appears to be quite a standard function using get_actions and play_action,
//          can be abstracted higher up the inheritance tree?
int Sokoban::get_successors(const State* s, std::vector<std::pair<std::shared_ptr<State>,std::shared_ptr<Action>>>& v){
    // Assert that we have the correct type of State
    const BoardState* bs = dynamic_cast<const BoardState*>(s);
    if (!bs) return ERR_CODE::STATE_TYPE_ERROR;

    // Run 'simulation' using our valid actions
    std::vector<std::shared_ptr<Action>> va;
    int ret_code = get_actions(s, va);
    if (ret_code){
        std::cerr << "(Sokoban::get_successors) Error: " << ret_code << std::endl;
        return ret_code;
    }

    // Generate copies of states
    for (std::shared_ptr<Action> a: va){
        // Make copy of current state
        std::shared_ptr<State> new_state = std::make_shared<BoardState>(*bs);
        // Play current state to next state
        if (play_action(new_state.get(), a.get())){
            // Push new state into vector of (successors,action) pairs
            v.push_back(std::make_pair(new_state, a));   
        }
    }

    // Return success
    return ERR_CODE::SUCCESS;
}

int Sokoban::get_actions(const State* s, std::vector<std::shared_ptr<Action>>& v){
    // Assert that we have the correct type of State
    const BoardState* bs = dynamic_cast<const BoardState*>(s);
    if (!bs) return ERR_CODE::STATE_TYPE_ERROR;

    // Decide whether we are in _prune mode
    if (_prune){
        // Efficient moves only -> e.g. only moves that will shift a box
        // For moves which affect box locations, it sufficies to look at the
        // intersection between _traversible and _boxes sets
        for (const std::pair<pii, int>& box_pii: bs->_boxes){ // _boxes.size() < _traversible.size() (so more efficient this way)
            pii p = box_pii.first;
            if (bs->is_traversible(p)){
                // Valid box to be pushed to new location
                // Generate which direction the box can be pushed from
                for (int i=0;i<4;++i){
                    int ax = -ADJ[i][0];
                    int ay = -ADJ[i][1];
                    pii from_loc = pii(p.first + ax, p.second + ay);
                    // Need to check in _traversible and NOT in _boxes since _boxes is subset of _traversible from bfs call
                    if (bs->is_traversible(from_loc) && !bs->is_box(from_loc)){
                        // Location to push box from is traversible by player
                        // Validate end location of box
                        pii new_box_loc = pii(p.first - ax, p.second - ay);
                        if (!bs->is_valid(new_box_loc) || bs->is_wall(new_box_loc) || bs->is_box(new_box_loc)) continue;
                        // Valid action of type push_move found
                        std::ostringstream os;
                        os << "MOVE " << p.first << " " << p.second << " PUSH " << MOVE_DIR[i];
                        std::string name = os.str();
                        int dist = bs->get_dist(from_loc);  // Number of steps to move into position to push box + 1 for push
                        // We should be able to reach unless we have not set _player_loc correctly
                        assert(dist < std::numeric_limits<int>::max());
                        v.push_back(std::make_shared<PositionAction>(action_types::push_move, (double)(dist+1), name, p, i));
                    }
                }
            }
        }
    }
    else{
        // Step-wise movement (only? - for now)
        for (int i=0;i<4;++i){
            // No need for action validation (play_action will validate)
            std::ostringstream os;
            os << MOVE_DIR[i];
            std::string name = os.str();
            v.push_back(std::make_shared<PositionAction>(action_types::step_move, 1.0, name, i));
        }
        #ifdef DEBUG
        v.push_back(std::make_shared<PositionAction>(action_types::debug_hash, -1.0, "DEBUG", -1));
        #endif
    }

    return ERR_CODE::SUCCESS;
}

void Sokoban::display(std::ostream& os){
    os << _state;   // Should defer call to operator<<(ostream&, State*)?
}

Sokoban::~Sokoban(){
    delete _goal_state;
}

pii Sokoban::get_dims() const{
    return pii(_cols, _rows);
}

int Sokoban::get_rev_actions(const BoardState* bs, std::vector<std::shared_ptr<PositionAction>>& v){
    if (!_prune){
        std::cerr << "(Sokoban) Cannot call .get_rev_actions() on human-playable game (for now)" << std::endl;
        return ERR_CODE::REV_ACTION_UNAVAILABLE;
    }
    for (const std::pair<pii, int>& box_pii: bs->_boxes){ // _boxes.size() < _traversible.size() (so more efficient this way)
        pii p = box_pii.first;
        // From an uninitialized goal state, any boxes can be moved (don't know where player will end up)
        if (bs->_traversible.size() == 0 || bs->is_traversible(p)){
            // Valid box to be pushed to new location
            // Generate which direction the box can be pushed from
            for (int i=0;i<4;++i){
                int ax = -ADJ[i][0];
                int ay = -ADJ[i][1];
                pii orig_box_loc = pii(p.first + ax, p.second + ay);
                pii orig_player_loc = pii(orig_box_loc.first + ax, orig_box_loc.second + ay);
                // Need to check in _traversible and NOT in _boxes since _boxes is subset of _traversible from bfs call
                if ((bs->_traversible.size() == 0 || bs->is_traversible(orig_player_loc)) && !bs->is_box(orig_box_loc) && !bs->is_wall(orig_box_loc) && !bs->is_box(orig_player_loc) && !bs->is_wall(orig_player_loc)){
                    // Valid action of type push_move found
                    std::ostringstream os;
                    os << "MOVE " << p.first << " " << p.second << " PUSH " << MOVE_DIR[i];
                    std::string name = os.str();
                    int dist = 1;
                    if (bs->_traversible.size() > 0){
                        dist = bs->get_dist(orig_player_loc);  // Number of steps to move into position to push box + 1 for push
                    }
                    v.push_back(std::make_shared<PositionAction>(action_types::push_move, (double)(dist+1), name, orig_box_loc, i));
                }
            }
        }
    }

    return ERR_CODE::SUCCESS;
}

bool Sokoban::play_rev_action(BoardState* bs, PositionAction* pa){
    // Assess whether this is a move command or jump-to and push
    // depending on which, check for action validity
    if (pa->_dir >= 4 && pa->_dir < 0) return false;
    // Parse valid action
    pii box_orig_loc = bs->is_valid(pa->_move_loc)?pa->_move_loc:bs->_player_loc;
    int ax = ADJ[pa->_dir][0];
    int ay = ADJ[pa->_dir][1];
    pii box_current_loc = pii(box_orig_loc.first + ax, box_orig_loc.second + ay);
    pii player_orig_loc = pii(box_orig_loc.first - ax, box_orig_loc.second - ay);
    switch (pa->_specifier){
        case action_types::push_move:
            if (!bs->is_valid(box_orig_loc) || !bs->is_valid(box_current_loc) || !bs->is_valid(player_orig_loc)) return false;
            if (!bs->is_box(box_current_loc)) return false;
            break;
        case action_types::debug_hash:
            std::cerr<< bs->hash() << std::endl; return false;
            break;
        default:
            return false;
    }
    // Apply action
    if (pa->_specifier == action_types::step_move){
        // Only need to move player by 1 step, checking if box exists to be pushed
        if (bs->is_box(box_current_loc)){
            // Check for new box location validity
            if (!bs->is_valid(box_orig_loc) || bs->is_wall(box_orig_loc) || bs->is_box(box_orig_loc)) return false;
            // Remove box
            int box_id = bs->_boxes[box_current_loc];
            bs->_boxes.erase(box_current_loc);
            // Add new box location
            bs->_boxes[box_orig_loc] = box_id;
        }
        // Move player to new location
        bs->_player_loc = player_orig_loc;
        // Recompute _traversible
        bs->_traversible.clear();
        bfs(*bs, bs->_player_loc, bs->_traversible, true);
        return true;
    }
    else if (pa->_specifier == action_types::push_move){
        // We know a box exists where player moves onto
        // Check for new box location validity
        if (!bs->is_valid(box_orig_loc) || bs->is_wall(box_orig_loc) || bs->is_box(box_orig_loc) || bs->is_box(player_orig_loc) || bs->is_wall(player_orig_loc)) return false;
        // Remove box
        int box_id = bs->_boxes[box_current_loc];
        bs->_boxes.erase(box_current_loc);
        // Add new box location
        bs->_boxes[box_orig_loc] = box_id;
        // Move player to new location
        bs->_player_loc = player_orig_loc;
        // Recompute _traversible
        bs->_traversible.clear();
        bfs(*bs, bs->_player_loc, bs->_traversible, true);
        return true;
    }
    else return false;
}

int Sokoban::play(Action* a){
    bool play_succeeded = play_action(_state, a);
    if (play_succeeded) return ERR_CODE::SUCCESS;
    else return ERR_CODE::PLAY_FAILED;
}

bool Sokoban::play_action(State* s, Action* a){
    //NOTE: remember to update s->_traversible by running BFS from new _player_loc after movement

    // Firstly, cast to correctly typed objects
    PositionAction* pa = dynamic_cast<PositionAction*>(a);
    if (!pa) return false;
    BoardState* bs = dynamic_cast<BoardState*>(s);
    if (!bs) return false;

    // Sanity check: assume _traversible has been correctly populated as of construction
    if (bs->_traversible.size() < 1){
        std::cerr << "(Sokoban::play_action) Error! BoardState does not have _traversible initialized! Make sure to compute _traversible upon construction of new BoardState" << std::endl;
        std::cerr << bs;
        return false;
    }

    // Assess whether this is a move command or jump-to and push
    // depending on which, check for action validity
    switch (pa->_specifier){
        case action_types::step_move:
            if (pa->_dir >= 4 && pa->_dir < 0) return false;
            break;
        case action_types::push_move:
            if (pa->_dir >= 4 && pa->_dir < 0) return false;
            if (!bs->is_valid(pa->_move_loc)) return false;
            if (!bs->is_box(pa->_move_loc)) return false;
            // Make sure player can actually move here from prior location
            if (!bs->is_traversible(pa->_move_loc)) return false;
            break;
        case action_types::debug_hash:
            std::cerr<< bs->hash() << std::endl; return false;
            break;
        default:
            return false;
    }
    // Parse valid action
    pii new_player_loc = bs->is_valid(pa->_move_loc)?pa->_move_loc:bs->_player_loc;
    int ax = ADJ[pa->_dir][0];
    int ay = ADJ[pa->_dir][1];
    pii new_action_loc = pii(new_player_loc.first + ax, new_player_loc.second + ay);
    // Check if new action location is valid
    if (!bs->is_valid(new_action_loc) || bs->is_wall(new_action_loc)) return false;
    // Apply action
    if (pa->_specifier == action_types::step_move){
        // Only need to move player by 1 step, checking if box exists to be pushed
        if (bs->is_box(new_action_loc)){
            // Compute new box location
            pii new_box = pii(new_action_loc.first + ax, new_action_loc.second + ay);
            // Check for new box location validity
            if (!bs->is_valid(new_box) || bs->is_wall(new_box) || bs->is_box(new_box)) return false;
            // Remove box
            int box_id = bs->_boxes[new_action_loc];
            bs->_boxes.erase(new_action_loc);
            // Add new box location
            bs->_boxes[new_box] = box_id;
        }
        // Move player to new location
        bs->_player_loc = new_action_loc;
        // Recompute _traversible
        bs->_traversible.clear();
        bfs(*bs, bs->_player_loc, bs->_traversible, true);
        return true;
    }
    else if (pa->_specifier == action_types::push_move){
        // We know a box exists where player moves onto
        // Check for new box location validity
        if (!bs->is_valid(new_action_loc) || bs->is_wall(new_action_loc) || bs->is_box(new_action_loc)) return false;
        // Remove box
        int box_id = bs->_boxes[new_player_loc];
        bs->_boxes.erase(new_player_loc);
        // Add new box location
        bs->_boxes[new_action_loc] = box_id;
        // Move player to new location
        bs->_player_loc = new_player_loc;
        // Recompute _traversible
        bs->_traversible.clear();
        bfs(*bs, bs->_player_loc, bs->_traversible, true);
        return true;
    }
    else return false;
}

////////////////
// BoardState //
////////////////
BoardState::BoardState(const BoardState& bs):_rows(bs._rows),_cols(bs._cols),_player_loc(bs._player_loc){
    // Insert contents from bs into own sets
    //TODO: does stl operator= copy assignemnt shallow-copy sets?
    for (const pii& p: bs._walls){
        _walls.insert(p);
    }
    for (const std::pair<pii, int>& box_pii: bs._boxes){
        _boxes[box_pii.first] = box_pii.second;
    }
    for (const pii& p: bs._goals){
        _goals.insert(p);
    }
    for (const std::pair<pii, int>& dist_map_p: bs._traversible){
        _traversible[dist_map_p.first] = dist_map_p.second;
    }
}

BoardState::BoardState(int r, int c):_rows(r),_cols(c),_player_loc(pii(-1,-1)){}

//Yiheng:   I know I wrote this... but is it ever used?
BoardState& BoardState::operator=(BoardState& other){
    _rows = other._rows;
    _cols = other._cols;
    _player_loc = other._player_loc;
    _walls.clear();
    for (const pii& p: other._walls){
        _walls.insert(p);
    }
    _boxes.clear();
    for (const std::pair<pii, int>& box_pii: other._boxes){
        _boxes[box_pii.first] = box_pii.second;
    }
    _goals.clear();
    for (const pii& p: other._goals){
        _goals.insert(p);
    }
    _traversible.clear();
    for (const std::pair<pii, int>& dist_map_p: other._traversible){
        _traversible[dist_map_p.first] = dist_map_p.second;
    }
    return *this;
}

// We DO NOT need to check player location
// rather, we check equality between _traversible sets
bool BoardState::operator==(const State& other) const{
    try{
        const BoardState& bs = dynamic_cast<const BoardState&>(other);
        if (!(_rows == bs._rows && _cols == bs._cols)) return false;
        // Check for _boxes, _walls, _goals, finally, _traversible
        if (!(_boxes.size() == bs._boxes.size() && 
            _walls.size() == bs._walls.size() && 
            _goals.size() == bs._goals.size() && 
            _traversible.size() == bs._traversible.size())) return false;
        // Check for contents
        for (const std::pair<pii, int>& box_pii: _boxes){
            if (!bs.is_box(box_pii.first)) return false;
        }
        for (const pii& p: _walls){
            if (!bs.is_wall(p)) return false;
        }
        for (const pii& p: _goals){
            if (!bs.is_goal(p)) return false;
        }
        for (const std::pair<pii, int>& dist_map_p: _traversible){
            if (!bs.is_traversible(dist_map_p.first)) return false;
            // In no cases should the value of this hashmap actually differ at this point
            // all else being equal
        }
        return true;
    }catch(std::bad_cast){
        // If it is not the same type of state they can't be equal
        return false;
    }
}

// Inequality using equality
bool BoardState::operator!=(const State& other) const{
    return !(*this == other);
}

// Display
// Slightly more involved here since we have to 'unpack' the compressed state
void BoardState::display(std::ostream& os) const{
    for (int y=0;y<_rows;++y){
        for (int x=0;x<_cols;++x){
            pii cur = pii(x, y);
            if (is_wall(cur)) os << '#';
            else if (is_box(cur) && is_goal(cur)) os << '@';
            else if (_player_loc.first == x && _player_loc.second == y && is_goal(cur)) os << '!';
            else if (_player_loc.first == x && _player_loc.second == y) os << 'o';
            // else if (is_box(cur)) os << _boxes.find(cur)->second%10;
            else if (is_box(cur)) os << 'x';
            else if (is_goal(cur)) os << '_';
            else os << '.';
        }
        os << std::endl;
    }
}

pii BoardState::get_player_loc() const{
    return _player_loc;
}

pii BoardState::get_dims() const{
    return pii(_cols, _rows);
}

const dist_map& BoardState::get_boxes() const{
    return _boxes;
}

const set_pii& BoardState::get_goals() const{
    return _goals;
}

bool BoardState::is_wall(int x, int y) const{
    return is_wall(pii(x,y));
}

bool BoardState::is_box(int x, int y) const{
    return is_box(pii(x,y));
}

bool BoardState::is_goal(int x, int y) const{
    return is_goal(pii(x,y));
}

bool BoardState::is_wall(const pii& loc) const{
    return _walls.find(loc) != _walls.end();
}

bool BoardState::is_box(const pii& loc) const{
    return _boxes.find(loc) != _boxes.end();
}

bool BoardState::is_goal(const pii& loc) const{
    return _goals.find(loc) != _goals.end();
}

bool BoardState::is_valid(int x, int y) const{
    return is_valid(pii(x,y));
}

bool BoardState::is_valid(const pii& loc) const{
    return loc.first >= 0 && loc.first < _cols && loc.second >= 0 && loc.second < _rows;
}

bool BoardState::is_traversible(int x, int y) const{
    return is_traversible(pii(x,y));
}

bool BoardState::is_traversible(const pii& loc) const{
    return _traversible.find(loc) != _traversible.end();
}

int BoardState::get_dist(const pii& loc) const{
    if (is_traversible(loc)){
        dist_map::const_iterator it = _traversible.find(loc);
        return it->second;
    }
    return std::numeric_limits<int>::max();
}

pii BoardState::get_box(int id) const{
    for (const std::pair<pii, int> box_pii: _boxes){
        if (box_pii.second == id) return box_pii.first;
    }
    return pii(-1,-1);
}

size_t BoardState::hash() const{
    const size_t prime = 511;
    size_t _hash = 0;
    // get a pairhash hasher
    PairHash hasher;
    for (const pii& p: _walls) _hash += hasher(p);
    _hash *= prime;
    for (const std::pair<pii, int> box_pii: _boxes) _hash += hasher(box_pii.first);
    _hash *= prime;
    for (const pii& p: _goals) _hash += hasher(p);
    _hash *= prime;
    for (const std::pair<pii, int>& dist_map_p: _traversible){
        _hash += hasher(dist_map_p.first);
    }
    _hash *= prime;
    // Once again, we do not need to hash _player_loc since unless _traversible changes
    // player has effectively not moved at all
    return _hash;
}

/////////
// BFS //
/////////
void bfs(BoardState& bs, set_pii& v){
    bfs(bs, bs._player_loc, v);
}

// At the end of things, we will have a record of traversed locations written into set_pii v
// check membership of locations bs._boxes in this set_pii v to recover moved boxes
void bfs(BoardState& bs, pii& loc, set_pii& v){
    int ADJ[4][2] = {{0,-1},{1,0},{0,1},{-1,0}};
    std::queue<pii> q;
    q.push(loc);
    v.insert(loc);
    while (!q.empty()){
        pii cur = q.front(); q.pop();
        // Stop expanding if we are on a box
        if (bs.is_box(cur)){
            continue;
        }
        // Expand from current location
        for (int i=0;i<4;++i){
            int ax = ADJ[i][0] + cur.first;
            int ay = ADJ[i][1] + cur.second;
            pii next_loc = pii(ax, ay);
            // Expand if not in v, is_valid and is not wall
            if (v.find(next_loc) == v.end() && bs.is_valid(next_loc) && !bs.is_wall(next_loc)){
                q.push(next_loc);
                v.insert(next_loc);
            }
        }
    }
}

// BFS and track distances
void bfs(BoardState& bs, pii& loc, dist_map& map, bool stop_box){
    int ADJ[4][2] = {{0,-1},{1,0},{0,1},{-1,0}};
    typedef std::pair<pii, int> pii_dist;
    std::queue<pii_dist> q;
    q.push(pii_dist(loc, 0));
    map[loc] = 0;
    while (!q.empty()){
        pii_dist q_front = q.front(); q.pop();
        pii cur = q_front.first;
        int dist = q_front.second;
        // Stop expanding if we are on a box
        if (stop_box && bs.is_box(cur)){
            continue;
        }
        // Expand from current location
        for (int i=0;i<4;++i){
            int ax = ADJ[i][0] + cur.first;
            int ay = ADJ[i][1] + cur.second;
            pii next_loc = pii(ax, ay);
            int next_dist = dist + 1;
            // Expand if not in v, is_valid and is not wall
            if (map.find(next_loc) == map.end() && bs.is_valid(next_loc) && !bs.is_wall(next_loc)){
                q.push(pii_dist(next_loc, next_dist));
                map[next_loc] = next_dist;
            }
        }
    }
}

/////////////
// Display //
/////////////
std::ostream& operator<<(std::ostream& os, BoardState& s){
    s.display(os);
    return os;
}

std::ostream& operator<<(std::ostream& os, Sokoban& g){
    g.display(os);
    return os;
}
