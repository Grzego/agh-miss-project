#pragma once
#include <string>

struct SimulationOptions
{
    // jaka musi byc wartosc pola by agent wybral je za cel
    double target_threshold = 0.9; 

    // prawdopodobienstwo ze agent przekaze innemu sciezke i miejsce
    double share_good_path_place = 0.1;
    
    // prawdopodobienstwo ze agent przekaze innemu sciezke
    double share_good_path = 0.2;
    
    // prawdopodobienstwo ze agent przekaze innemu miejsce
    double share_good_place = 0.5;

    // prawdopodobienstwo ze agent przekaze innemu niedokladna lokalizacje miejsca
    double share_good_distributed_place = 0.8;

    // prawdopodobienstwo ze agent przekaze innemu kierunek
    double share_good_direction = 1.0;

    // prawdopodobienstwo ze agent przekaze innemu miejsce
    double share_bad_place = 0.8;

    // prawdopodobienstwo ze agent przekaze innemu niedokladna lokalizacje miejsca
    double share_bad_distributed_place = 1.0;

    // prawdopodobienstwo ze agent wybierze sciezke na ktorej znajduje sie cos niebezpiecznego
    double risky_choices = 0.5;

    // prawdopodobienstwo ze agent przezyje starcie z niebezpiecznym stworzeniem
    double survival_chance = 0.8;

    // wartosc powyzej ktorej pole jest uznawane za dobre
    double good_threshold = 0.9;

    // wartosc ponizej ktorej pole jest uznawane za zle
    double bad_threshold = -0.9;

    // promien w obrebie ktorego agenci dziela sie wiedza
    double share_radius = 45.0;

    // prawdopodobienstwo z jakim agent przekaze informacje innemu
    double share_chance = 0.9;

    // prawdopodobienstwo ze wymiana zostanie powtorzona w krotkim odstepie czasowym
    double repeated_share = 0.00;

    // czas w krokach przez ktory przekazywane sa informacje
    unsigned int learn_time = 15;

    // rozproszenie miejsca (dotyczy przekazywania wiedzy)
    int distribute_radius = 2;

    // ilosc agentow na starcie
    int start_agent_count = 3;

    // czas w krokach po ktorych nastepuje pojawienie sie nowego agenta
    unsigned int agent_spawn_time = 20;
    
    // czas miedzy krokami (dotyczy wyswietlania)
    double step_time = 0.016;

    // czas w krokach jaki agent jest w stanie przetwac bez wody lub jedzenia (po ktorym umiera)
    unsigned int foodless_survival = 100;

    // czas w krokach po ktorych nastepuje dodanie losowego elementu do mapy
    unsigned int terrain_modify_step = 25;

    // okresla ile razy mozna pobrac jedzenie lub wode z danego pola
    unsigned int default_field_value = 10;

    // wspolna wiedza
    bool common_knowledge = false;

    // licznik krokow
    int step_counter = 0;

    // calkowita liczba zebranego jedzenia
    unsigned int total_food = 0;

    // -----
    std::string name;
};