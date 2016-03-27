#include "simulation.h"

#include <iostream>

Simulation::Simulation(Map &_map, SimulationOptions _sim_opts)
    : simulation_opts(_sim_opts)
    , map(_map)
    , environment(_map, simulation_opts)
    , agent_unique_id(0)
    , viewed_agent(-1)
    , is_done(false)
{
    if (_sim_opts.common_knowledge) {
        common_knowledge = std::make_shared<Knowledge>();
    }

    for (int i = 0; i < simulation_opts.start_agent_count; ++i) {
        agents.emplace_back(map.start(), agent_unique_id++, simulation_opts, common_knowledge);
    }

}

Simulation::~Simulation() = default;

// -----

void Simulation::step()
{
    if (is_done) {
        return;
    }
    ++simulation_opts.step_counter;
    if (agents.size() > 0) {
        if (simulation_opts.step_counter % simulation_opts.agent_spawn_time == 0) {
            agents.emplace_back(map.start(), agent_unique_id++, simulation_opts, common_knowledge);
        }

        environment.step(agents);

        if (viewed_agent != -1) {
            if (!agents[viewed_agent].is_alive()) {
                disable_view();
            } else {
                int deads = 0;
                for (int i = 0; i < viewed_agent; ++i) {
                    if (!agents[i].is_alive()) ++deads;
                }
                viewed_agent -= deads;
            }
        }

        agents.erase(std::remove_if(agents.begin(), agents.end(), [](auto &&a) { return !a.is_alive(); }), agents.end());
    } else {
        is_done = true;
        std::cout << "Population died." << std::endl;
    }
}

SimulationOptions& Simulation::get_options()
{
    return simulation_opts;
}

Environment const & Simulation::get_env() const
{
    return environment;
}

void Simulation::draw(sf::RenderTarget &_target, sf::RenderStates _states) const
{
    map.set_agent_view(viewed_agent != -1 ? &agents.at(viewed_agent) : nullptr);

    _target.draw(map, _states);
    for (auto &&a : agents) {
        _target.draw(a, _states);
    }
}

bool Simulation::is_finished() const
{
    return is_done;
}

unsigned int Simulation::agents_count() const
{
    return agents.size();
}

std::vector<Agent> const & Simulation::get_agents() const
{
    return agents;
}

void Simulation::show_next_agent()
{
    if (agents.size() == 0) {
        return;
    }

    if (viewed_agent != -1) {
        agents[viewed_agent].set_viewed(false);
    }
    viewed_agent += 1;
    viewed_agent %= agents.size();

    agents[viewed_agent].set_viewed(true);
}


void Simulation::disable_view()
{
    if (viewed_agent != -1) {
        agents[viewed_agent].set_viewed(false);
        viewed_agent = -1;
    }
}