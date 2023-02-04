#pragma once

#include <deque>
#include <string>
#include <unordered_map>
#include <string_view>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <unordered_set>
#include <algorithm>
#include <set>
#include <iterator>

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
    };

    struct BusInfo
    {
        int all_stops;
        int unique_stops;
        double route_length = 0;
        double curvature;
    };

    struct StopInfo
    {
        std::vector<std::string_view> buses;
    };

    namespace detail
    {
        struct StopsPairHash
        {
            size_t operator()(const std::pair<Stop *, Stop *> &stops_pair) const
            {
                return hasher_(stops_pair.first) + hasher_(stops_pair.second) * 37;
            }

        private:
            std::hash<const void *> hasher_;
        };
    } // namespace detail

    class TransportCatalogue
    {
    public:
        void AddStop(std::string_view name, double lat, double lng);
        Stop *FindStop(const std::string_view name);

        void AddBus(std::string_view name, const std::vector<std::string_view> &route_stops);
        Bus *FindBus(const std::string_view name);

        BusInfo GetBusInfo(const std::string_view name);
        StopInfo GetStopInfo(const std::string_view name);

        void AddStopDistances(std::string_view name,
                              const std::vector<std::pair<std::string_view, double>> &data);

    private:
        /* data */
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, Stop *> stopname_to_stop_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Bus *> busname_to_bus_;
        std::unordered_map<Stop *, std::set<std::string_view>> stop_to_buses_;
        std::unordered_map<std::pair<Stop *, Stop *>, double, detail::StopsPairHash> stops_ptr_to_distance_;
    };

} // namespace transport_catalogue
