#include "utils.h"

#include <random>
#include <deque>
#include <algorithm>

Vec2::Vec2(int _y, int _x)
    : y(_y), x(_x)
{
}

Vec2::operator sf::Vector2f const &() const
{
    return sf::Vector2f(x, y);
}

bool operator==(Vec2 const &_v1, Vec2 const &_v2)
{
    return _v1.x == _v2.x && _v1.y == _v2.y;
}

bool operator!=(Vec2 const &_v1, Vec2 const &_v2)
{
    return _v1.x != _v2.x || _v1.y != _v2.y;
}

bool operator<(Vec2 const &_v1, Vec2 const &_v2)
{
    return _v1.x < _v2.x && _v1.y < _v2.y;
}

Vec2 hex_position(double _radius, int _y, int _x)
{
    return Vec2(_y * (_radius - 4), (_x + (double)(_y % 2) * 0.5) * _radius);
}

Vec2 hex_position(double _radius, Vec2 const &_v)
{
    return hex_position(_radius, _v.y, _v.x);
}

Vec2 position_hex(double _radius, int _y, int _x)
{
    int yp = std::floor(static_cast<double>(_y) / (_radius - 4.0));
    return Vec2(yp, std::floor(static_cast<double>(_x) / _radius - 0.5 * static_cast<double>(yp % 2)));
}

Vec2 position_hex(double _radius, Vec2 const &_p)
{
    return position_hex(_radius, _p.y, _p.x);
}

int euklid_dist(Vec2 const &_a, Vec2 const &_b)
{
    const double radius = std::ceil(std::sqrt(3) * 25);
    auto p1 = hex_position(radius, _a);
    auto p2 = hex_position(radius, _b);
    double d1 = p1.x - p2.x;
    double d2 = p1.y - p2.y;
    return static_cast<int>(std::sqrt(d1 * d1 + d2 * d2));
}

std::vector<Vec2> hex_places(int _y, int _x)
{
    return std::vector<Vec2>{
        { _y, _x - 1 },
        { _y, _x + 1 },
        { _y - 1, _x - 1 + _y % 2 },
        { _y - 1, _x + _y % 2 },
        { _y + 1, _x - 1 + _y % 2 },
        { _y + 1, _x + _y % 2 }
    };
}

std::vector<Vec2> hex_places(Vec2 const &_p)
{
    return hex_places(_p.y, _p.x);
}

std::vector<std::pair<Vec2, double>> distribute_point(Vec2 const &_point, int _radius, double _val)
{
    std::vector<std::pair<Vec2, double>> result;

    std::deque<std::pair<Vec2, int>> q;
    q.push_back({ _point, 0 });
    while (!q.empty()) {
        auto top = q.front();
        q.pop_front();
        if (top.second >= _radius) continue;
        result.push_back({ top.first, _val / 10.0 /*static_cast<double>(_radius)*/ });
        for (auto &&p : hex_places(top.first)) {
            q.push_back({ p, top.second + 1 });
        }
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    return result;
}

std::vector<Vec2> on_line(Vec2 const &_p1, Vec2 const &_p2)
{
    std::vector<Vec2> result;

    double dx = _p1.x - _p2.x;
    double dy = _p1.y - _p2.y;
    double dist = std::sqrt(dx * dx + dy * dy);
    int dst = static_cast<int>(dist);

    double difx = dx / dist;
    double dify = dy / dist;

    for (int i = 0; i < dst; ++i) {
        result.push_back({ _p1.x + static_cast<int>(difx * static_cast<double>(i)),
                           _p1.y + static_cast<int>(difx * static_cast<double>(i)) });
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    return result;
}

bool are_same(double _a, double _b)
{
    return std::fabs(_a - _b) < 0.0000001;
}

double clamp(double _min, double _max, double _v)
{
    return std::max(std::min(_max, _v), _min);
}

// -----

class Random
{
public:
    Random()
        : generator(rand_dev())
    { }

    std::random_device rand_dev;
    std::mt19937 generator;
};

static Random random{};

Vec2 random_element(std::unordered_set<Vec2> const &_map)
{
    std::uniform_int_distribution<> distr(0, _map.size() - 1);
    return *std::next(_map.begin(), distr(random.generator));
}

int random_int(int _min, int _max) // inclusive
{
    std::uniform_int_distribution<> distr(_min, _max);
    return distr(random.generator);
}

double random_double()
{
    std::uniform_real_distribution<> distr{};
    return distr(random.generator);
}

// -----

std::vector<std::string> split(std::string _str, char _on)
{
    _str.push_back(_on);
    std::vector<std::string> result;
    unsigned int b = 0, e = 0;
    for (e = 0; e < _str.size(); ++e) {
        if (_str[e] == _on) {
            result.push_back(_str.substr(b, e - b));
            b = e + 1;
        }
    }
    return result;
}

std::string trim(std::string _str)
{
    int b, e;
    for (b = 0; b < _str.size() && (_str[b] == ' ' || _str[b] == '\t' || _str[b] == '\n'); ++b);
    for (e = _str.size() - 1; e >= 0 && (_str[e] == ' ' || _str[e] == '\t' || _str[e] == '\n'); --e);
    return _str.substr(b, e - b + 1);
}