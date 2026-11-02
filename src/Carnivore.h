#ifndef CARNIVORE_H
#define CARNIVORE_H

#include "Animal.h"

// Predator: attacks a herbivore in a neighbouring cell (succeeding with probability
// carnivore_hunt_success_percent) and takes its place;
// after a miss or with no prey nearby it wanders.
class Carnivore : public Animal {
public:
    Carnivore(Position pos, int initialEnergy) : Animal(pos, initialEnergy, OrganismType::CARNIVORE) {}

    char symbol() const override { return 'C'; }
    std::string typeName() const override { return "Carnivore"; }

protected:
    bool feed(World& world) override;
    int moveCost(const World& world) const override;
    int reproduceThreshold(const World& world) const override;
    std::shared_ptr<Animal> makeOffspring(Position pos, int energy) const override;
};

#endif  // CARNIVORE_H
