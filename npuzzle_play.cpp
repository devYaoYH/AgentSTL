#include "src/game/PlayableGame.h"
#include "src/game/NPuzzle.h"
#include "src/agent/PlayerAgent.h"
#include <getopt.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>

//Note: exposing shared_ptr<>.get() pointer is probably a bad design pattern...

int main(int argc, char* argv[]){

    // Parse argument on dimension of NPuzzle
    int dim_x = 3; int dim_y = 3;
    int scramble_num = 1000000;
    int c, d;
    while((c = getopt(argc, argv, "s:x:y:n:")) != -1){
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
            case '?':
                if (optopt == 'n')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                return 1;

        }
    }

    NPuzzle* np = new NPuzzle(dim_y, dim_x);
    std::shared_ptr<State> goal = np->get_goal_state();

    if (!goal.get()){
        std::cout << "Goal is not specified" << std::endl;
        return 1;
    }
    std::cout << goal.get();
    std::cout << "This is the goal state: " << np->is_goal_state(goal.get()) << " (should be 1)" << std::endl;
    
    np->scramble(scramble_num);
    std::shared_ptr<State> current_state = np->get_state();
    std::cout << std::endl << "Original problem start state after scrambling:" << std::endl;
    std::cout << np;
    std::cout << "This is the goal state: " << np->is_goal_state(current_state.get()) << " (should be 0)" << std::endl;

    // Start our search agent (initialize with playable version of sokoban)
    PlayerAgent* player = new PlayerAgent(new PlayableGame(np));

    // Solve our puzzle (hopefully)
    std::vector<std::shared_ptr<Action>> ans;
    int run_code = player->solve(ans);

    if (run_code){
        std::cerr << "(main) Error: solver threw error code: " << run_code << std::endl;
        // Cleanup
        delete np;
        delete player;
        return run_code;
    }

    // Print solution
    for (std::shared_ptr<Action> a: ans){
        np->play(a.get());
    }
    std::cout << "Found solution in " << ans.size() << " moves" << std::endl;
    current_state = np->get_state();
    std::cout << np << std::endl;
    std::cout << "This is the goal state: " << np->is_goal_state(current_state.get()) << std::endl;
    
    // Cleanup
    delete np;
    delete player;
    return 0;
}
