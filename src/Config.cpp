#include "Config.h"

#include <fstream>
#include <stdexcept>

#include "json.hpp"

using json = nlohmann::json;

Config Config::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("cannot open config file '" + filename + "'");
    }

    json j;
    try {
        file >> j;
    } catch (const json::parse_error& e) {
        throw std::runtime_error("invalid JSON in '" + filename + "': " + e.what());
    }

    Config c;
    auto read = [&j](const char* key, int& field) { field = j.value(key, field); };
    read("grid_width", c.grid_width);
    read("grid_height", c.grid_height);
    read("max_steps", c.max_steps);
    read("seed", c.seed);
    read("log_positions_every", c.log_positions_every);
    read("initial_plants", c.initial_plants);
    read("initial_herbivores", c.initial_herbivores);
    read("initial_carnivores", c.initial_carnivores);
    read("plant_energy_gain", c.plant_energy_gain);
    read("plant_reproduce_threshold", c.plant_reproduce_threshold);
    read("herbivore_move_cost", c.herbivore_move_cost);
    read("herbivore_energy_from_plant", c.herbivore_energy_from_plant);
    read("herbivore_reproduce_threshold", c.herbivore_reproduce_threshold);
    read("carnivore_move_cost", c.carnivore_move_cost);
    read("carnivore_energy_from_herbivore", c.carnivore_energy_from_herbivore);
    read("carnivore_reproduce_threshold", c.carnivore_reproduce_threshold);
    read("carnivore_hunt_success_percent", c.carnivore_hunt_success_percent);
    read("initial_energy_plant", c.initial_energy_plant);
    read("initial_energy_herbivore", c.initial_energy_herbivore);
    read("initial_energy_carnivore", c.initial_energy_carnivore);
    c.validate();
    return c;
}

void Config::validate() const {
    auto require = [](bool ok, const char* msg) {
        if (!ok) throw std::runtime_error(std::string("invalid config: ") + msg);
    };
    require(grid_width > 0 && grid_height > 0, "grid dimensions must be positive");
    require(max_steps >= 0, "max_steps must be non-negative");
    require(log_positions_every >= 0, "log_positions_every must be non-negative");
    const long cells = static_cast<long>(grid_width) * grid_height;
    require(initial_plants >= 0 && initial_plants <= cells, "initial_plants must fit on the grid (1 per cell)");
    require(initial_herbivores >= 0 && initial_carnivores >= 0 &&
                initial_herbivores + initial_carnivores <= cells,
            "initial animals must fit on the grid (1 per cell)");
    require(plant_reproduce_threshold > 0 && herbivore_reproduce_threshold > 0 &&
                carnivore_reproduce_threshold > 0,
            "reproduction thresholds must be positive");
    require(carnivore_hunt_success_percent >= 0 && carnivore_hunt_success_percent <= 100,
            "carnivore_hunt_success_percent must be in [0, 100]");
}
