#include <spdlog/spdlog.h>

#include "cfx/tmx/map.h"
#include "impl/parsing.h"

using namespace cfx::tmx;

void Map::load(std::string_view tmx_file_path)
{
    const auto tmx_map = impl::parse_map(tmx_file_path);
    const auto tmx_dir = std::string_view(tmx_file_path.begin(), tmx_file_path.find_last_of("/"));

    width_      = tmx_map.width;
    height_     = tmx_map.height;
    tilewidth_  = tmx_map.tilewidth;
    tileheight_ = tmx_map.tileheight;

    // todo: orientation
    orientation_ = ("orthogonal" == tmx_map.orientation) ? Orientation::ORTHOGONAL : Orientation::ISOMETRIC;

    // start with gid 1
    tex_tiles_.push_back({});
    for (const auto& ts: tmx_map.tilesets)
    {
        const auto cols = ts.columns;
        const auto rows = ts.tilecount / ts.columns;

        tex_sources_.push_back(std::move(fmt::format("{}/{}", tmx_dir, ts.source)));
        const auto src = std::string_view(tex_sources_.back());

        for (auto r = 0; r < rows; ++r)
            for (auto c = 0; c < cols; ++c)
            {
                TextureTile t {
                    .x   = c * ts.tilewidth,
                    .y   = r * ts.tileheight,
                    .w   = ts.tilewidth,
                    .h   = ts.tileheight,
                    .src = src,
                };

                tex_tiles_.push_back(std::move(t));
            }
    }

    for (const auto& lr: tmx_map.layers)
    {
        Layer layer {
            .name   = lr.name,
            .width  = lr.width,
            .height = lr.height,
        };

        auto gid = lr.gids.begin();
        for (auto row = 0U; row < lr.height; ++row)
            for (auto col = 0U; col < lr.width; ++col)
            {
                if (*gid > 0)
                    layer.tiles.emplace_back(col, row, &tex_tiles_[*gid]);

                ++gid;
            }

        layers_.push_back(std::move(layer));
    }

    for (auto& og: tmx_map.object_groups)
    {
        ObjectGroup object_group {
            .name   = og.name,
            .width  = og.width,
            .height = og.height,
        };

        // todo: other types of objects
        for (const auto& p: og.polygons)
        {
            object_group.polygons.emplace_back(p.x, p.y, p.width, p.height, p.points);
        }
    }
}

const std::vector<Map::Layer>& Map::layers() const
{
    return layers_;
}

const std::vector<Map::ObjectGroup> Map::object_groups() const
{
    return object_groups_;
}

uint32_t Map::width() const
{
    return width_;
}

uint32_t Map::height() const
{
    return height_;
}

uint32_t Map::tilewidth() const
{
    return tilewidth_;
}

uint32_t Map::tileheight() const
{
    return tileheight_;
}