#include "src/game/Sokoban.h"
#include "src/heuristic/SokobanHeuristic.h"
#include "src/agent/AstarSearchAgent.h"
#include <getopt.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <chrono>
#include <string>

#define SOK_DB_LIM 254        // Number of patterns to generate

//Note: exposing shared_ptr<>.get() pointer is probably a bad design pattern...

int help(){
    printf("Usage:  ./sokoban_test -p <astar|all> [-f: level file]\n");
    return 1;
}

int main(int argc, char* argv[]){

    // Parse argument for init Sokoban level
    double weight = 1;
    int c, d;
    char* in_file = nullptr;
    std::string algo = "None";
    while((c = getopt(argc, argv, "f:w:p:")) != -1){
        switch(c){
            case 'f':
                in_file = optarg;
                break;
            case 'w':
                weight = std::atoi(optarg);
                break;
            case 'p':
                algo = std::string(optarg);
                break;
            case '?':
                if (optopt == 'n')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                return help();
        }
    }

    // Debug chosen parameters:
    printf("Running Sokoban Test:\n");
    if (in_file) printf("File: %s\n",in_file);
    else printf("File: None (console input)\n");
    printf("Weight: %.3lf\n",weight);
    printf("Algo: %s\nContinue? [ENTER]",algo.c_str());
    //std::string tmp; getline(std::cin, tmp);
    std::cout << std::endl;

    // Check if we have agent (save us some startup time)
    if (algo.compare("all") && algo.compare("astar") && algo.compare("all")){
        return help();
    }

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
                    for (int y=0;y<r;++y) free(game_board[y]);
                    free(game_board);
                    game_board = 0;
                    return 3;
                }
            }
            ++y;
        }

        // Create new sokoban game
        sokoban = new Sokoban(r, c, game_board, true);    // Meant for Human players, no pruning
        if (!sokoban->valid()){
            std::cerr << "ERROR: Game Board is invalid" << std::endl;
            // Cleanup
            for (int y=0;y<r;++y) free(game_board[y]);
            free(game_board);
            game_board = 0;
            return 4;
        }

        // Cleanup
        for (int y=0;y<r;++y) free(game_board[y]);
        free(game_board);
        game_board = 0;
    }
    else{
        // Seed by asking player to type in board
        std::cout << "Input board dimensions (row, columns): " << std::endl;
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
                    for (int y=0;y<r;++y) free(game_board[y]);
                    free(game_board);
                    game_board = 0;
                    return 3;
                }
            }
            ++y;
        }

        // Create new sokoban game
        sokoban = new Sokoban(r, c, game_board, true);    // Meant for Human players, no pruning
        if (!sokoban->valid()){
            std::cerr << "ERROR: Game Board is invalid" << std::endl;
            // Cleanup
            for (int y=0;y<r;++y) free(game_board[y]);
            free(game_board);
            game_board = 0;
            return 4;
        }

        // Cleanup
        for (int y=0;y<r;++y) free(game_board[y]);
        free(game_board);
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
    std::cout << std::endl << goal.get();
    std::cout << "This is the goal state: " << sokoban->is_goal_state(goal.get()) << " (should be 1)" << std::endl;

    // Grab a puzzle heuristic class
    Heuristic* sokoban_heu = new SokobanHeuristic();
    std::vector<Heuristic*> hs = {sokoban_heu};

    // Start our search agent (initialize with problem & heuristic)
    // Spawn search agents based on input string
    std::vector<Agent*> agents;
    if (algo.compare("all") == 0){
        Agent* astar_search = new AstarSearchAgent(sokoban, sokoban_heu, weight);
        agents.push_back(astar_search);
    }
    else if (algo.compare("astar") == 0){
        Agent* astar_search = new AstarSearchAgent(sokoban, sokoban_heu, weight);
        agents.push_back(astar_search);
    }
    else{
        return help();
    }
    int status = 0;
    for(int i = 0; i<agents.size(); i++){
        // Solve our puzzle (hopefully)
        std::vector<std::shared_ptr<Action>> ans;
        auto start = std::chrono::high_resolution_clock::now();
        int run_code = agents[i]->solve(ans);
        auto stop = std::chrono::high_resolution_clock::now();

        if (run_code){
            std::cerr << "(main) Error: solver threw error code: " << run_code << std::endl;
            status = run_code;
            break;
        }

        // Print solution
        Sokoban copy(*sokoban);
        double tot_cost = 0.0;
        for (std::shared_ptr<Action> a: ans){
            tot_cost += a->_cost;
            copy.play(a.get());
        }
        std::cout << "Found solution in " << ans.size() << " moves (" << (int)tot_cost << " steps)" << std::endl;
        std::cout << "Took " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " milliseconds" << std::endl;
        current_state = copy.get_state();
        std::cout << copy << std::endl;
        std::cout << "This is the goal state: " << copy.is_goal_state(current_state.get()) << std::endl;
    }

    // Cleanup
    delete sokoban;
    for(int i = 0; i<agents.size(); i++){
        delete agents[i];
    }
    for(int i = 0; i<hs.size(); i++){
        delete hs[i];
    }
    return status;
}
