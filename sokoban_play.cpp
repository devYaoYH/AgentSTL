#include "src/game/PlayableGame.h"
#include "src/game/Sokoban.h"
#include "src/agent/PlayerAgent.h"
#include <getopt.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>

void howtoplay(){
    printf("Usage: ./sokoban [-f: file to read game level from]\n");
    printf("Welcome to Sokoban! The following describes the meaning of characters used to display the playable board.\n");
    printf(".    : Empty cell\n");
    printf("x    : Box\n");
    printf("o    : Player starting location\n");
    printf("#    : Wall\n");
    printf("_    : Goal position for Box\n");
    printf("@    : Goal position with Box\n");
    printf("!    : Goal position with player\n");
    printf("Push all the boxes into goals to WIN! Good Luck.\n\n");
}

//Note: exposing shared_ptr<>.get() pointer is probably a bad design pattern...

int main(int argc, char* argv[]){

    // Read in a file to init Sokoban Playing board
    char c;
    char* in_file = nullptr;
    while((c = getopt(argc, argv, "f:")) != -1){
        switch(c){
            case 'f':
                in_file = optarg;
                break;
            case '?':
                if (optopt == 'n')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                return 1;
        }
    }

    howtoplay();

    // Pointer to our game
    Sokoban* sokoban;

    if (in_file){
        // Have input file, read game board from file
        std::ifstream fin;
        fin.open(in_file);
        if (!fin){
            std::cerr << "ERROR: <" << in_file << "> not found" << std::endl;
            return 1;
        }
        // File has the following format
        // .    : Empty cell
        // x    : Box
        // o    : Player starting location
        // #    : Wall
        // _    : Goal position for Box
        // @    : Goal position with Box
        //IMPT! remember to add ending newline to file!!!

        // Parse file (Board Dimensions)
        std::string line;
        int r, c;
        if (!getline(fin, line)){
            std::cerr << "ERROR: Unable to read game board dimensions from file" << std::endl;
            return 2;
        }
        else{
            std::istringstream iss(line);
            iss >> r >> c;
            if (iss.fail()){
                std::cerr << "ERROR: Unable to read game board dimensions from file" << std::endl;
                return 2;
            }
        }
        
        // Create 2D char array for Sokoban init
        char** game_board = (char**)malloc(sizeof(char*)*r);
        for (int y=0;y<r;++y){
            game_board[y] = (char*)malloc(sizeof(char)*c);
        }
        // Read into board
        int y = 0;
        while (!getline(fin, line).eof() && y < r){
            std::istringstream iss(line);
            for (int x=0;x<c;++x){
                iss>>game_board[y][x];
                if (iss.fail()){
                    std::cerr << "ERROR: Unable to read game cell" << std::endl;
                    // Cleanup
                    for (int y=0;y<r;++y) delete game_board[y];
                    delete game_board;
                    game_board = 0;
                    return 3;
                }
            }
            ++y;
        }

        // Create new sokoban game
        sokoban = new Sokoban(r, c, game_board, false);    // Meant for Human players, no pruning
        if (!sokoban->valid()){
            std::cerr << "ERROR: Game Board is invalid" << std::endl;
            // Cleanup
            for (int y=0;y<r;++y) delete game_board[y];
            delete game_board;
            game_board = 0;
            return 4;
        }

        // Cleanup
        for (int y=0;y<r;++y) delete game_board[y];
        delete game_board;
        game_board = 0;
    }
    else{
        // Seed by asking player to type in board
        std::cout << "Input board dimensions (row, columns): " << std::endl; std::cout.flush();
        int r, c;
        std::cin >> r >> c; std::cin.ignore();
        // Create 2D char array for Sokoban init
        char** game_board = (char**)malloc(sizeof(char*)*r);
        for (int y=0;y<r;++y){
            game_board[y] = (char*)malloc(sizeof(char)*c);
        }
        std::string line;
        // Read into board
        int y = 0;
        while (!getline(std::cin, line).eof() && y < r){
            std::istringstream iss(line);
            for (int x=0;x<c;++x){
                iss>>game_board[y][x];
                if (iss.fail()){
                    std::cerr << "ERROR: Unable to read game cell" << std::endl;
                    // Cleanup
                    for (int y=0;y<r;++y) delete game_board[y];
                    delete game_board;
                    game_board = 0;
                    return 3;
                }
            }
            ++y;
        }

        // Create new sokoban game
        sokoban = new Sokoban(r, c, game_board, false);    // Meant for Human players, no pruning
        if (!sokoban->valid()){
            std::cerr << "ERROR: Game Board is invalid" << std::endl;
            // Cleanup
            for (int y=0;y<r;++y) delete game_board[y];
            delete game_board;
            game_board = 0;
            return 4;
        }

        // Cleanup
        for (int y=0;y<r;++y) delete game_board[y];
        delete game_board;
        game_board = 0;
        //std::cerr << "Seeding board by typing is currently not supported" << std::endl;
        //return 5;
    }

    std::cout << sokoban;

    std::shared_ptr<State> current_state = sokoban->get_state();
    std::shared_ptr<State> goal = sokoban->get_goal_state();

    if (!goal.get()){
        std::cout << "Goal is not specified" << std::endl;
        return 1;
    }
    std::cout << goal.get();
    std::cout << "This is the goal state: " << sokoban->is_goal_state(goal.get()) << " (should be 1)" << std::endl;

    std::cout.flush();
    
    // Grab a puzzle heuristic class
    //Heuristic* sokoban_heu = new SokobanHeuristic();

    // Start our search agent (initialize with playable version of sokoban)
    PlayerAgent* player = new PlayerAgent(new PlayableGame(sokoban));

    // Solve our puzzle (hopefully)
    std::vector<std::shared_ptr<Action>> ans;
    int run_code = player->solve(ans);

    if (run_code){
        std::cerr << "(main) Error: solver threw error code: " << run_code << std::endl;
        // Cleanup
        delete sokoban;
        delete player;
        // delete sokoban_heu;
        return run_code;
    }

    // Print solution
    for (std::shared_ptr<Action> a: ans){
        sokoban->play(a.get());
    }
    std::cout << "Found solution in " << ans.size() << " moves" << std::endl;
    current_state = sokoban->get_state();
    std::cout << sokoban << std::endl;
    std::cout << "This is the goal state: " << sokoban->is_goal_state(current_state.get()) << std::endl;
    
    // Cleanup
    delete sokoban;
    delete player;
    // delete sokoban_heu;
    return 0;
}
