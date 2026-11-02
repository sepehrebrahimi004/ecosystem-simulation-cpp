#ifndef ANIMAL_H
#define ANIMAL_H

#include <memory>

#include "Organism.h"

// Shared behaviour of herbivores and carnivores. A turn is:
//   1. try to feed (species-specific),
//   2. otherwise move to a free neighbouring cell and pay the move cost
//      (the cost is paid even when every neighbour is occupied: it is metabolism),
//   3. die if energy <= 0,
//   4. reproduce into a free neighbouring cell if energy >= threshold.
// At most one animal occupies a cell.
class Animal : public Organism {
public:
    using Organism::Organism;
    void act(World& world) final;

protected:
    // Returns true if the animal ate this turn (and therefore does not move).
    virtual bool feed(World& world) = 0;
    // Preferred destination among the free neighbours; default is a random one.
    virtual Position chooseMove(World& world, const std::vector<Position>& freeCells);
    virtual int moveCost(const World& world) const = 0;
    virtual int reproduceThreshold(const World& world) const = 0;
    virtual std::shared_ptr<Animal> makeOffspring(Position pos, int energy) const = 0;
};

#endif  // ANIMAL_H
