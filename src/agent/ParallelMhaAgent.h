#pragma once

#include "Agent.h"
#include "../game/Game.h"
#include "../heuristic/Heuristic.h"
#include "MhaAgent.h"
#include <vector>
#include <memory>
#include <ctime>
#include <cstdlib>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <limits>
#include <queue>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer.h>
#include <cilk/reducer_list.h>
#include <mutex>
#include <atomic>

//#define DEBUG

class ParallelImhaSearchAgent: public HeuristicSearchAgent{
    typedef std::pair<double, std::shared_ptr<AugmentedState>> open_item;
    private:
        long reexpand;
        long move_open;
        long impossible_states_in_open;
        long iters;
        long parallel_ms;
        double _w;
        double _goal_g;
        double _max_p_closed_a;

        std::mutex _g_lock;
        
        int expand(open_item &s, int index);

        std::unordered_set<std::shared_ptr<const State>,
            StatePointerHash, DerefCompare> _closed_a;
        std::unordered_map<std::shared_ptr<const State>,
            std::shared_ptr<AugmentedState>, StatePointerHash,
            DerefCompare> _closed_u;
        std::vector<std::set<open_item, OpenItemCompare>> _open;

        cilk::reducer< cilk::op_list_append<std::shared_ptr<AugmentedState>>> _remove;
        cilk::reducer< cilk::op_list_append<std::shared_ptr<AugmentedState>>> _openupdate;
        
        typedef decltype(_open.begin()->begin()) open_set_iter_t;
        /*
         * _openlocs maintains a list of iterators (one into each open set) so that
         * we can look up in constant time the position of a given open_item
         * given a state. Used so that we can `decrease` the priority of
         * AugmentedState's in the open sets rather than just inserting copies
         * https://stackoverflow.com/questions/6438086/iterator-invalidation-rules
         * indicates that saving iterators into a set is safe
         */
        typedef std::unordered_map<std::shared_ptr<const State>, open_set_iter_t, StatePointerHash, DerefCompare> loc_map;
        std::vector<std::shared_ptr<loc_map>> _openlocs;
    public:
        // By default, the subopt. factor is 1 (find an optimal solution).
        ParallelImhaSearchAgent(Game *g, std::vector<Heuristic*> &heuristics, double w=1);
        int solve(std::vector<std::shared_ptr<Action>>& va) override;
        bool term_criterion(const std::shared_ptr<const AugmentedState> &as, double comp) override;
        bool p_criterion(const std::shared_ptr<const AugmentedState> &as) override;
        bool p_criterion_always_false(const std::shared_ptr<const AugmentedState> &as) override;
        double priority(const std::shared_ptr<const AugmentedState> &as) override;
};
