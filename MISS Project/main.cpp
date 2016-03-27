#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>

#include <SFML\Graphics.hpp>
#include <SFGUI\SFGUI.hpp>
#include <SFGUI\Widgets.hpp>

#include "simulation\map.h"
#include "simulation\simulation.h"

#include "simulation\utils.h"


// -----

struct InitData
{
    std::string map_file = "sample2.mp";
    std::string option_file = "";
    unsigned int width = 1600, height = 900;
};


// -----

template <typename T>
void add_new_scrollbar(sfg::Box::Ptr _ptr, float _min, float _max, float _step, std::string const &_text, T &_opts);

void parse_options(std::string _file_name, SimulationOptions &_opts);

InitData read_init(std::string const &_file_name);


int main_test(int argc, char *argv[])
{
    if (argc <= 2) {
        std::cout << "Not enough arguments." << std::endl;
        std::cout << "Usage: miss.exe [params_file] [map_file] [num_of_tests] [num_of_steps_per_test]" << std::endl;
        return 0;
    }

    // -----

    SimulationOptions options;
    parse_options(argv[1], options);
    std::string map_file(argv[2]);
    int num_of_tests = from_string<int>(argv[3]);
    int max_steps = from_string<int>(argv[4]);
    // -----

    std::cout << "Name: " << options.name << std::endl << std::endl;
    std::ofstream results(options.name + ".txt");


    std::vector<double> discovered_prec(num_of_tests, 0.0);
    std::vector<double> average_steps(num_of_tests, 0.0);
    std::vector<double> average_lifetime(num_of_tests, 0.0);
    std::vector<unsigned int> food_at_end(num_of_tests, 0);
    std::vector<double> average_agents(max_steps, 0.0);
    std::vector<double> average_food(max_steps, 0.0);
    std::vector<double> average_discovery(max_steps, 0.0);
    unsigned int survived_simulations = 0;

    std::vector<unsigned int> dead(max_steps, 0);

    for (int i = 0; i < num_of_tests; ++i) {
        Map map;
        map.load(map_file);

        double possible_discoveries = map.dimensions().x * map.dimensions().y;

        Simulation sim(map, options);
        SimulationOptions &opts = sim.get_options();

        while (!sim.is_finished() && opts.step_counter < max_steps) {
            sim.step();

            average_agents[opts.step_counter] += static_cast<double>(sim.agents_count());
            average_food[opts.step_counter] += static_cast<double>(opts.total_food);
            average_discovery[opts.step_counter] += static_cast<double>(sim.get_env().get_discovered().size()) / possible_discoveries;
        }

        if (opts.step_counter == max_steps) {
            std::cout << "Population survived." << std::endl;
            ++survived_simulations;
        } else {
            for (unsigned int left = opts.step_counter + 1; left < max_steps; ++left) {
                average_food[left] += average_food[opts.step_counter];
                average_discovery[left] += average_discovery[opts.step_counter];
            }
        }

        discovered_prec[i] = static_cast<double>(sim.get_env().get_discovered().size()) / possible_discoveries;
        average_steps[i] = static_cast<double>(opts.step_counter);
        food_at_end[i] = opts.total_food;

        double avg_lt = 0.0;
        if (sim.get_env().get_lifetimes().size() > 0) {
            for (auto &&a : sim.get_env().get_lifetimes()) {
                avg_lt += static_cast<double>(a);
            }
            avg_lt /= static_cast<double>(sim.get_env().get_lifetimes().size());
        }

        average_lifetime[i] = avg_lt;

        std::cout << "\tDiscovered:    " << discovered_prec[i] << std::endl;
        std::cout << "\tSteps:         " << average_steps[i] << std::endl;
        std::cout << "\tAvg. lifetime: " << avg_lt << std::endl;
        std::cout << "\tGathered food: " << opts.total_food << std::endl;
        std::cout << std::endl;
    }

    std::cout << "Total survived simulations: " << survived_simulations << "/" << num_of_tests << std::endl;

    // -----

    double disc_perc = 0.0;
    for (auto &&d : discovered_prec) {
        disc_perc += d;
    }
    disc_perc /= static_cast<double>(num_of_tests);

    // -----

    double avg_steps = 0.0;
    for (auto &&a : average_steps) {
        avg_steps += a;
    }
    avg_steps /= static_cast<double>(num_of_tests);

    // -----

    double avg_lifetime = 0.0;
    for (auto &&a : average_lifetime) {
        avg_lifetime += a;
    }
    avg_lifetime /= static_cast<double>(num_of_tests);

    // -----

    /*
    1. Œrednia iloœæ kroków symulacji
    2. Œrednio procentowo odkrycie mapy
    3. Œredni czas ¿ycia pojedynczego agenta
    4. Procentowo przetrwane symulacje

    5. Iloœæ zebranego jedzenia (per test)

    6. Œrednia iloœæ agentów na krok symulacji
    7. Œrednia iloœæ zebranego po¿ywienia
    8. Œredni procent odkrycia mapy
    */

    results << avg_steps << std::endl;
    results << disc_perc << std::endl;
    results << avg_lifetime << std::endl;
    results << (static_cast<double>(survived_simulations) / static_cast<double>(num_of_tests)) << std::endl;

    for (auto &&a : food_at_end) {
        results << a << " ";
    } results << std::endl;

    unsigned int idx = 0;
    for (auto &&a : average_agents) {
        unsigned int div = num_of_tests - dead[idx++];
        if (div > 0) {
            results << (a / static_cast<double>(div)) << " ";
        } else {
            results << 0.0 << " ";
        }
    } results << std::endl;

    idx = 0;
    for (auto &&a : average_food) {
        unsigned int div = num_of_tests - dead[idx++];
        if (div > 0) {
            results << (a / static_cast<double>(div)) << " ";
        } else {
            results << 0.0 << " ";
        }
    } results << std::endl;

    idx = 0;
    for (auto &&a : average_discovery) {
        unsigned int div = num_of_tests - dead[idx++];
        if (div > 0) {
            results << (a / static_cast<double>(div)) << " ";
        } else {
            results << 0.0 << " ";
        }
    } results << std::endl;

    results.close();
    return 0;
}


