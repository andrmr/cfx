#include <spdlog/spdlog.h>

#include <set>

#include "cfx/path/astar_path.h"

using namespace cfx::path;
using cfx::util::Point_i;

constexpr auto COST_DIAG {14};
constexpr auto COST_NORM {10};

namespace cfx::path::impl {

std::vector<std::vector<Node>> grid_to_nodes(const Grid& grid)
{
    std::vector<std::vector<Node>> nodes(grid.size(), std::vector<Node>(grid[0].size()));

    for (size_t i = 0; i < grid[0].size(); ++i)
    {
        for (size_t j = 0; j < grid.size(); ++j)
        {
            nodes[i][j] = {walkable: grid[i][j], x: static_cast<uint32_t>(j), y: static_cast<uint32_t>(i)};
        }
    }

    return nodes;
}

uint32_t get_heuristic_distance(const Node& from, const Node& to)
{
    auto dist_x = std::labs(from.x - to.x);
    auto dist_y = std::labs(from.y - to.y);

    if (dist_x > dist_y)
        return COST_DIAG * dist_y + COST_NORM * (dist_x - dist_y);
    else
        return COST_DIAG * dist_x + COST_NORM * (dist_y - dist_x);
}

std::vector<Point_i> retrace_path(const Node& from, const Node& to)
{
    std::vector<Point_i> path;

    auto current = &to;
    while (current != &from)
    {
        path.emplace_back(current->x, current->y);
        current = current->parent;
    }

    return path;
}

std::vector<Node*> get_nbors(const Node& n, std::vector<std::vector<Node>>& nodes)
{
    std::vector<Node*> nbors;

    const auto cols = nodes[0].size();
    const auto rows = nodes.size();

    for (auto x = -1; x <= 1; ++x)
    {
        for (auto y = -1; y <= 1; ++y)
        {
            if (x == 0 && y == 0)
                continue;

            auto nbor_x = n.x + x;
            auto nbor_y = n.y + y;

            if (nbor_x >= 0 && nbor_x < cols && nbor_y >= 0 && nbor_y < rows)
            {
                nbors.push_back(&nodes[nbor_y][nbor_x]);
            }
        }
    }

    return nbors;
}

} // namespace cfx::path::impl

using namespace impl;

std::vector<Point_i> AStarPath::get_path(Point_i const& from, Point_i const& to)
{
    auto& start = nodes_[from.y][from.x];
    auto& end   = nodes_[to.y][to.x];

    std::set<Node*> open_set;
    std::set<Node*> closed_set;

    open_set.insert(&start);

    while (!open_set.empty())
    {
        auto current = *open_set.begin();

        for (auto& n: open_set)
        {
            if (n->f_cost() < current->f_cost() || n->f_cost() == current->f_cost() && n->h_cost < current->h_cost)
            {
                current = n;
            }
        }

        if (current == &end)
        {
            return impl::retrace_path(start, end);
        }

        closed_set.insert(std::move(open_set.extract(current)));

        for (auto nbors = impl::get_nbors(*current, nodes_); auto n: nbors)
        {
            if (n->walkable || closed_set.contains(n))
            {
                continue;
            }

            auto new_cost_to_nbor = current->g_cost + impl::get_heuristic_distance(*current, *n);

            if (new_cost_to_nbor < n->g_cost || !open_set.contains(n))
            {
                n->g_cost = new_cost_to_nbor;
                n->h_cost = impl::get_heuristic_distance(*n, end);
                n->parent = current;

                if (!open_set.contains(n))
                    open_set.insert(n);
            }
        }
    }

    return std::vector<Point_i> {};
}

AStarPath::AStarPath(const std::vector<std::vector<bool>>& grid)
    : nodes_ {impl::grid_to_nodes(grid)},
      cols_ {nodes_[0].size()},
      rows_ {nodes_.size()}
{
}