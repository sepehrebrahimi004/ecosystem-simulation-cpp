#ifndef CONFIG_H
#define CONFIG_H

#include <string>

// Simulation parameters. Every field has a default and can be overridden from a JSON file.
struct Config {
    int grid_width{50};
    int grid_height{50};
    int max_steps{1000};
    int seed{-1};                   // -1: seed from std::random_device
    int log_positions_every{5};     // write grid positions every N steps (0 = never)

    int initial_plants{150};
    int initial_herbivores{60};
    int initial_carnivores{20};

    int plant_energy_gain{2};
    int plant_reproduce_threshold{15};

    int herbivore_move_cost{1};
    int herbivore_energy_from_plant{8};
    int herbivore_reproduce_threshold{20};

    int carnivore_move_cost{2};
    int carnivore_energy_from_herbivore{15};
    int carnivore_reproduce_threshold{30};
    int carnivore_hunt_success_percent{100};  // chance that an attack on adjacent prey succeeds

    int initial_energy_plant{8};
    int initial_energy_herbivore{15};
    int initial_energy_carnivore{25};

    // Reads a JSON file; missing keys keep their defaults. Throws std::runtime_error on
    // a missing file, malformed JSON or invalid values.
    static Config loadFromFile(const std::string& filename);

    // Throws std::runtime_error if a parameter is out of range.
    void validate() const;
};

#endif  // CONFIG_H