int main()
{
    auto init = read_init("init.conf");

    sfg::SFGUI sfgui;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 4;
    sf::RenderWindow rw(sf::VideoMode(init.width, init.height), "Collective Learning", sf::Style::Default, settings);
    rw.resetGLStates();

    // -- Simulation
    // -----

    sf::Clock sim_timer;

    Map map;
    map.load(init.map_file);

    SimulationOptions start_opts;
    if (!init.option_file.empty()) {
        parse_options(init.option_file, start_opts);
    }

    Simulation sim(map, start_opts);

    SimulationOptions &opts = sim.get_options();


    // -- GUI
    // -----
    int change_map_opt = 0;
    bool run_simulation = false;

    bool drag = false;
    sf::Vector2f last_pos;
    bool allow_gui = false;

    auto gui_wnd = sfg::Window::Create();
    gui_wnd->SetStyle(gui_wnd->GetStyle() ^ sfg::Window::TITLEBAR);
    gui_wnd->SetRequisition(sf::Vector2f(300.0f, 900.0f));

    gui_wnd->GetSignal(sfg::Window::OnMouseEnter).Connect([&]() {
        allow_gui = true;
        drag = false;
    });

    gui_wnd->GetSignal(sfg::Window::OnMouseLeave).Connect([&]() {
        allow_gui = false;
    });

    auto viewport = sfg::Viewport::Create();
    viewport->SetRequisition(sf::Vector2f(300.0f, 900.f));

    auto box = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);

    // ---

    auto button_box = sfg::Box::Create();

    auto next_button = sfg::Button::Create("Next agent");
    next_button->GetSignal(sfg::Button::OnLeftClick).Connect([&]() {
        sim.show_next_agent();
    });
    button_box->Pack(next_button);

    auto no_view_button = sfg::Button::Create("Default view");
    no_view_button->GetSignal(sfg::Button::OnLeftClick).Connect([&]() {
        sim.disable_view();
    });
    button_box->Pack(no_view_button);

    auto play_button = sfg::Button::Create("Play");
    play_button->GetSignal(sfg::Button::OnLeftClick).Connect([&]() {
        run_simulation = !run_simulation;
        play_button->SetLabel(run_simulation ? "Pause" : "Play");
    });
    button_box->Pack(play_button);
    box->Pack(button_box);

    add_new_scrollbar(box, 0.0f, 1.0f, 0.05f, "Target threshold", opts.target_threshold);
    add_new_scrollbar(box, 0.0f, 1.0f, 0.05f, "Share good path and place", opts.share_good_path_place);
    add_new_scrollbar(box, 0.0f, 1.0f, 0.05f, "Share good path", opts.share_good_path);
    add_new_scrollbar(box, 0.0f, 1.0f, 0.05f, "Share good place", opts.share_good_place);
    add_new_scrollbar(box, 0.0f, 1.0f, 0.05f, "Share good distr. place", opts.share_good_distributed_place);
    add_new_scrollbar(box, 0.0f, 1.0f, 0.05f, "Share good direction", opts.share_good_direction);
    add_new_scrollbar(box, 0.0f, 1.0f, 0.05f, "Share bad place", opts.share_bad_place);
    add_new_scrollbar(box, 0.0f, 1.0f, 0.05f, "Share bad distr. place", opts.share_bad_distributed_place);
    add_new_scrollbar(box, 0.0f, 1.0f, 0.05f, "Risky choices", opts.risky_choices);
    add_new_scrollbar(box, 0.0f, 1.0f, 0.05f, "Survival chance", opts.survival_chance);
    add_new_scrollbar(box, -1.f, 1.0f, 0.05f, "Good threshold", opts.good_threshold);
    add_new_scrollbar(box, -1.f, 1.0f, 0.05f, "Bad threshold", opts.bad_threshold);
    add_new_scrollbar(box, 0.0f, 150.0f, 25.f, "Share radius", opts.share_radius);
    add_new_scrollbar(box, 0.0f, 1.0f, 0.05f, "Share chance", opts.share_chance);
    add_new_scrollbar(box, 0.0f, 1.0f, 0.05f, "Repeated share chance", opts.repeated_share);
    add_new_scrollbar(box, 2.0f, 5.0f, 1.0f, "Distribute radius", opts.distribute_radius);
    add_new_scrollbar(box, 1.0f, 50.f, 5.f, "Learn time", opts.learn_time);
    add_new_scrollbar(box, 0.0f, 500.f, 25.f, "Survive without food", opts.foodless_survival);
    add_new_scrollbar(box, 1.0f, 2000.f, 25.f, "Terrain modify time [steps]", opts.terrain_modify_step);
    add_new_scrollbar(box, 1.0f, 100.0f, 5.f, "Default field value", opts.default_field_value);
    add_new_scrollbar(box, 1.0f, 1000.0f, 20.f, "Agent spawn time [steps]", opts.agent_spawn_time);
    add_new_scrollbar(box, 0.0f, 2.0f, 0.025f, "Step time [s]", opts.step_time);
    

    // -----



    
    auto btn_empty = sfg::Button::Create("Empty");
    auto btn_food = sfg::Button::Create("Food");
    auto btn_water = sfg::Button::Create("Water");
    auto btn_block = sfg::Button::Create("Block");
    auto btn_danger = sfg::Button::Create("Danger");

    auto select_correct = [&]() {
        btn_empty->SetLabel(change_map_opt == 1 ? "[Empty]" : "Empty");
        btn_food->SetLabel(change_map_opt == 2 ? "[Food]" : "Food");
        btn_water->SetLabel(change_map_opt == 3 ? "[Water]" : "Water");
        btn_block->SetLabel(change_map_opt == 4 ? "[Block]" : "Block");
        btn_danger->SetLabel(change_map_opt == 5 ? "[Danger]" : "Danger");
    };

    btn_empty->GetSignal(sfg::Button::OnLeftClick).Connect([&]() {
        change_map_opt = change_map_opt == 1 ? 0 : 1;
        select_correct();
    });
    
    btn_food->GetSignal(sfg::Button::OnLeftClick).Connect([&]() {
        change_map_opt = change_map_opt == 2 ? 0 : 2;
        select_correct();
    });
    
    btn_water->GetSignal(sfg::Button::OnLeftClick).Connect([&]() {
        change_map_opt = change_map_opt == 3 ? 0 : 3;
        select_correct();
    });
    
    btn_block->GetSignal(sfg::Button::OnLeftClick).Connect([&]() {
        change_map_opt = change_map_opt == 4 ? 0 : 4;
        select_correct();
    });
    
    btn_danger->GetSignal(sfg::Button::OnLeftClick).Connect([&]() {
        change_map_opt = change_map_opt == 5 ? 0 : 5;
        select_correct();
    });

    auto terr_box = sfg::Box::Create();
    terr_box->Pack(btn_empty);
    terr_box->Pack(btn_food);
    terr_box->Pack(btn_water);
    terr_box->Pack(btn_block);
    terr_box->Pack(btn_danger);

    box->Pack(terr_box);

    gui_wnd->Add(box);

    // -----

    sf::Clock timer;

    while (rw.isOpen()) {
        sf::Event event;
        while (rw.pollEvent(event)) {
            gui_wnd->HandleEvent(event);
            // -----

            if (allow_gui) continue;

            if (event.type == sf::Event::Closed) {
                rw.close();
            } else if (event.type == sf::Event::Resized) {
                auto view = rw.getView();
                view.setSize(sf::Vector2f(event.size.width, event.size.height));
                rw.setView(view);

                gui_wnd->SetRequisition(sf::Vector2f(300.0f, event.size.height));
                //box->SetRequisition(sf::Vector2f(300.0f, event.size.height));
            } else if (change_map_opt > 0) {
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    int yy = event.mouseButton.y + rw.getView().getCenter().y - rw.getView().getSize().y / 2;
                    int xx = event.mouseButton.x + rw.getView().getCenter().x - rw.getView().getSize().x / 2;

                    const double radius = std::ceil(std::sqrt(3) * 25);
                    auto hex = position_hex(radius, yy, xx);

                    if (change_map_opt == 1) {
                        map.change_field(hex, Field::Empty);
                    } else if (change_map_opt == 2) {
                        map.change_field(hex, Field::Food); 
                    } else if (change_map_opt == 3) {
                        map.change_field(hex, Field::Water);
                    } else if (change_map_opt == 4) {
                        map.change_field(hex, Field::Blocked);
                    } else if (change_map_opt == 5) {
                        map.change_field(hex, Field::Danger);
                    }

                }
            } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                last_pos.x = event.mouseButton.x;
                last_pos.y = event.mouseButton.y;
                drag = true;
            } else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                last_pos.x = event.mouseButton.x;
                last_pos.y = event.mouseButton.y;
                drag = false;
            } else if (event.type == sf::Event::MouseMoved && drag) {
                sf::Vector2f pos(event.mouseMove.x, event.mouseMove.y);
                auto view = rw.getView();
                view.move(last_pos - pos);
                rw.setView(view);

                last_pos = pos;
            }

        }

        // -----

        if (run_simulation && sim_timer.getElapsedTime().asSeconds() > sim.get_options().step_time) {
            sim.step();
            sim_timer.restart();
        }

        // -----

        gui_wnd->Update(timer.getElapsedTime().asSeconds());
        timer.restart();

        // -----
        
        rw.clear(sf::Color::White);

        rw.draw(sim);

        sfgui.Display(rw);

        rw.display();
    }
    

    return 0;
}


