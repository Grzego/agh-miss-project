#pragma once

#include "map.h"
#include "environment.h"
#include "utils.h"
#include "simulation_options.h"
#include "knowledge.h"

#include <SFML\Graphics.hpp>

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>

/**
 * Klasa odpowiedzialna za obiekt agenta
 */
class Agent : public sf::Drawable
{
public:
    /**
     * Konstruktor obiektu typu Agent
     * @param pos pozycja startowa
     * @param id id agenta
     * @param opts opcje symulacji
     * @param knowledge wiedza agenta
     */
    Agent(Vec2 const &, unsigned int, SimulationOptions &, std::shared_ptr<Knowledge> _knowledge = nullptr);
    
    /**
     * Konstruktor przenoszacy
     * @param other
     */
    Agent(Agent &&);

    /**
     * Operator przypisania 
     */
    Agent& operator=(Agent &&);

    /**
     * Destruktor obiektu Agent
     */
    virtual ~Agent();

    // -----
    
    /**
     * Metoda ktora pozwala agentowi dokonac decyzji gdzie pojsc dalej
     * @param map mapa
     */
    void make_decision(Map const &);

    /**
     * Metoda zwracajaca podjeta przez agenta decyzje
     * @return dec kolejna pozycja
     */
    Vec2 get_decision() const;

    /**
     * Meotda zwraca aktualna pozycje agenta
     * @return pos pozycja
     */
    Vec2 get_position() const;

    /**
     * Metoda pozwala agentowi zareagowac na otrzymana nagrode
     * @param reward otrzymana nagroda
     */
    void receive_reward(Reward &&);

    /**
     * Metoda powoduje smierc agenta
     */
    void die();

    /**
     * Metoda sprawdzajaca czy dany agent wciaz zyje
     * @return alive
     */
    bool is_alive() const;

    /**
     * Metoda pozwala agentowi podzielic sie wiedza z innym agentem
     * @param agent agent ktoremu przekazywana jest wiedza
     * @param map mapa potrzebna do wyznaczania sciezek
     */
    void share_knowledge(Agent &, Map &);

    /**
     * Metoda zwracajaca informacje czy agent dowiedzial sie czegos nowego
     * @return int wskaznik zmiany wiedzy
     */
    int has_new_knowledge() const;

    /**
     * Metoda zwraca informacje o tym jak dany agent widzi dane pole
     * @param pos pozycja na mapie
     * @return typ pola wg agenta
     */
    int know_of(Vec2 const &) const;

    /**
     * Metoda pozwala ustawic podglad na danym agencie
     * @param onoff
     */
    void set_viewed(bool);

    /**
     * Metoda pozwala ustawic agenta w trybie przekazywania wiedzy
     * @param onoff
     */
    void set_share(bool);

    /**
     * Metoda zwraca id agenta
     * @return id agenta
     */
    unsigned int get_id() const;

    /**
     * Metoda zwieksza wskaznik glodu agenta
     */
    void increase_food_timer();

    /**
     * Metoda resetuje wskaznik glodu agenta
     */
    void reset_food_timer();

    /**
     * Metoda zwraca czy dany agent jest w stanie glodu
     * @return stan glodu
     */
    bool starving() const;

    /**
     * Metoda pozwala agentowi zebrac zywnosc
     */
    void give_food();

    /**
     * Metoda sprawdza czy dany agent posiada zywnosc
     * @return informacja czy agent niesie zywnosc
     */
    bool carrying_food() const;

    /**
     * Metoda powoduje oddanie przez agenta zywnosci
     */
    void take_food();
    
    /**
     * Metoda aktualizuje czas zycia danego agenta
     */
    void next_day();

    /**
     * Metoda pobiera czas zycia danego agenta
     * @return czas zycia agenta
     */
    unsigned int lifetime() const;

    // -----

    /**
     * Metoda pobiera wiedze jaka posiada dany agent
     * @return knowledge wiedza agenta
     */
    Knowledge const& get_knowledge() const;

    // -----
    /**
     * Metoda z biblioteki SFML sluzaca do rysowania na oknie
     */
    virtual void draw(sf::RenderTarget &_target, sf::RenderStates _states) const override; 

protected:
    /**
     * Metoda sluzy do wybrania przez agenta planowanego celu
     * @param map
     */
    void choose_target(Map const &);

    /**
     * Metoda sprawdza czy aktualna sciezka jest wziac wazna
     * @return waznosc sciezki
     */
    bool is_path_valid();

    /**
     * Metoda pozwala na dodanie podanej sciezki do wiedzy agenta
     * @param path sciezka
     */
    void consume_path(std::vector<std::pair<Vec2, unsigned int>> const &);

    /**
     * Metoda pozwala na dodanie miejsca do wiedzy agenta
     * @param place miejsce
     * @param value waga
     * @param time_stamp znacznik czasowy wiedzy
     */
    void consume_place(Vec2 const &, double, unsigned int);
    
private:

    std::shared_ptr<Knowledge> knowledge;

    bool                alive;
    Vec2                target;
    Vec2                position;
    std::vector<Vec2>   path;

    Vec2                mother_land;
    Vec2                decision;
    int                 new_knowledge;
    unsigned int        time_alive;

    bool                has_food;
    bool                is_hungry;
    bool                is_viewed;
    bool                is_sharing;
    unsigned int        food_timer;
    unsigned int        id;

    // -----

    SimulationOptions   &simulation_opts;
};