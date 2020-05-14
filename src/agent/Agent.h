#pragma once

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

class Agent{
    protected:
        Game* search_problem;
    public:
        Agent(Game* sp):search_problem(sp){};
        virtual int solve(std::vector<std::shared_ptr<Action>>& va) = 0;
        virtual ~Agent(){search_problem = 0;};
};
