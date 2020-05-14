#pragma once

#include "../game/Game.h"
#include "../game/PlayableGame.h"
#include "Agent.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>

class PlayerAgent: public Agent{
    protected:
        PlayableGame* game;
        bool get_action(const State* s, std::string& key);
    public:
    	// PlayerAgent need to OWN this instance of PlayableGame
    	// i.e. PlayableGame pointer should NOT be shared between multiple PlayerAgents
        PlayerAgent(PlayableGame* g);
        // Preserves similar interface as other Agents
        virtual int solve(std::vector<std::shared_ptr<Action>>& va) override;
        virtual ~PlayerAgent();
};
