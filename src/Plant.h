#ifndef PLANT_H
#define PLANT_H

#include "Organism.h"

// Stationary producer: gains energy every step and seeds an empty neighbouring cell
// once its energy reaches the reproduction threshold.
class Plant : public Organism {
public:
    Plant(Position pos, int initialEnergy) : Organism(pos, initialEnergy, OrganismType::PLANT) {}

    void act(World& world) override;
    char symbol() const override { return 'P'; }
    std::string typeName() const override { return "Plant"; }
};

#endif  // PLANT_H
