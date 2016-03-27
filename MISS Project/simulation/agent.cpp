#include <iterator>

#include "agent.h"
#include "utils.h"

Agent::Agent(Vec2 const &_start_pos, unsigned int _id, SimulationOptions &_opts, std::shared_ptr<Knowledge> _knowledge)
    : position(_start_pos)
    , simulation_opts(_opts)
    , mother_land(_start_pos)
    , target(_start_pos)
    , new_knowledge(0)
    , time_alive(0)
    , has_food(false)
    , is_hungry(false)
    , is_viewed(false)
    , is_sharing(false)
    , alive(true)
    , id(_id)
    , food_timer(0)
{
    knowledge = _knowledge;
    if (!knowledge) {
        knowledge = std::make_shared<Knowledge>();
    }

    knowledge->values[position] = 0.0;
}

Agent::Agent(Agent &&) = default;

Agent& Agent::operator=(Agent &&) = default;

Agent::~Agent() = default;

// -----

void Agent::make_decision(Map const &_map)
{
    if (target == _map.start() && target == position) {
        choose_target(_map);
    } else if (target == position) {
        target = _map.start();
    }

    if (!is_path_valid()) {
        int total_cost = -1;
        path = _map.search_path(position, target, knowledge, total_cost);
        if (total_cost > 1000000 && random_double() < simulation_opts.risky_choices) {
            path.clear();
        }
    }

    if (is_path_valid()) {
        decision = path.back();
        path.pop_back();
    } else {
        decision = position;
    }
}

Vec2 Agent::get_decision() const
{
    return decision;
}

Vec2 Agent::get_position() const
{
    return position;
}

bool Agent::is_alive() const
{
    return alive;
}

void Agent::next_day()
{
    ++time_alive;
}

unsigned int Agent::lifetime() const
{
    return time_alive;
}

void Agent::receive_reward(Reward &&_reward)
{
    if (_reward.value > simulation_opts.good_threshold) {
        if (knowledge->positive.find(_reward.next_position) == knowledge->positive.end()) {
            knowledge->positive.insert(_reward.next_position);
            ++new_knowledge;
        }
        has_food = true;
        target = mother_land;
        path.clear();
    } else if (_reward.value < simulation_opts.bad_threshold) {
        if (knowledge->negative.find(_reward.value) == knowledge->negative.end()) {
            knowledge->negative.insert(_reward.next_position);
            ++new_knowledge;
        }
        target = mother_land;
        path.clear();
    }

    if (!are_same(_reward.value, knowledge->values[decision])) {
        if (knowledge->values[decision] > simulation_opts.good_threshold && _reward.value < simulation_opts.good_threshold) {
            knowledge->positive.erase(decision);
        }
        if (knowledge->values[decision] < simulation_opts.bad_threshold && _reward.value > simulation_opts.bad_threshold) {
            knowledge->negative.erase(decision);
        }
        ++new_knowledge;
        path.clear();
    }

    if (decision != _reward.next_position) {
        knowledge->values.erase(decision);
        knowledge->positive.erase(decision);
        knowledge->negative.erase(decision);
        path.clear();
            
        knowledge->blocked.insert(decision);
    }

    knowledge->values[_reward.next_position] = clamp(-1.0, 1.0, knowledge->values[_reward.next_position] + _reward.value);
    knowledge->time_stamp[_reward.next_position] = simulation_opts.step_counter;
    position = _reward.next_position;
}

void Agent::die()
{
    alive = false;
}


int Agent::has_new_knowledge() const
{
    return new_knowledge;
}


int Agent::know_of(Vec2 const &_p) const
{
    if (knowledge->blocked.find(_p) != knowledge->blocked.end()) return 4;
    if (knowledge->positive.find(_p) != knowledge->positive.end()) return 3;
    if (knowledge->negative.find(_p) != knowledge->negative.end()) return 2;
    if (knowledge->values.find(_p) != knowledge->values.end()) return 1;
    return 0;
}

void Agent::set_viewed(bool _is_viewed)
{
    is_viewed = _is_viewed;
}

void Agent::set_share(bool _is_sharing)
{
    is_sharing = _is_sharing;
}

unsigned int Agent::get_id() const
{
    return id;
}

void Agent::increase_food_timer()
{
    ++food_timer;
    is_hungry = false;
    if (food_timer > simulation_opts.foodless_survival) {
        die();
    } else if (food_timer > simulation_opts.foodless_survival / 2) {
        is_hungry = true;
    }
}

void Agent::reset_food_timer()
{
    food_timer = 0;
}

bool Agent::starving() const
{
    return is_hungry;
}

void Agent::give_food()
{
    has_food = true;
}

bool Agent::carrying_food() const
{
    return has_food;
}

void Agent::take_food()
{
    has_food = false;
}

Knowledge const& Agent::get_knowledge() const
{
    return *knowledge;
}


