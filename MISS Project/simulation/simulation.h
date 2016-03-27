#pragma once

#include "environment.h"
#include "agent.h"
#include "simulation_options.h"

#include <SFML\Graphics.hpp>

#include <vector>

/**
 * Klasa odpowiedzialna za symulacje
 */
class Simulation : public sf::Drawable
{
public:
    /**
     * Konstruktor dla typu Simulation
     * @param _map referencja do obiektu typu Map
     * @param _sim_opts obiekt zawierajacy opcje symulacji
     */
    Simulation(Map &_map, SimulationOptions _sim_opts = SimulationOptions());
    
    /**
     * Destruktor dla typu Simulation
     */
    virtual ~Simulation();


    /**
     * Metoda wynokujaca jeden krok symulacji
     */
    void step();

    /**
     * Metoda zwraca opcje symulacji
     * @return opcje symulacji
     */
    SimulationOptions& get_options();

    // -----

    /**
     * Metoda zwraca informacje czy symulacja sie zakonczyla
     * @return informacje czy symulacja sie zakonczyla
     */
    bool is_finished() const;

    /**
     * Metoda zwraca ilosc agentow w danym kroku symulacji
     * @return ilosc agentow
     */
    unsigned int agents_count() const;

    /**
     * Metoda zwraca wektor zawierajacy agentow w danym kroku symulacji
     * @return wektor agentow
     */
    std::vector<Agent> const & get_agents() const;

    /**
     * Metoda zwraca srodowisko 
     * @return srodowisko w danej symulacji
     */
    Environment const & get_env() const;

    // -----

    /**
     * Metoda przelacza widok na nastepnego agenta
     */
    void show_next_agent();

    /**
     * Metoda wylacza poglad agenta
     */
    void disable_view();

    /**
     * Metoda z biblioteki SFML sluzaca do rysowania na oknie
     */
    virtual void draw(sf::RenderTarget &_target, sf::RenderStates _states) const override;

private:
    Map                 &map;
    std::vector<Agent>  agents;
    Environment         environment;
    SimulationOptions   simulation_opts;

    std::shared_ptr<Knowledge> common_knowledge;

    unsigned int    agent_unique_id;
    int             viewed_agent;
    bool            is_done;
};