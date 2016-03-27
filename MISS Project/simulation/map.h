#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <SFML\Graphics.hpp>
#include "utils.h"
#include "knowledge.h"


class Agent;

/**
 * Enum opisujacy pola dostepne na mapie.
 */
enum class Field
{
    Empty,
    Food,
    Water,
    Danger,
    Blocked,
    Population,
    // -----
};


/**
 * Klasa odpowiedzialna za przechowywanie informacji o mapie i znajdowanie sciezek
 */
class Map : public sf::Drawable
{
public:
    /**
     * Konstruktor klasy
     */
    Map();
    
    /**
     * Destruktor klasy
     */
    virtual ~Map();
    
    /**
     * Metoda zluzaca do wczytywania mapy z pliku
     * @param _file nazwa pliku z mapa
     */
    void load(std::string const &_file);

    /**
     * Metoda sluzaca do wyznaczania sciezki na mapie
     * @param _start poczatek sciezki
     * @param _end koniec sciezki
     * @param _knowledge wiedza do wyznacznia trasy
     * @param out _total_cost calkowity koszt trasy
     * @return path wyznaczona sciezka
     */
    std::vector<Vec2> search_path(Vec2 const &_start, Vec2 const &_end, std::shared_ptr<Knowledge> _knowledge, int &_total_cost) const;

    /**
     * Metoda pozwala na zmiane typu danego pola
     * @param pos miejsce
     * @param field nowy typ
     */
    void change_field(Vec2 const &, Field);

    /**
     * Metoda pozwala pobrac typ pola w danym miejscu
     * @param place miejsce
     * @return field typ pola
     */
    Field get_field(Vec2 const &) const;

    /**
     * Metoda zwraca otoczenia dla danego miejsca
     * @param place miejsce
     * @return places pobliskie miejsca
     */
    std::vector<Vec2> places(Vec2 const &) const;

    /**
     * Metoda zwraca wymiary mapy
     * @return wymiary
     */
    Vec2 dimensions() const;

    /**
     * Metoda zwraca miejsce startowe populacji
     * @return miejsce startowe populacji
     */
    Vec2 start() const;
    
    // -----
    /**
     * Metoda ustawia podglad na agencie
     * @param wskaznik do agenta
     */
    void set_agent_view(Agent const *);

    /**
     * Metoda z biblioteki SFML sluzaca do rysowania na oknie
     */
    virtual void draw(sf::RenderTarget &, sf::RenderStates) const override;

private:
    std::vector<std::vector<Field>> fields;

    int width;
    int height;

    Vec2 population;

    // -----
    Agent const *agent_view;
};
