#ifndef WORLD_H
#define WORLD_H

#include <iosfwd>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Config.h"
#include "Organism.h"
#include "Position.h"

struct PopulationCounts {
    int plants{0};
    int herbivores{0};
    int carnivores{0};
    int total() const { return plants + herbivores + carnivores; }
};

// Owns all organisms, the spatial index and the turn sequence.
// Invariant: each cell holds at most one living plant and at most one living animal.
class World {
public:
    explicit World(const Config& config);

    // Places the initial populations on random distinct cells (one plant and one
    // animal per cell at most).
    void initialize();

    // One time step: every organism alive at the start acts once, in random order.
    // Newborns join the world at the end of the step.
    void step();

    const Config& config() const { return m_config; }
    int currentStep() const { return m_step; }
    PopulationCounts counts() const;
    const std::vector<std::shared_ptr<Organism>>& organisms() const { return m_organisms; }

    // ---- Spatial queries and updates used by the organisms ----
    std::vector<Position> neighbors(Position pos) const;
    std::shared_ptr<Organism> plantAt(Position pos) const;
    std::shared_ptr<Organism> animalAt(Position pos) const;
    bool hasPlant(Position pos) const { return plantAt(pos) != nullptr; }
    bool hasAnimal(Position pos) const { return animalAt(pos) != nullptr; }
    void move(const std::shared_ptr<Organism>& organism, Position to);
    // Registers an offspring. It occupies its cell immediately but acts from the next step.
    void spawn(const std::shared_ptr<Organism>& organism);

    // ---- Output ----
    void writePositions(std::ostream& out) const;  // rows "step,x,y,type"
    void render(std::ostream& out) const;          // ANSI-coloured terminal frame

    // Throws std::logic_error if the occupancy invariant or the index is violated.
    void checkInvariants() const;

private:
    void place(const std::shared_ptr<Organism>& organism);
    void removeDead();

    Config m_config;
    int m_step{0};
    std::vector<std::shared_ptr<Organism>> m_organisms;
    std::vector<std::shared_ptr<Organism>> m_newborns;
    std::unordered_map<Position, std::vector<std::shared_ptr<Organism>>> m_grid;
};

#endif  // WORLD_H
