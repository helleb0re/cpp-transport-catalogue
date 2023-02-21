#pragma once

#include <string>
#include <vector>
#include <utility>

#include "geo.h"

namespace transport_catalogue
{

    struct Stop
    {
        std::string name;
        geo::Coordinates coord;
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
} // namespace transport_catalogue
