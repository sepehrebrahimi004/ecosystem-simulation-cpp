// Minimal self-contained tests (no framework): build with `make test`.
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "Config.h"
#include "RandomUtils.h"
#include "World.h"

namespace {

int failures = 0;

void check(bool ok, const std::string& what) {
    if (!ok) {
        std::cerr << "  FAIL: " << what << '\n';
        ++failures;
    }
}

void run(const std::string& name, const std::function<void()>& test) {
    const int before = failures;
    try {
        test();
    } catch (const std::exception& e) {
        check(false, std::string("exception: ") + e.what());
    }
    std::cout << (failures == before ? "[ ok ] " : "[FAIL] ") << name << '\n';
}

Config small() {
    Config c;
    c.grid_width = 20;
    c.grid_height = 15;
    c.initial_plants = 60;
    c.initial_herbivores = 20;
    c.initial_carnivores = 5;
    return c;
}

std::vector<int> trajectory(const Config& c, unsigned seed, int steps) {
    RandomUtils::seed(seed);
    World w(c);
    w.initialize();
    std::vector<int> out;
    for (int i = 0; i < steps; ++i) {
        w.step();
        const auto n = w.counts();
        out.insert(out.end(), {n.plants, n.herbivores, n.carnivores});
    }
    return out;
}

}  // namespace

int main() {
    run("initial populations are placed exactly", [] {
        RandomUtils::seed(1);
        World w(small());
        w.initialize();
        const auto n = w.counts();
        check(n.plants == 60 && n.herbivores == 20 && n.carnivores == 5, "initial counts");
        w.checkInvariants();
    });

    run("occupancy invariant holds for 500 steps (default parameters)", [] {
        RandomUtils::seed(2);
        World w(Config{});
        w.initialize();
        for (int i = 0; i < 500 && w.counts().total() > 0; ++i) {
            w.step();
            w.checkInvariants();
        }
    });

    run("same seed gives the same run, different seed a different one", [] {
        check(trajectory(small(), 7, 100) == trajectory(small(), 7, 100), "reproducible");
        check(trajectory(small(), 7, 100) != trajectory(small(), 8, 100), "seed matters");
    });

    run("plants alone fill the grid and stop at one per cell", [] {
        Config c = small();
        c.initial_herbivores = c.initial_carnivores = 0;
        RandomUtils::seed(3);
        World w(c);
        w.initialize();
        for (int i = 0; i < 300; ++i) w.step();
        check(w.counts().plants == c.grid_width * c.grid_height, "grid full of plants");
        w.checkInvariants();
    });

    run("herbivores without food starve after initial_energy / move_cost steps", [] {
        Config c = small();
        c.initial_plants = c.initial_carnivores = 0;
        c.initial_energy_herbivore = 10;
        c.herbivore_move_cost = 1;
        RandomUtils::seed(4);
        World w(c);
        w.initialize();
        for (int i = 0; i < 9; ++i) w.step();
        check(w.counts().herbivores == 20, "all alive after 9 steps");
        w.step();
        check(w.counts().herbivores == 0, "all dead after 10 steps");
    });

    run("a carnivore eats an adjacent herbivore and takes its cell", [] {
        Config c;
        c.grid_width = 2;
        c.grid_height = 1;
        c.initial_plants = 0;
        c.initial_herbivores = 1;
        c.initial_carnivores = 1;
        c.carnivore_reproduce_threshold = 1000;
        RandomUtils::seed(5);
        World w(c);
        w.initialize();
        w.step();
        const auto n = w.counts();
        check(n.herbivores == 0 && n.carnivores == 1, "prey eaten");  // 2 cells: always adjacent
        w.checkInvariants();
    });

    run("invalid configuration is rejected", [] {
        Config c = small();
        c.initial_plants = c.grid_width * c.grid_height + 1;
        bool threw = false;
        try {
            World w(c);
        } catch (const std::runtime_error&) {
            threw = true;
        }
        check(threw, "too many plants throws");
    });

    std::cout << (failures ? "FAILED\n" : "all tests passed\n");
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
