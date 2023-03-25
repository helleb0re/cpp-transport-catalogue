#pragma once

#include <string>
#include <vector>
#include <utility>
#include <variant>

#include "geo.h"

using namespace std::string_literals;

namespace transport_catalogue
{
    struct Stop
    {
        std::string name;
        geo::Coordinates coord;
        size_t id;
    };

    struct Bus
    {
        std::string name;
        std::vector<Stop *> route;
        double route_length = 0;
        double geo_length = 0;
        bool is_roundtrip;
    };

    struct BusStat
    {
        int all_stops;
        int unique_stops;
        double route_length = 0;
        double curvature;
    };

    struct PathDataItemBus
    {
        PathDataItemBus(std::string_view name, int span_count, double time)
            : name(name), span_count(span_count), time(time) {}

        std::string type = "Bus"s;
        std::string_view name;
        int span_count;
        double time;
    };

    struct PathDataItemWait
    {
        PathDataItemWait(std::string_view stop_name, double time)
            : stop_name(std::move(stop_name)), time(time) {}

        std::string type = "Wait"s;
        std::string_view stop_name;
        double time;
    };

    using PathDataItem = std::variant<PathDataItemBus, PathDataItemWait>;
    struct PathData
    {
        double total_time = 0;
        std::vector<PathDataItem> items;
    };
} // namespace transport_catalogue
