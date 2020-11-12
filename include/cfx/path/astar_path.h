#pragma once

#include "cfx/util/point.h"

// todo: cleanup interface and implementation

namespace cfx::path {

using Grid = std::vector<std::vector<bool>>;

namespace impl {
struct Node;
} // namespace impl

class AStarPath
{
private:
    std::vector<std::vector<impl::Node>> nodes_;

    size_t cols_;
    size_t rows_;

public:
    AStarPath(const std::vector<std::vector<bool>>& grid);
    std::vector<util::Point_i> get_path(const util::Point_i& from, const util::Point_i& to);
};

namespace impl {
struct Node
{
    bool walkable {false};

    uint32_t x {0};
    uint32_t y {0};

    uint32_t g_cost {0};
    uint32_t h_cost {0};

    const Node* parent {nullptr};

    Node() = default;

    Node(bool walkable, uint32_t x, uint32_t y)
        : walkable {walkable}, x {x}, y {y}
    {
    }

    uint32_t f_cost() const
    {
        return g_cost + h_cost;
    }
};
} // namespace impl

} // namespace cfx::path