InitData read_init(std::string const &_file_name)
{
    std::ifstream init(_file_name);
    if (!init) {
        std::cout << "Could not read init file." << std::endl;
        return InitData{};
    }

    InitData result;
    std::string line;
    while (std::getline(init, line)) {
        if (line.empty()) continue;
        auto &&spl = split(line, '=');
        if (spl.size() == 2) {
            auto &&name = trim(spl[0]);
            auto &&val = trim(spl[1]);

            if (name == "map_file") {
                result.map_file = val;
            } else if (name == "option_file") {
                result.option_file = val;
            } else if (name == "width") {
                result.width = from_string<unsigned int>(val);
            } else if (name == "height") {
                result.height = from_string<unsigned int>(val);
            }
        }
    }
    return result;
}


void parse_options(std::string _file_name, SimulationOptions &_opts)
{
    std::ifstream input(_file_name);
    if (!input) {
        std::cout << "Could not read options file." << std::endl;
        return;
    }
    
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty()) continue;
        auto &&spl = split(line, '=');
        if (spl.size() == 1) {
            _opts.name = spl[0].substr(2);
            std::cout << "File: " << _opts.name << std::endl;

        } else {
            auto &&name = trim(spl[0]);
            auto &&val = trim(spl[1]);
            val.pop_back();

            std::cout << name << ": " << val << std::endl;

            if (name == "target_threshold") {
                _opts.target_threshold = from_string<double>(val);
            } else if (name == "share_good_path_place") {
                _opts.share_good_path_place = from_string<double>(val);
            } else if (name == "share_good_path") {
                _opts.share_good_path = from_string<double>(val);
            } else if (name == "share_good_place") {
                _opts.share_good_place = from_string<double>(val);
            } else if (name == "share_good_distributed_place") {
                _opts.share_good_distributed_place = from_string<double>(val);
            } else if (name == "share_good_direction") {
                _opts.share_good_direction = from_string<double>(val);
            } else if (name == "share_bad_place") {
                _opts.share_bad_place = from_string<double>(val);
            } else if (name == "share_bad_distributed_place") {
                _opts.share_bad_distributed_place = from_string<double>(val);
            } else if (name == "risky_choices") {
                _opts.risky_choices = from_string<double>(val);
            } else if (name == "survival_chance") {
                _opts.survival_chance = from_string<double>(val);
            } else if (name == "good_threshold") {
                _opts.good_threshold = from_string<double>(val);
            } else if (name == "bad_threshold") {
                _opts.bad_threshold = from_string<double>(val);
            } else if (name == "share_radius") {
                _opts.share_radius = from_string<double>(val);
            } else if (name == "share_chance") {
                _opts.share_chance = from_string<double>(val);
            } else if (name == "repeated_share") {
                _opts.repeated_share = from_string<double>(val);
            } else if (name == "learn_time") {
                _opts.learn_time = from_string<unsigned int>(val);
            } else if (name == "distribute_radius") {
                _opts.distribute_radius = from_string<int>(val);
            } else if (name == "start_agent_count") {
                _opts.start_agent_count = from_string<int>(val);
            } else if (name == "agent_spawn_time") {
                _opts.agent_spawn_time = from_string<unsigned int>(val);
            } else if (name == "foodless_survival") {
                _opts.foodless_survival = from_string<unsigned int>(val);
            } else if (name == "terrain_modify_step") {
                _opts.terrain_modify_step = from_string<unsigned int>(val);
            } else if (name == "default_field_value") {
                _opts.default_field_value = from_string<unsigned int>(val);
            } else if (name == "common_knowledge") {
                _opts.common_knowledge = from_string<unsigned int>(val);
            }
        }
    }
}

template <typename T>
void add_new_scrollbar(sfg::Box::Ptr _ptr, float _min, float _max, float _step, std::string const &_text, T &_opts)
{
    auto label_name = sfg::Label::Create(_text);
    auto box = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);

    auto tmp_label = sfg::Label::Create(std::to_string(_opts));
    auto tmp_scr = sfg::Scale::Create(_min, _max, _step);
    tmp_scr->SetRequisition(sf::Vector2f(200.0f, 10.0f));
    tmp_scr->GetAdjustment()->SetValue(_opts);
    tmp_scr->GetAdjustment()->GetSignal(sfg::Adjustment::OnChange).Connect([=, &_opts]() {
        auto val = tmp_scr->GetAdjustment()->GetValue();
        _opts = static_cast<T>(val);
        tmp_label->SetText(std::to_string(_opts));
    });

    auto tmp_box = sfg::Box::Create();
    tmp_box->Pack(tmp_label);
    tmp_box->Pack(tmp_scr);

    box->Pack(label_name);
    box->Pack(tmp_box);

    _ptr->Pack(box);
}