// Predator-prey ecosystem simulator: command-line entry point.
//
//   ecosystem [config.json] [--seed N] [--steps N] [--out DIR] [--display] [--delay MS]
//
// Writes DIR/population.csv every step and DIR/positions.csv every
// `log_positions_every` steps. --display draws the grid in the terminal.
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>

#include "Config.h"
#include "RandomUtils.h"
#include "World.h"

namespace fs = std::filesystem;

namespace {

struct Options {
    std::string configFile{"config.json"};
    std::string outDir{"output"};
    int seed{-2};   // -2: not given on the command line
    int steps{-1};  // -1: use the config value
    bool display{false};
    int delayMs{50};
};

Options parseArgs(int argc, char* argv[]) {
    Options opt;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto value = [&]() -> std::string {
            if (i + 1 >= argc) throw std::runtime_error("missing value after " + arg);
            return argv[++i];
        };
        if (arg == "--seed") opt.seed = std::stoi(value());
        else if (arg == "--steps") opt.steps = std::stoi(value());
        else if (arg == "--out") opt.outDir = value();
        else if (arg == "--display") opt.display = true;
        else if (arg == "--delay") opt.delayMs = std::stoi(value());
        else if (arg == "-h" || arg == "--help") {
            std::cout << "usage: ecosystem [config.json] [--seed N] [--steps N] [--out DIR] [--display] [--delay MS]\n";
            std::exit(0);
        } else if (!arg.empty() && arg[0] == '-') throw std::runtime_error("unknown option " + arg);
        else opt.configFile = arg;
    }
    return opt;
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        const Options opt = parseArgs(argc, argv);
        Config config = Config::loadFromFile(opt.configFile);
        if (opt.seed != -2) config.seed = opt.seed;
        if (opt.steps >= 0) config.max_steps = opt.steps;

        const unsigned seed = config.seed >= 0 ? static_cast<unsigned>(config.seed) : std::random_device{}();
        RandomUtils::seed(seed);

        fs::create_directories(opt.outDir);
        std::ofstream population(fs::path(opt.outDir) / "population.csv");
        std::ofstream positions(fs::path(opt.outDir) / "positions.csv");
        if (!population || !positions) throw std::runtime_error("cannot write to '" + opt.outDir + "'");
        population << "step,plants,herbivores,carnivores\n";
        positions << "step,x,y,type\n";

        World world(config);
        world.initialize();
        auto log = [&]() {
            const PopulationCounts n = world.counts();
            population << world.currentStep() << ',' << n.plants << ',' << n.herbivores << ','
                       << n.carnivores << '\n';
            const int every = config.log_positions_every;
            if (every > 0 && world.currentStep() % every == 0) world.writePositions(positions);
        };

        std::cerr << "seed " << seed << ", " << config.grid_width << "x" << config.grid_height
                  << " grid, up to " << config.max_steps << " steps\n";
        log();
        while (world.currentStep() < config.max_steps && world.counts().total() > 0) {
            world.step();
            log();
            if (opt.display) {
                world.render(std::cout);
                std::this_thread::sleep_for(std::chrono::milliseconds(opt.delayMs));
            }
        }

        const PopulationCounts n = world.counts();
        std::cerr << "finished at step " << world.currentStep() << ": " << n.plants << " plants, "
                  << n.herbivores << " herbivores, " << n.carnivores << " carnivores\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
}
