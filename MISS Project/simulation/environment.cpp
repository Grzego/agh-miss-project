#include "environment.h"
#include "agent.h"

#include <iostream>

Environment::Environment(Map &_map, SimulationOptions &_opts)
    : map(_map)
    , simulation_options(_opts)
{
}


void Environment::step(std::vector<Agent> &_agents)
{
    for (auto it = share_timers.begin(); it != share_timers.end(); ) {
        if (--it->second == 0) {
            it = share_timers.erase(it);
        } else {
            ++it;
        }
    }

    for (auto &&a : _agents) {
        if (share_timers.find(a.get_id()) == share_timers.end()) {
            a.set_share(false);
            a.make_decision(map);
            do_action(a);
        }
        a.increase_food_timer();
        a.next_day();
        if (!a.is_alive()) {
            lifetimers.push_back(a.lifetime());
        }
    }

    
    if (!simulation_options.common_knowledge) {
        for (int i = 0; i < _agents.size(); ++i) {
            for (int j = 0; j < _agents.size(); ++j) {
                unsigned int a1 = _agents[i].get_id();
                unsigned int a2 = _agents[j].get_id();
                if (i == j || !_agents[i].is_alive() || !_agents[j].is_alive() || share_timers.find(a1) != share_timers.end() || share_timers.find(a2) != share_timers.end() ||
                    (recent_shares[{a1, a2}] == _agents[i].has_new_knowledge() && random_double() > simulation_options.repeated_share) || _agents[i].starving() || _agents[j].starving()) {
                    continue;
                }

                if (euklid_dist(_agents[i].get_position(), _agents[j].get_position()) < simulation_options.share_radius) {
                    _agents[i].share_knowledge(_agents[j], map);
                    recent_shares[{a1, a2}] = _agents[i].has_new_knowledge();
                    share_timers[a1] = simulation_options.learn_time;
                    share_timers[a2] = simulation_options.learn_time;
                    _agents[i].set_share(true);
                    _agents[j].set_share(true);
                }
            }
        }
    }

    if (simulation_options.step_counter % simulation_options.terrain_modify_step == 0) {
        int rx = random_int(0, map.dimensions().x - 1);
        int ry = random_int(0, map.dimensions().y - 1);

        auto field = map.get_field(Vec2(ry, rx));
        if (field != Field::Population && field != Field::Blocked) {
            int rf = random_int(1, 3);
            map.change_field(Vec2(ry, rx), static_cast<Field>(rf));
        }
    }
}

std::unordered_set<Vec2> const & Environment::get_discovered() const
{
    return discovered;
}

std::vector<unsigned int> const & Environment::get_lifetimes() const
{
    return lifetimers;
}

// -----

int& get_with_def(std::unordered_map<Vec2, int> &m, Vec2 const &key, int val)
{
    if (m.find(key) == m.end())
        return (m[key] = val);
    return m[key];
}

// -----

void Environment::do_action(Agent &_agent)
{
    auto dec = _agent.get_decision();
    auto field = map.get_field(dec);

    discovered.insert(dec);

    if (field == Field::Water || field == Field::Food) {
        if (--get_with_def(places, dec, simulation_options.default_field_value) < 0) {
            map.change_field(dec, Field::Empty);
            places.erase(dec);
        }
    }

    if (field == Field::Blocked) {
        _agent.receive_reward(Reward{ _agent.get_position(), 0.0 });
    } else if (field == Field::Water || field == Field::Food) {
        _agent.receive_reward(Reward{ dec, 1.0 });
        _agent.reset_food_timer();
        _agent.give_food();
    } else if (field == Field::Danger) {
        bool is_alive = random_double() < simulation_options.survival_chance;
        if (!is_alive) {
            _agent.die();
        } else {
            _agent.receive_reward(Reward{ dec, -1.0 });
            if (--get_with_def(places, dec, simulation_options.default_field_value) < 0) {
                map.change_field(dec, Field::Empty);
                places.erase(dec);
            }
        }
    } else if (field == Field::Population) {
        if (_agent.carrying_food()) {
            _agent.take_food();
            simulation_options.total_food += 1;
            _agent.reset_food_timer();
        } else if (simulation_options.total_food > 0) {
            _agent.reset_food_timer();
            simulation_options.total_food -= 1;
        }
        _agent.receive_reward(Reward{ dec, 0.0 });
    } else {
        _agent.receive_reward(Reward{ dec, 0.0 });
    }
}