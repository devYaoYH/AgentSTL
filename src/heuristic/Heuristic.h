#pragma once

#include "../game/Game.h"
#include <cmath>

// Define a short helper function for manhattan distances between two pair objects
template <class T>
T manhattan_distance(std::pair<T, T> p1, std::pair<T, T> p2){
    T dx = p1.first - p2.first;
    T dy = p1.second - p2.second;
    return std::abs(dx) + std::abs(dy);
};

// Squared distance computation
template <class T>
T sq_distance(std::pair<T, T> p1, std::pair<T, T> p2){
    T dx = p1.first - p2.first;
    T dy = p1.second - p2.second;
    return dx*dx + dy*dy;
};

struct pair_hash{
	template <class T1, class T2>
	std::size_t operator() (const std::pair<T1, T2>& p) const{
		return std::hash<T1>()(p.first) ^ std::hash<T2>()(p.second);
	}
};

class Heuristic{
    public:
        virtual double score(const State* s, const Game* g) const = 0;
        virtual ~Heuristic(){};
};

class EmptyHeuristic: public Heuristic{
    public:
        double score(const State *s, const Game *g) const{
            return 0;
        }
};
