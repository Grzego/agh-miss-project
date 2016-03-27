#pragma once

#include "map.h"
#include "simulation_options.h"


class Agent;

/**
 * Struktura trzymajaca informacje o nagrodzie jaka otrzymal agent
 */
struct Reward
{
    Vec2 next_position;
    double value;
};

/**
 * Klasa odpowiedzialna za srodowisko w symulacji
 */
class Environment
{
public:
    /**
     * Konstruktor
     * @param map referencja do mapy
     * @param simopts referencja do opcji symulacji
     */
    Environment(Map &, SimulationOptions &);

    /**
     * Metoda wykonujaca krok symulacji na zbiorze agentow
     * @param agents wektor agentow
     */
    void step(std::vector<Agent> &);
    
    /**
     * Metoda zwracajaca set odkrytych przez agentow obszarow
     * @return set odkryte pozycje
     */
    std::unordered_set<Vec2> const & get_discovered() const;

    /**
     * Metoda zwracajaca wektor zawierajacy czasy zycia kazdego z agentow
     * @return vector
     */
    std::vector<unsigned int> const & get_lifetimes() const;

protected:
    /**
     * Metoda wykonujaca zaplanowana przez agenta akcje
     * @param agent
     */
    void do_action(Agent &);

private:
    std::unordered_map<std::pair<int, int>, int> recent_shares;
    std::unordered_map<int, int> share_timers;

    std::unordered_map<Vec2, int> places;
    std::unordered_set<Vec2> discovered;

    std::vector<unsigned int> lifetimers;

    Map                 &map;
    SimulationOptions   &simulation_options;
};