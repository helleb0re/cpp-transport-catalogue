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
#include <iterator>
#include <numeric>

#include "geo.h"
#include "domain.h"

namespace transport_catalogue
{
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
        void AddStop(const std::string_view &name, geo::Coordinates coord);
        Stop *FindStop(const std::string_view name) const;
        const std::unordered_set<std::string_view> *GetBusSchedules(const std::string_view stop_name) const;

        void AddBus(const std::string_view &name, const std::vector<std::string_view> &route_stops, bool is_roundtrip);
        Bus *FindBus(const std::string_view name) const;

        void AddStopDistances(const std::string_view &name,
                              const std::vector<std::pair<std::string_view, double>> &distance);

        const std::deque<Bus> &GetAllBuses() const;

    private:
        /* data */
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, Stop *> stopname_to_stop_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Bus *> busname_to_bus_;
        std::unordered_map<Stop *, std::unordered_set<std::string_view>> stop_to_buses_;
        std::unordered_map<std::pair<Stop *, Stop *>, double, detail::StopsPairHash> stops_ptr_to_distance_;

        double GetRouteLength(Stop *prev_stop, Stop *now_stop, bool is_roundtrip) const;

        template <typename It>
        void CalculateRouteDistance(It it_begin, It it_end, bool is_roundtrip)
        {
            Stop *now_stop = nullptr;
            Stop *prev_stop = nullptr;
            for_each(it_begin, it_end, [&](const auto &route_stop)
                     {
            now_stop = stopname_to_stop_.at(route_stop);

            buses_.back().route.push_back(now_stop);
            stop_to_buses_[now_stop].insert(buses_.back().name);

            if (prev_stop)
            {
                double geo_length = geo::ComputeDistance(now_stop->coord, prev_stop->coord);
                buses_.back().geo_length += geo_length * (is_roundtrip ? 1 : 2);
                
                buses_.back().route_length += GetRouteLength(prev_stop, now_stop, is_roundtrip);
            }

            prev_stop = now_stop; });
        }
    };

} // namespace transport_catalogue
