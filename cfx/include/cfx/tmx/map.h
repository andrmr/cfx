#pragma once

namespace cfx::tmx {

/// Provides ready to use data structures based on a TMX tiled map.
class Map
{
    struct TextureTile
    {
        uint32_t x;
        uint32_t y;
        uint32_t w;
        uint32_t h;

        std::string_view src;
    };

    struct Layer
    {
        std::string name;
        uint32_t width;
        uint32_t height;

        struct Tile;
        std::vector<Tile> tiles;

        struct Tile
        {
            uint32_t col;
            uint32_t row;
            const TextureTile* tex;
        };
    };

    struct ObjectGroup
    {
        std::string name;
        uint32_t width;
        uint32_t height;

        struct Polygon;
        std::vector<Polygon> polygons;

        struct Object
        {
            uint32_t x;
            uint32_t y;
            uint32_t width;
            uint32_t height;
        };

        struct Polygon: Object
        {
            Polygon(uint32_t x, uint32_t y, uint32_t width, uint32_t height, std::vector<std::pair<int32_t, int32_t>> points)
                : Object {x, y, width, height},
                  points {std::move(points)}
            {
            }

            std::vector<std::pair<int32_t, int32_t>> points;
        };
    };

    enum class Orientation
    {
        ORTHOGONAL,
        ISOMETRIC
    };

public:
    Map() = default;

    void load(std::string_view tmx_file_path);

    const std::vector<Layer>& layers() const;
    const std::vector<ObjectGroup> object_groups() const;
    uint32_t width() const;
    uint32_t height() const;
    uint32_t tilewidth() const;
    uint32_t tileheight() const;

private:
    uint32_t width_;
    uint32_t height_;
    uint32_t tilewidth_;
    uint32_t tileheight_;

    Orientation orientation_ {Orientation::ORTHOGONAL};

    std::vector<std::string> tex_sources_;
    std::vector<TextureTile> tex_tiles_;
    std::vector<Layer> layers_;
    std::vector<ObjectGroup> object_groups_;
};

} // namespace cfx::tmx