#include "src/game/NPuzzle.h"
#include "src/heuristic/NPuzzleHeuristic.h"
#include "src/agent/AstarSearchAgent.h"
#include <getopt.h>
#include <iostream>
#include <memory>
#include <chrono>
#include <string>

//Note: exposing shared_ptr<>.get() pointer is probably a bad design pattern...

int help(){
    printf("Usage: ./npuzzle_test -p <astar|all> [-n: n*n dims] [-x: dim_x] [-y: dim_y] [-w: weight] [-s scrambles]\n");
    return 1;
}

int main(int argc, char* argv[]){

    // Parse argument on dimension of NPuzzle
    int dim_x = 3; int dim_y = 3;
    int scramble_num = 1000;
    double weight = 1;
    int c, d;
    std::string algo = "None";
    while((c = getopt(argc, argv, "s:x:y:n:w:p:")) != -1){
        switch(c){
            case 'n':
                d = std::atoi(optarg);
                dim_x = d;
                dim_y = d;
                break;
            case 'x':
                dim_x = std::atoi(optarg);
                break;
            case 'y':
                dim_y = std::atoi(optarg);
                break;
            case 's':
                scramble_num = std::atoi(optarg);
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
    
    // Check if we have agent (save us some startup time)
    if (algo.compare("all") && algo.compare("astar") && algo.compare("all")){
        return help();
    }

    // Debug chosen parameters:
    printf("Running %dx%d NPuzzle Test:\n",dim_x,dim_y);
    printf("Scrambles: %d\n",scramble_num);
    printf("Weight: %.3lf\n",weight);
    printf("Algo: %s\n",algo.c_str());
    std::cout << std::endl;

    NPuzzle* np = new NPuzzle(dim_y, dim_x);
    std::shared_ptr<State> goal = np->get_goal_state();

    if (!goal.get()){
        std::cout << "Goal is not specified" << std::endl;
        return 1;
    }
    std::cout << goal.get();
    std::cout << "This is the goal state: " << np->is_goal_state(goal.get()) << " (should be 1)" << std::endl;
    
    np->scramble(scramble_num);

    Heuristic* np_heu = new NPuzzleHeuristic();
    std::vector<Heuristic*> hs = {np_heu};

    // Start our search agent (initialize with problem & heuristic)
    // Spawn search agents based on input string
    std::vector<Agent*> agents;
    if (algo.compare("all") == 0){
        Agent* astar_search = new AstarSearchAgent(np, np_heu, weight);
        agents.push_back(astar_search);
    }
    else if (algo.compare("astar") == 0){
        Agent* astar_search = new AstarSearchAgent(np, np_heu, weight);
        agents.push_back(astar_search);
    }
    else{
        return help();
    }
    int status = 0;
    for(int i = 0; i<agents.size(); i++){
        std::shared_ptr<State> current_state = np->get_state();
        std::cout << std::endl << "Original problem start state after scrambling:" << std::endl;
        std::cout << np;
        std::cout << "This is the goal state: " << np->is_goal_state(current_state.get()) << " (should be 0)" << std::endl;
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
        NPuzzle copy(*np);
        for (std::shared_ptr<Action> a: ans){
            copy.play(a.get());
        }
        std::cout << "Found solution in " << ans.size() << " moves" << std::endl;
        std::cout << "Took " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " milliseconds" << std::endl;
        current_state = copy.get_state();
        std::cout << copy << std::endl;
        std::cout << "This is the goal state: " << copy.is_goal_state(current_state.get()) << std::endl;
    }

    // Cleanup
    delete np;
    for(int i = 0; i<agents.size(); i++){
        delete agents[i];
    }
    for(int i = 0; i<hs.size(); i++){
        delete hs[i];
    }
    return status;
}
