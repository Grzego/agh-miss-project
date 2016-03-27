#pragma once

#include <SFML\Graphics.hpp>

#include <unordered_set>
#include <unordered_map>


struct Vec2
{
public:
    Vec2(int _y = 0, int _x = 0);

    operator sf::Vector2f const &() const;

    friend bool operator==(Vec2 const &, Vec2 const &);
    friend bool operator!=(Vec2 const &, Vec2 const &);
    friend bool operator<(Vec2 const &, Vec2 const &);

    int x, y;
};

namespace std
{
    template<>
    struct hash<Vec2>
    {
    public:
        inline size_t operator()(Vec2 const & _v) const
        {
            return hash<int>()(_v.x) * 31 + hash<int>()(_v.y);
        }

    };

    template<>
    struct hash<pair<int, int>>
    {
    public:
        inline size_t operator()(pair<int, int> const &_p) const
        {
            return hash<int>()(_p.first) * 31 + hash<int>()(_p.second);
        }
    };
}

Vec2 hex_position(double _radius, int _y, int _x);

Vec2 hex_position(double _radius, Vec2 const &);

Vec2 position_hex(double _radius, int _y, int _x);

Vec2 position_hex(double _radius, Vec2 const &);

int euklid_dist(Vec2 const &, Vec2 const &);

std::vector<Vec2> hex_places(int _y, int _x);

std::vector<Vec2> hex_places(Vec2 const &);

std::vector<std::pair<Vec2, double>> distribute_point(Vec2 const &, int, double);

std::vector<Vec2> on_line(Vec2 const &, Vec2 const &);

bool are_same(double, double);

double clamp(double, double, double);

// --- randomness

Vec2 random_element(std::unordered_set<Vec2> const &);

int random_int(int, int);

double random_double();

// -----

std::vector<std::string> split(std::string _str, char _on);

std::string trim(std::string _str);

template <typename T>
T from_string(std::string _str)
{
    T result;
    std::stringstream ss;
    ss << _str;
    ss >> result;
    return result;
}