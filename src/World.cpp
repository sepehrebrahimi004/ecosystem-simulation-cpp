#include "World.h"

#include <algorithm>
#include <ostream>
#include <sstream>
#include <stdexcept>

#include "Carnivore.h"
#include "Herbivore.h"
#include "Plant.h"
#include "RandomUtils.h"

World::World(const Config& config) : m_config(config) { m_config.validate(); }

void World::initialize() {
    m_organisms.clear();
    m_newborns.clear();
    m_grid.clear();
    m_step = 0;

    std::vector<Position> cells;
    cells.reserve(static_cast<std::size_t>(m_config.grid_width) * m_config.grid_height);
    for (int x = 0; x < m_config.grid_width; ++x)
        for (int y = 0; y < m_config.grid_height; ++y) cells.push_back({x, y});

    RandomUtils::shuffle(cells);
    for (int i = 0; i < m_config.initial_plants; ++i)
        place(std::make_shared<Plant>(cells[i], m_config.initial_energy_plant));

    RandomUtils::shuffle(cells);  // animals are placed independently of plants
    int next = 0;
    for (int i = 0; i < m_config.initial_herbivores; ++i)
        place(std::make_shared<Herbivore>(cells[next++], m_config.initial_energy_herbivore));
    for (int i = 0; i < m_config.initial_carnivores; ++i)
        place(std::make_shared<Carnivore>(cells[next++], m_config.initial_energy_carnivore));
}

void World::step() {
    RandomUtils::shuffle(m_organisms);  // random turn order removes order bias
    for (std::size_t i = 0; i < m_organisms.size(); ++i) {
        if (m_organisms[i]->isAlive()) m_organisms[i]->act(*this);
    }
    for (auto& child : m_newborns) m_organisms.push_back(child);  // already on the grid
    m_newborns.clear();
    removeDead();
    ++m_step;
}

PopulationCounts World::counts() const {
    PopulationCounts c;
    for (const auto& o : m_organisms) {
        if (!o->isAlive()) continue;
        switch (o->type()) {
            case OrganismType::PLANT: ++c.plants; break;
            case OrganismType::HERBIVORE: ++c.herbivores; break;
            case OrganismType::CARNIVORE: ++c.carnivores; break;
        }
    }
    return c;
}

std::vector<Position> World::neighbors(Position pos) const {
    return pos.neighbors(m_config.grid_width, m_config.grid_height);
}

std::shared_ptr<Organism> World::plantAt(Position pos) const {
    auto it = m_grid.find(pos);
    if (it == m_grid.end()) return nullptr;
    for (const auto& o : it->second)
        if (o->isAlive() && !o->isAnimal()) return o;
    return nullptr;
}

std::shared_ptr<Organism> World::animalAt(Position pos) const {
    auto it = m_grid.find(pos);
    if (it == m_grid.end()) return nullptr;
    for (const auto& o : it->second)
        if (o->isAlive() && o->isAnimal()) return o;
    return nullptr;
}

void World::move(const std::shared_ptr<Organism>& organism, Position to) {
    auto& from = m_grid[organism->position()];
    from.erase(std::remove(from.begin(), from.end(), organism), from.end());
    m_grid[to].push_back(organism);
    organism->setPosition(to);
}

void World::spawn(const std::shared_ptr<Organism>& organism) {
    m_grid[organism->position()].push_back(organism);
    m_newborns.push_back(organism);
}

void World::place(const std::shared_ptr<Organism>& organism) {
    m_grid[organism->position()].push_back(organism);
    m_organisms.push_back(organism);
}

void World::removeDead() {
    auto dead = [](const std::shared_ptr<Organism>& o) { return !o->isAlive(); };
    for (auto it = m_grid.begin(); it != m_grid.end();) {
        auto& cell = it->second;
        cell.erase(std::remove_if(cell.begin(), cell.end(), dead), cell.end());
        it = cell.empty() ? m_grid.erase(it) : std::next(it);
    }
    m_organisms.erase(std::remove_if(m_organisms.begin(), m_organisms.end(), dead), m_organisms.end());
}

void World::writePositions(std::ostream& out) const {
    for (const auto& o : m_organisms) {
        if (!o->isAlive()) continue;
        const Position p = o->position();
        out << m_step << ',' << p.x << ',' << p.y << ',' << o->symbol() << '\n';
    }
}

void World::render(std::ostream& out) const {
    static const char* GREEN = "\033[32m";
    static const char* CYAN = "\033[36m";
    static const char* RED = "\033[31m";
    static const char* GRAY = "\033[90m";
    static const char* RESET = "\033[0m";

    // An animal is drawn on top of a plant in the same cell.
    std::vector<std::string> rows(m_config.grid_height, std::string(m_config.grid_width, '.'));
    for (const auto& o : m_organisms) {
        if (!o->isAlive()) continue;
        char& c = rows[o->position().y][o->position().x];
        if (o->isAnimal() || c == '.') c = o->symbol();
    }

    const PopulationCounts n = counts();
    std::ostringstream buf;
    buf << "\033[2J\033[H"
        << "Step " << m_step << " / " << m_config.max_steps << "   "
        << GREEN << "plants " << n.plants << RESET << "   "
        << CYAN << "herbivores " << n.herbivores << RESET << "   "
        << RED << "carnivores " << n.carnivores << RESET << '\n';
    for (const auto& row : rows) {
        for (char c : row) {
            const char* color = c == 'P' ? GREEN : c == 'H' ? CYAN : c == 'C' ? RED : GRAY;
            buf << color << c << ' ' << RESET;
        }
        buf << '\n';
    }
    out << buf.str() << std::flush;  // one write per frame avoids flicker
}

void World::checkInvariants() const {
    for (const auto& [pos, cell] : m_grid) {
        int plants = 0, animals = 0;
        for (const auto& o : cell) {
            if (o->position() != pos) throw std::logic_error("grid index out of sync with position");
            if (!o->isAlive()) continue;
            (o->isAnimal() ? animals : plants)++;
        }
        if (plants > 1) throw std::logic_error("more than one plant in a cell");
        if (animals > 1) throw std::logic_error("more than one animal in a cell");
    }
    std::size_t indexed = 0;
    for (const auto& [pos, cell] : m_grid) indexed += cell.size();
    if (indexed != m_organisms.size()) throw std::logic_error("grid and organism list differ in size");
    for (const auto& o : m_organisms) {
        if (o->isAnimal() && o->energy() <= 0) throw std::logic_error("living animal with no energy");
        const Position p = o->position();
        if (p.x < 0 || p.y < 0 || p.x >= m_config.grid_width || p.y >= m_config.grid_height)
            throw std::logic_error("organism outside the grid");
    }
}
