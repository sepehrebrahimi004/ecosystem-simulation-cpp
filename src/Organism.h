#ifndef ORGANISM_H
#define ORGANISM_H

#include <memory>
#include <string>

#include "Position.h"

class World;

enum class OrganismType { PLANT, HERBIVORE, CARNIVORE };

// Abstract base for every living entity on the grid.
class Organism : public std::enable_shared_from_this<Organism> {
public:
    Organism(Position pos, int initialEnergy, OrganismType type)
        : m_id(s_nextId++), m_pos(pos), m_energy(initialEnergy), m_type(type) {}
    virtual ~Organism() = default;

    Organism(const Organism&) = delete;
    Organism& operator=(const Organism&) = delete;

    // One turn: feed, move, pay energy costs and possibly reproduce.
    virtual void act(World& world) = 0;
    virtual char symbol() const = 0;
    virtual std::string typeName() const = 0;

    int id() const { return m_id; }
    Position position() const { return m_pos; }
    void setPosition(Position pos) { m_pos = pos; }
    int energy() const { return m_energy; }
    int age() const { return m_age; }
    bool isAlive() const { return m_alive; }
    bool isAnimal() const { return m_type != OrganismType::PLANT; }
    OrganismType type() const { return m_type; }
    void kill() { m_alive = false; }

protected:
    // Gives half of the energy (rounded down) to an offspring and returns that amount.
    int splitEnergy() {
        const int half = m_energy / 2;
        m_energy -= half;
        return half;
    }

    int m_id;
    Position m_pos;
    int m_energy;
    int m_age{0};
    bool m_alive{true};
    OrganismType m_type;

private:
    static inline int s_nextId = 1;
};

#endif  // ORGANISM_H
