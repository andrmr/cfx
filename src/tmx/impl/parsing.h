#pragma once

#include <spdlog/spdlog.h>

#include <cereal/external/rapidxml/rapidxml.hpp>
#include <cereal/external/rapidxml/rapidxml_utils.hpp>

#include <charconv>
#include <sstream>

#include "cfx/util/concepts.h"

namespace cfx::tmx::impl {

// https://doc.mapeditor.org/en/stable/reference/tmx-map-format
inline constexpr auto FORMAT_VERSION = "1.4";

using XmlNodePtr = const cereal::rapidxml::xml_node<char>*;

template <util::Arithmetic T>
void get_attribute_value(XmlNodePtr from_node, T& into_val, const char* attribute_name)
{
    if (const auto attribute = from_node->first_attribute(attribute_name))
        std::from_chars(attribute->value(), attribute->value() + attribute->value_size(), into_val);
}

template <util::CharAssignable T>
void get_attribute_value(XmlNodePtr from_node, T& into_val, const char* attribute_name)
{
    if (const auto attribute = from_node->first_attribute(attribute_name))
        into_val = attribute->value();
}

enum class Encoding
{
    XML,
    CSV
};

using Gid = uint32_t;
struct Layer;
struct Tileset;
struct ObjectGroup;

struct Map
{
    std::string orientation;

    uint32_t width;
    uint32_t height;
    uint32_t tilewidth;
    uint32_t tileheight;

    std::vector<Tileset> tilesets;
    std::vector<Layer> layers;
    std::vector<ObjectGroup> object_groups;
};

struct Tileset
{
    uint32_t tilewidth;
    uint32_t tileheight;
    uint32_t tilecount;
    uint32_t columns;

    std::string source;
    uint32_t source_width;
    uint32_t source_height;
};

struct Layer
{
    std::string name;
    uint32_t width;
    uint32_t height;

    std::vector<Gid> gids;
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
        std::vector<std::pair<int32_t, int32_t>> points;
    };
};

Tileset parse_tileset(XmlNodePtr tmx_tileset)
{
    Tileset ts;

    get_attribute_value(tmx_tileset, ts.tilewidth, "tilewidth");
    get_attribute_value(tmx_tileset, ts.tileheight, "tileheight");
    get_attribute_value(tmx_tileset, ts.tilecount, "tilecount");
    get_attribute_value(tmx_tileset, ts.columns, "columns");

    auto tmx_image = tmx_tileset->first_node("image");
    get_attribute_value(tmx_image, ts.source, "source");
    get_attribute_value(tmx_image, ts.source_width, "width");
    get_attribute_value(tmx_image, ts.source_height, "height");

    return ts;
}

Layer parse_layer(XmlNodePtr tmx_layer)
{
    Layer lr;

    get_attribute_value(tmx_layer, lr.name, "name");
    get_attribute_value(tmx_layer, lr.width, "width");
    get_attribute_value(tmx_layer, lr.height, "height");

    auto tmx_tile_data = tmx_layer->first_node("data");
    auto encoding      = Encoding::XML;

    if (const auto tmx_encoding = tmx_tile_data->first_attribute("encoding"))
    {
        // todo: encoding options
        encoding = Encoding::CSV;
    }

    switch (encoding)
    {
        case Encoding::XML: {
            for (auto tmx_tile_gid = tmx_tile_data->first_node("tile"); tmx_tile_gid; tmx_tile_gid = tmx_tile_gid->next_sibling())
            {
                Gid gid {0};
                get_attribute_value(tmx_tile_gid, gid, "gid");
                lr.gids.push_back(gid);
            }

            break;
        }

        case Encoding::CSV: {
            std::istringstream ss(tmx_tile_data->value());

            for (Gid gid {0}; ss >> gid;)
            {
                lr.gids.push_back(gid);
                if (ss.peek() == ',' || ss.peek() == '\r' || ss.peek() == '\n')
                    ss.ignore();
            }

            break;
        }

        default:
            break;
    }

    return lr;
}

ObjectGroup parse_object_group(XmlNodePtr tmx_object_group)
{
    ObjectGroup og;

    get_attribute_value(tmx_object_group, og.name, "name");
    get_attribute_value(tmx_object_group, og.width, "width");
    get_attribute_value(tmx_object_group, og.height, "height");

    for (auto tmx_object = tmx_object_group->first_node("object"); tmx_object; tmx_object = tmx_object->next_sibling())
    {
        ObjectGroup::Object o;

        get_attribute_value(tmx_object, o.x, "x");
        get_attribute_value(tmx_object, o.y, "y");
        get_attribute_value(tmx_object, o.width, "width");
        get_attribute_value(tmx_object, o.height, "height");

        if (const auto tmx_polygon = tmx_object->first_node("polygon"))
        {
            ObjectGroup::Polygon p {std::move(o)};

            std::istringstream ss(tmx_polygon->first_attribute("points")->value());
            for (int32_t x {0}, y {0}; ss >> x, ss.ignore(), ss >> y;)
            {
                p.points.push_back({p.x + x, p.y + y});
            }

            og.polygons.push_back(p);
        }
    }

    for (const auto& p: og.polygons)
    {
        SPDLOG_DEBUG("polygon x {}, y {}", p.x, p.y);
        for (const auto& [x, y]: p.points)
            SPDLOG_DEBUG("xy: {};{}", x, y);
    }

    return og;
}

Map parse_map(std::string_view tmx_file_path)
{
    Map map;

    cereal::rapidxml::file<> tmx_file(tmx_file_path.data());
    cereal::rapidxml::xml_document<> tmx;
    tmx.parse<0>(tmx_file.data());

    auto tmx_root = tmx.first_node("map");

    std::string_view format_version;
    get_attribute_value(tmx_root, format_version, "version");

    if (format_version != FORMAT_VERSION)
        throw std::runtime_error(fmt::format("Wrong TMX format version: {}. Expected: {}.", format_version, FORMAT_VERSION));

    get_attribute_value(tmx_root, map.orientation, "orientation");
    get_attribute_value(tmx_root, map.width, "width");
    get_attribute_value(tmx_root, map.height, "height");
    get_attribute_value(tmx_root, map.tilewidth, "tilewidth");
    get_attribute_value(tmx_root, map.tileheight, "tileheight");

    for (auto tmx_node = tmx_root->first_node(); tmx_node; tmx_node = tmx_node->next_sibling())
    {
        auto tmx_node_type = std::string_view(tmx_node->name());

        if ("tileset" == tmx_node_type)
        {
            map.tilesets.push_back(std::move(parse_tileset(tmx_node)));
        }
        else if ("layer" == tmx_node_type)
        {
            map.layers.push_back(std::move(parse_layer(tmx_node)));
        }
        else if ("objectgroup" == tmx_node_type)
        {
            map.object_groups.push_back(std::move(parse_object_group(tmx_node)));
        }
        else if ("imagelayer" == tmx_node_type)
        {
            // todo
        }
        else
        {
            SPDLOG_ERROR("[TMX] Unknown TMX node type: {}", tmx_node_type);
        }
    }

    return map;
}

} // namespace cfx::tmx::impl