void Agent::share_knowledge(Agent &_other, Map &_map)
{
    std::vector<Vec2> share_positive;
    std::vector<Vec2> share_negative;

    for (auto &&p : knowledge->positive) {
        if (knowledge->values[p] > simulation_opts.good_threshold &&
            _other.know_of(p) != know_of(p) &&
            _other.knowledge->time_stamp[p] < knowledge->time_stamp[p] &&
            random_double() < simulation_opts.share_chance) {
            share_positive.push_back(p);
        }
    }

    for (auto &&p : knowledge->negative) {
        if (knowledge->values[p] < simulation_opts.bad_threshold &&
            _other.know_of(p) != know_of(p) &&
            _other.knowledge->time_stamp[p] < knowledge->time_stamp[p] &&
            random_double() < simulation_opts.share_chance) {
            share_negative.push_back(p);
        }
    }

    // -----

    double share_method = random_double();

    // -----

    if (share_method < simulation_opts.share_good_path_place) {
        for (auto &&p : share_positive) {
            int tc;
            std::vector<std::pair<Vec2, unsigned int>> pth;
            auto search = _map.search_path(position, p, knowledge, tc);
            std::transform(search.begin(), search.end(), std::back_inserter(pth), [=](auto &&_a) {
                return std::make_pair(_a, knowledge->time_stamp[_a]);
            });
            _other.consume_path(pth);
            _other.consume_place(p, knowledge->values[p], knowledge->time_stamp[p]);
        }

    } else if (share_method < simulation_opts.share_good_path) {
        for (auto &&p : share_positive) {
            int tc;
            std::vector<std::pair<Vec2, unsigned int>> pth;
            auto search = _map.search_path(position, p, knowledge, tc);
            std::transform(search.begin(), search.end(), std::back_inserter(pth), [=](auto &&_a) {
                return std::make_pair(_a, knowledge->time_stamp[_a]);
            });
            _other.consume_path(pth);
        }

    } else if (share_method < simulation_opts.share_good_place) {
        for (auto &&p : share_positive) {
            _other.consume_place(p, knowledge->values[p], knowledge->time_stamp[p]);
        }

    } else if (share_method < simulation_opts.share_good_distributed_place) {
        for (auto &&p : share_positive) {
            auto dis_point = distribute_point(p, random_int(1, simulation_opts.distribute_radius), knowledge->values[p]);
            for (auto &&d : dis_point) {
                _other.consume_place(d.first, d.second, knowledge->time_stamp[p]);
            }
        }

    } else if (share_method < simulation_opts.share_good_direction) {
        for (auto &&p : share_positive) {
            std::vector<std::pair<Vec2, unsigned int>> pth;
            auto line = on_line(position, p);
            std::transform(line.begin(), line.end(), std::back_inserter(pth), [=](auto &&_a) {
                return std::make_pair(_a, knowledge->time_stamp[p]);
            });
            _other.consume_path(pth);
        }
    }

    // -----

    share_method = random_double();

    // -----

    if (share_method < simulation_opts.share_bad_place) {
        for (auto &&p : share_negative) {
            _other.consume_place(p, knowledge->values[p], knowledge->time_stamp[p]);
        }

    } else if (share_method < simulation_opts.share_bad_distributed_place) {
        for (auto &&p : share_negative) {
            auto dis_point = distribute_point(p, random_int(1, simulation_opts.distribute_radius), knowledge->values[p]);
            for (auto &&d : dis_point) {
                _other.consume_place(d.first, d.second, knowledge->time_stamp[p]);
            }
        }

    }
}

void Agent::consume_path(std::vector<std::pair<Vec2, unsigned int>> const &_path)
{
    for (auto &&p : _path) {
        volatile auto v = knowledge->values[p.first];
        knowledge->time_stamp[p.first] = p.second;
    }
}

void Agent::consume_place(Vec2 const &_place, double _val, unsigned int _time_stamp)
{
    auto &val = knowledge->values[_place];
    val += _val;
    val = clamp(-1.0, 1.0, val);

    if (val > simulation_opts.good_threshold) {
        knowledge->positive.insert(_place);
        knowledge->negative.erase(_place);
        knowledge->time_stamp[_place] = _time_stamp;
    } else if (val < simulation_opts.bad_threshold) {
        knowledge->negative.insert(_place);
        knowledge->positive.erase(_place);
        knowledge->time_stamp[_place] = _time_stamp;
    }
}

void Agent::draw(sf::RenderTarget &_target, sf::RenderStates _states) const
{
    const double radius = std::ceil(std::sqrt(3) * 25);
    auto pos = hex_position(radius, position);
    pos.x += 15.0f; pos.y += 15.0f;

    sf::CircleShape shp(10);
    shp.setFillColor(is_sharing ? sf::Color(255, 215, 0) : (is_viewed ? sf::Color(176, 86, 232) : sf::Color(149, 238, 255)));
    shp.setPosition(pos);

    _target.draw(shp, _states);
}

void Agent::choose_target(Map const &_map)
{
    std::vector<Vec2> choices;
    for (auto &&p : knowledge->positive) {
        if (knowledge->values[p] > simulation_opts.target_threshold) {
            choices.push_back(p);
        }
    }
    
    if (choices.empty()) {
        Vec2 dim = _map.dimensions();
        target.x = random_int(0, dim.x - 1);
        target.y = random_int(0, dim.y - 1);
    } else {
        target = choices[random_int(0, choices.size() - 1)];
    }
}

bool Agent::is_path_valid()
{
    return !path.empty();
}