#pragma once

#include "Agent.h"
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
#include <atomic>

class HeuristicSearchAgent: public Agent{
    protected:
        std::vector<Heuristic *> &_heuristics;

        struct AugmentedState{
          public:
            std::shared_ptr<const State> _state;    // Keep this as a shared ptr
            std::shared_ptr<Action> _action;        // Keep this as a shared ptr
            double _g;
            std::vector<double> _h_vals;
            bool _closed_a_flag;
            std::atomic_flag _closed_u_flag;
            bool _closed_u_danger_flag;
            std::shared_ptr<const AugmentedState> _bp;
            AugmentedState(std::shared_ptr<const State> s, std::shared_ptr<Action> a,
                std::shared_ptr<const AugmentedState> bp):
              _state(s), _action(a), _bp(bp), _closed_a_flag(false), 
              _closed_u_flag(ATOMIC_FLAG_INIT), _closed_u_danger_flag(false){};
            AugmentedState(std::shared_ptr<const State> s, std::shared_ptr<Action> a,
                double g, std::shared_ptr<const AugmentedState> bp):
              _state(s), _action(a), _g(g), _bp(bp), _closed_a_flag(false), 
              _closed_u_flag(ATOMIC_FLAG_INIT), _closed_u_danger_flag(false){};
            AugmentedState():_state(nullptr), _action(nullptr), _g(0), 
                _bp(nullptr), _closed_a_flag(false), 
                _closed_u_flag(ATOMIC_FLAG_INIT), _closed_u_danger_flag(false){};
            AugmentedState(std::shared_ptr<const State> s, std::shared_ptr<Action> a,
                double g, std::vector<double> h_vals, std::shared_ptr<const AugmentedState> bp):
                _state(s), _action(a), _g(g), _h_vals(h_vals), _bp(bp), _closed_a_flag(false),
                _closed_u_flag(ATOMIC_FLAG_INIT), _closed_u_danger_flag(false){};
        };

        typedef std::pair<double, std::shared_ptr<AugmentedState>> open_item;
        struct OpenItemCompare{
            bool operator()(const open_item &a, const open_item &b){
                return (a.first < b.first ||
                    (a.first == b.first && a.second->_state != b.second->_state));
            }
        };
    public:
        HeuristicSearchAgent(Game* g, std::vector<Heuristic*> &heuristics);
        virtual ~HeuristicSearchAgent() = default;
        virtual int solve(std::vector<std::shared_ptr<Action>>& va) override = 0;
        virtual bool term_criterion(const std::shared_ptr<const AugmentedState> &as, double comp) = 0;
        virtual bool p_criterion(const std::shared_ptr<const AugmentedState> &as) = 0;
        virtual bool p_criterion_always_false(const std::shared_ptr<const AugmentedState> &as) {
            return false;
        }
        virtual double priority(const std::shared_ptr<const AugmentedState> &as) = 0;
};

class SerialImhaSearchAgent: public HeuristicSearchAgent{
    typedef std::pair<double, std::shared_ptr<AugmentedState>> open_item;
    private:
        long reexpand;
        double _w;
        double _max_p_closed_a;
        int expand(std::pair<double, std::shared_ptr<AugmentedState>> &s);
        std::unordered_set<std::shared_ptr<const State>,
            StatePointerHash, DerefCompare> _closed_a;
        // _closed_u needs to remember the AugmentedState corresponding to
        // this state when we got inadmissibly expanded in case we need to get
        // reinserted into _open. Technically we only need to remember the _g
        // value but storing a pointer to the whole AugmentedState allows us
        // to avoid recomputing the heuristics when we reinsert
        std::unordered_map<std::shared_ptr<const State>,
            std::shared_ptr<AugmentedState>, StatePointerHash,
            DerefCompare> _closed_u;

        std::vector<std::set<open_item, OpenItemCompare>> _open;

        typedef decltype(_open.begin()->begin()) open_set_iter_t;
        /*
         * _locs maintains a list of iterators (one into each open set) so that
         * we can look up in constant time the position of a given open_item
         * given a state. Used so that we can `decrease` the priority of
         * AugmentedState's in the open sets rather than just inserting copies
         * https://stackoverflow.com/questions/6438086/iterator-invalidation-rules
         * indicates that saving iterators into a set is safe
         */
        std::unordered_map<std::shared_ptr<const State>, std::vector<open_set_iter_t>, StatePointerHash, DerefCompare> _locs;
        void fill_locs_end(std::vector<open_set_iter_t> &vec);
    public:
        // By default, the subopt. factor is 1 (find an optimal solution).
        SerialImhaSearchAgent(Game *g, std::vector<Heuristic*> &heuristics, double w=1);
        int solve(std::vector<std::shared_ptr<Action>>& va) override;
        bool term_criterion(const std::shared_ptr<const AugmentedState> &as, double comp) override;
        bool p_criterion(const std::shared_ptr<const AugmentedState> &as) override;
        double priority(const std::shared_ptr<const AugmentedState> &as) override;
};
