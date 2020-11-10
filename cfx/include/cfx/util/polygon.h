#pragma once

#include "cfx/util/point.h"

namespace cfx::util {

struct Polygon
{
    std::vector<Point_i> corners;
    Point_i origin;
};

class CollisionPolygon: public Polygon
{
private:
    bool is_precalc_ {false};
    std::vector<float> constant_;
    std::vector<float> multiple_;

    /// Precalculates values needed for contains checks.
    void precalc()
    {
        if (is_precalc_)
            [[likely]] return;

        for (int i = 0, j = corners.size() - 1; i < corners.size(); ++i)
        {
            if (corners[j].y == corners[i].y)
            {
                constant_[i] = corners[i].x;
                multiple_[i] = 0;
            }
            else
            {
                constant_[i] = corners[i].x - (corners[i].y * corners[j].x) / (corners[j].y - corners[i].y) + (corners[i].y * corners[i].x) / (corners[j].y - corners[i].y);
                multiple_[i] = (corners[j].x - corners[i].x) / (corners[j].y - corners[i].y);
            }

            j = i;
        }

        is_precalc_ = true;
    }

public:
    CollisionPolygon(std::vector<Point_i> corners, Point_i origin = {0, 0})
        : Polygon {std::move(corners), std::move(origin)},
          constant_ {std::vector<float>(corners.size())},
          multiple_ {std::vector<float>(multiple_.size())}
    {
    }

    /// Returns true if the given point is inside the polygon.
    bool contains(const Point_i& p)
    {
        precalc();

        bool odd_nodes = false, previous = false, current = corners[corners.size() - 1].y > p.y;

        for (int i = 0; i < corners.size(); ++i)
        {
            previous = current;
            current  = corners[i].y > p.y;
            if (current != previous)
                odd_nodes ^= ((p.y * multiple_[i] + constant_[i]) < p.x);
        }

        return odd_nodes;
    }
};

} // namespace cfx::util