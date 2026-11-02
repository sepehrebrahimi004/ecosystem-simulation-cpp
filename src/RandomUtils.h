#ifndef RANDOM_UTILS_H
#define RANDOM_UTILS_H

#include <algorithm>
#include <random>
#include <vector>

// Single shared Mersenne Twister engine. Seeding it makes a whole run reproducible.
class RandomUtils {
public:
    static void seed(unsigned int value) { engine().seed(value); }

    static int randomInt(int min, int max) {
        if (min > max) std::swap(min, max);
        return std::uniform_int_distribution<int>(min, max)(engine());
    }

    template <typename T>
    static void shuffle(std::vector<T>& vec) {
        std::shuffle(vec.begin(), vec.end(), engine());
    }

    // Precondition: vec is not empty.
    template <typename T>
    static const T& randomElement(const std::vector<T>& vec) {
        return vec[static_cast<std::size_t>(randomInt(0, static_cast<int>(vec.size()) - 1))];
    }

private:
    static std::mt19937& engine() {
        static std::mt19937 instance(std::random_device{}());
        return instance;
    }
};

#endif  // RANDOM_UTILS_H
