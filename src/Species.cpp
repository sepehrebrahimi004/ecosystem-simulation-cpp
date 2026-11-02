// Behaviour of the three species: Plant, Animal (shared), Herbivore and Carnivore.
#include "Carnivore.h"
#include "Herbivore.h"
#include "Plant.h"
#include "RandomUtils.h"
#include "World.h"

namespace {

std::vector<Position> cellsWithoutPlant(const World& world, Position pos) {
    std::vector<Position> out;
    for (Position p : world.neighbors(pos))
        if (!world.hasPlant(p)) out.push_back(p);
    return out;
}

std::vector<Position> cellsWithoutAnimal(const World& world, Position pos) {
    std::vector<Position> out;
    for (Position p : world.neighbors(pos))
        if (!world.hasAnimal(p)) out.push_back(p);
    return out;
}

}  // namespace

// ---------------------------------------------------------------- Plant
void Plant::act(World& world) {
    if (!isAlive()) return;
    ++m_age;
    m_energy += world.config().plant_energy_gain;
    if (m_energy < world.config().plant_reproduce_threshold) return;

    const auto freeCells = cellsWithoutPlant(world, m_pos);
    if (freeCells.empty()) return;  // crowded: keep the energy for later
    const Position seedPos = RandomUtils::randomElement(freeCells);
    world.spawn(std::make_shared<Plant>(seedPos, splitEnergy()));
}

// ---------------------------------------------------------------- Animal
Position Animal::chooseMove(World&, const std::vector<Position>& freeCells) {
    return RandomUtils::randomElement(freeCells);
}

void Animal::act(World& world) {
    if (!isAlive()) return;
    ++m_age;

    if (!feed(world)) {
        const auto freeCells = cellsWithoutAnimal(world, m_pos);
        if (!freeCells.empty()) world.move(shared_from_this(), chooseMove(world, freeCells));
        m_energy -= moveCost(world);
    }

    if (m_energy <= 0) {
        kill();
        return;
    }

    if (m_energy >= reproduceThreshold(world)) {
        const auto freeCells = cellsWithoutAnimal(world, m_pos);
        if (!freeCells.empty()) {
            const Position childPos = RandomUtils::randomElement(freeCells);
            world.spawn(makeOffspring(childPos, splitEnergy()));
        }
    }
}

// ---------------------------------------------------------------- Herbivore
bool Herbivore::feed(World& world) {
    auto plant = world.plantAt(m_pos);
    if (!plant) return false;
    plant->kill();
    m_energy += world.config().herbivore_energy_from_plant;
    return true;
}

Position Herbivore::chooseMove(World& world, const std::vector<Position>& freeCells) {
    std::vector<Position> withFood;
    for (Position p : freeCells)
        if (world.hasPlant(p)) withFood.push_back(p);
    return RandomUtils::randomElement(withFood.empty() ? freeCells : withFood);
}

int Herbivore::moveCost(const World& world) const { return world.config().herbivore_move_cost; }
int Herbivore::reproduceThreshold(const World& world) const { return world.config().herbivore_reproduce_threshold; }
std::shared_ptr<Animal> Herbivore::makeOffspring(Position pos, int energy) const {
    return std::make_shared<Herbivore>(pos, energy);
}

// ---------------------------------------------------------------- Carnivore
bool Carnivore::feed(World& world) {
    std::vector<std::shared_ptr<Organism>> prey;
    for (Position p : world.neighbors(m_pos)) {
        auto a = world.animalAt(p);
        if (a && a->type() == OrganismType::HERBIVORE) prey.push_back(a);
    }
    if (prey.empty()) return false;
    if (RandomUtils::randomInt(1, 100) > world.config().carnivore_hunt_success_percent) return false;  // prey escapes

    auto victim = RandomUtils::randomElement(prey);
    victim->kill();
    world.move(shared_from_this(), victim->position());  // take the prey's cell
    m_energy += world.config().carnivore_energy_from_herbivore;
    return true;
}

int Carnivore::moveCost(const World& world) const { return world.config().carnivore_move_cost; }
int Carnivore::reproduceThreshold(const World& world) const { return world.config().carnivore_reproduce_threshold; }
std::shared_ptr<Animal> Carnivore::makeOffspring(Position pos, int energy) const {
    return std::make_shared<Carnivore>(pos, energy);
}
