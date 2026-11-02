#ifndef HERBIVORE_H
#define HERBIVORE_H

#include "Animal.h"

// Primary consumer: eats the plant in its own cell, otherwise moves, preferring
// neighbouring cells that contain a plant.
class Herbivore : public Animal {
public:
    Herbivore(Position pos, int initialEnergy) : Animal(pos, initialEnergy, OrganismType::HERBIVORE) {}

    char symbol() const override { return 'H'; }
    std::string typeName() const override { return "Herbivore"; }

protected:
    bool feed(World& world) override;
    Position chooseMove(World& world, const std::vector<Position>& freeCells) override;
    int moveCost(const World& world) const override;
    int reproduceThreshold(const World& world) const override;
    std::shared_ptr<Animal> makeOffspring(Position pos, int energy) const override;
};

#endif  // HERBIVORE_H
