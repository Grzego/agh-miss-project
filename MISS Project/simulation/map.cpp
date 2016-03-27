#include "map.h"
#include "agent.h"

#include <fstream>
#include <iostream>
#include <functional>
#include <set>


Map::Map()
    : agent_view(nullptr)
    , width(0)
    , height(0)
{
}

Map::~Map() = default;

// -----

void Map::load(std::string const &_file)
{
    std::ifstream file(_file);

    if (!file) {
        std::cout << "ERROR: file not found!" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        fields.push_back(std::vector<Field>());
        for (auto &&c : line) {
            if (c == '.') {
                fields.back().push_back(Field::Empty);
            } else if (c == 'F') {
                fields.back().push_back(Field::Food);
            } else if (c == 'X') {
                fields.back().push_back(Field::Blocked);
            } else if (c == 'W') {
                fields.back().push_back(Field::Water);
            } else if (c == 'P') {
                fields.back().push_back(Field::Population);
                population = Vec2(fields.size() - 1, fields.back().size() - 1);
            } else if (c == 'T') {
                fields.back().push_back(Field::Danger);
            }
        }
    }
    
    width = fields.front().size();
    height = fields.size();
}

std::vector<Vec2> Map::search_path(Vec2 const &_start, 
                                   Vec2 const &_end, 
                                   std::shared_ptr<Knowledge> _knowledge,
                                   int &_total_cost) const
{
    struct heap_data
    {
        heap_data() { }
        heap_data(int _a, Vec2 const &_b, int _c)
            : cost(_a), pos(_b), wave(_c) { }

        bool operator<(heap_data const &_other) const
        {
            return this->cost == _other.cost ? (this->wave == _other.wave ? this < &_other : this->wave < _other.wave) : this->cost < _other.cost;
        }

        int cost = std::numeric_limits<int>::max();
        Vec2 pos;
        int wave = std::numeric_limits<int>::max();
    };

    std::set<heap_data> heap;
    heap.insert(heap_data{ 0, _start, 0 });

    std::unordered_set<Vec2> visited;
    std::unordered_map<Vec2, int> costs;
    std::unordered_map<Vec2, Vec2> parent;

    parent[_start] = _start;
    costs[_start] = 0;

    heap_data pri_goal, sec_goal, tri_goal{ std::numeric_limits<int>::max(), _start, 0 };

    while (!heap.empty()) {
        auto top = *heap.begin();
        heap.erase(heap.begin());

        if (top.pos == _end) {
            pri_goal = top;
            break;
        }
        if (_knowledge->values.find(top.pos) == _knowledge->values.end() && top.cost < sec_goal.cost) {
            sec_goal = top;
        }
        if (tri_goal.wave < top.wave || (tri_goal.wave == top.wave && top.cost < tri_goal.cost)) {
            tri_goal = top;
        }

        for (auto &&place : places(top.pos)) {
            auto &&near = places(place);
            if (_knowledge->blocked.find(place) == _knowledge->blocked.end() &&
                _knowledge->negative.find(place) == _knowledge->negative.end() &&
                (_knowledge->values.find(place) != _knowledge->values.end() ||
                 std::any_of(near.begin(), near.end(),
                             [&](auto &&_p) {
                                 return _knowledge->values.find(_p) != _knowledge->values.end();
                             }
            ))) {
                int cost = (_knowledge->positive.find(place) == _knowledge->positive.end()) * 50 + 
                           (_knowledge->negative.find(place) != _knowledge->negative.end()) * 100000000;
                if (visited.find(place) == visited.end() &&
                    (costs.find(place) == costs.end() ||
                     costs[place] > cost + costs[top.pos])) {
                    int heur_cost = euklid_dist(place, _end);
                    costs[place] = cost + costs[top.pos];
                    heap.insert(heap_data{ costs[place] + heur_cost, place, top.wave + 1 });
                    parent[place] = top.pos;
                }
            }
        }
    }
    
    std::vector<Vec2> path;

    _total_cost = costs.find(pri_goal.pos) != costs.end() ? costs[pri_goal.pos] :
        costs.find(sec_goal.pos) != costs.end() ? costs[sec_goal.pos] :
        costs.find(tri_goal.pos) != costs.end() ? costs[tri_goal.pos] : 0;

    auto goal = costs.find(pri_goal.pos) != costs.end() ? pri_goal.pos :
                costs.find(sec_goal.pos) != costs.end() ? sec_goal.pos : tri_goal.pos;

    while (goal != parent[goal]) {
        path.push_back(goal);
        goal = parent[goal];
    }

    return path;
}


void Map::change_field(Vec2 const &_position, Field _field)
{
    fields[_position.y][_position.x] = _field;
}

Field Map::get_field(Vec2 const &_pos) const
{
    return fields[_pos.y][_pos.x];
}

std::vector<Vec2> Map::places(Vec2 const &_pos) const
{
    auto tmp = std::vector<Vec2>{
        { _pos.y, _pos.x - 1 },
        { _pos.y, _pos.x + 1 },
        { _pos.y - 1, _pos.x - 1 + _pos.y % 2 },
        { _pos.y - 1, _pos.x + _pos.y % 2 },
        { _pos.y + 1, _pos.x - 1 + _pos.y % 2 },
        { _pos.y + 1, _pos.x + _pos.y % 2 }
    };
    
    tmp.erase(std::remove_if(tmp.begin(), tmp.end(), 
                             [=](Vec2 const &_v) {
                                 return _v.x < 0 || _v.x >= width || _v.y < 0 || _v.y >= height;
                             }), tmp.end());
    
    return tmp;
}

Vec2 Map::dimensions() const
{
    return Vec2(height, width);
}

Vec2 Map::start() const
{
    return population;
}

void Map::set_agent_view(Agent const *_agent_view)
{
    agent_view = _agent_view;
}

void Map::draw(sf::RenderTarget &_target, sf::RenderStates _states) const
{
    sf::CircleShape shp(25, 6);
    shp.setOutlineThickness(2.0f);
    shp.setOutlineColor(sf::Color::White);

    const double radius = std::ceil(std::sqrt(3) * 25);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            auto &place = fields[i][j];

            shp.setPosition(hex_position(radius, i, j));
            int r = 255, g = 255, b = 255, a = 255, k = -1;
            if (agent_view) {
                k = agent_view->know_of(Vec2(i, j));
                a = (k > 0) * 155 + 100;
                if (k) {
                    r = 176; g = 226; b = 255;
                }
            }

            if (k == 4) {
                shp.setFillColor(sf::Color(220, 220, 220, 255));
            } if (k == 3) {
                shp.setFillColor(sf::Color(173, 255, 47, 255));
            } else if (k == 2) {
                shp.setFillColor(sf::Color(255, 140, 0, 255));
            } else if (place == Field::Blocked) {
                shp.setFillColor(sf::Color(190, 190, 190, a));
            } else if (place == Field::Empty) {
                shp.setFillColor(sf::Color(r, g, b, a));
            } else if (place == Field::Food) {
                shp.setFillColor(sf::Color(106, 196, 49, a));
            } else if (place == Field::Danger) {
                shp.setFillColor(sf::Color(224, 38, 38, a));
            } else if (place == Field::Water) {
                shp.setFillColor(sf::Color(49, 123, 196, a));
            } else if (place == Field::Population) {
                shp.setFillColor(sf::Color(176, 86, 232, a));
            }

            _target.draw(shp, _states);
        }
    }
}