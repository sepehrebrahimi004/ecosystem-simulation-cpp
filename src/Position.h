#ifndef POSITION_H
#define POSITION_H

#include <cstddef>
#include <functional>
#include <vector>

// A cell coordinate on the grid; (0, 0) is the top-left corner.
struct Position {
    int x{0};
    int y{0};

    bool operator==(const Position& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Position& other) const { return !(*this == other); }

    // The up to eight cells around this one (Moore neighbourhood) that lie inside the grid.
    std::vector<Position> neighbors(int gridWidth, int gridHeight) const {
        std::vector<Position> result;
        result.reserve(8);
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue;
                const int nx = x + dx;
                const int ny = y + dy;
                if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight) {
                    result.push_back({nx, ny});
                }
            }
        }
        return result;
    }
};

// Hash so that Position can key an std::unordered_map (boost::hash_combine style).
namespace std {
template <>
struct hash<Position> {
    std::size_t operator()(const Position& p) const noexcept {
        const std::size_t h1 = std::hash<int>{}(p.x);
        const std::size_t h2 = std::hash<int>{}(p.y);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};
}  // namespace std

#endif  // POSITION_H
