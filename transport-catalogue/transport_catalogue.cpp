#include "transport_catalogue.h"

using namespace std;

namespace transport_catalogue
{
    void TransportCatalogue::AddStop(const string_view &name, geo::Coordinates coord)
    {
        stops_.push_back({string(name), coord});
        stopname_to_stop_.insert({stops_.back().name, &stops_.back()});
        stop_to_buses_.insert({&stops_.back(), {}});
    }

    void TransportCatalogue::AddBus(const std::string_view &name,
                                    const std::vector<std::string_view> &route_stops,
                                    bool is_roundtrip)
    {
        buses_.push_back({string(name), {}, 0, 0, is_roundtrip});

        CalculateRouteDistance(route_stops.begin(), route_stops.end(), is_roundtrip);

        busname_to_bus_.insert({buses_.back().name, &buses_.back()});
    }

    Stop *TransportCatalogue::FindStop(const std::string_view name) const
    {
        if (stopname_to_stop_.count(name) == 0)
        {
            // throw std::invalid_argument("Invalid stop name"s);
            return nullptr;
        }
        return stopname_to_stop_.at(name);
    }

    Bus *TransportCatalogue::FindBus(const std::string_view name) const
    {
        if (busname_to_bus_.count(name) == 0)
        {
            // throw invalid_argument("Invalid bus name"s);
            return nullptr;
        }
        return busname_to_bus_.at(name);
    }

    void TransportCatalogue::AddStopDistances(const string_view &name,
                                              const vector<pair<string_view, double>> &distances)
    {
        Stop *stop1 = FindStop(name);
        for (const auto &[second_stop_name, dist] : distances)
        {
            Stop *stop2 = FindStop(second_stop_name);

            stops_ptr_to_distance_.insert({{stop1, stop2}, dist});
        }
    }

    const unordered_set<string_view> *TransportCatalogue::GetBusSchedules(const string_view stop_name) const
    {
        Stop *stop = FindStop(stop_name);
        return stop ? &stop_to_buses_.at(stop) : nullptr;
    }

    const deque<Bus> &TransportCatalogue::GetAllBuses() const
    {
        return buses_;
    }

    double TransportCatalogue::GetRouteLength(Stop *prev_stop, Stop *now_stop, bool is_roundtrip) const
    {
        double res = 0;
        if (stops_ptr_to_distance_.count({prev_stop, now_stop}) == 0)
            res += stops_ptr_to_distance_.at({now_stop, prev_stop});
        else
            res += stops_ptr_to_distance_.at({prev_stop, now_stop});

        if (!is_roundtrip)
            res += GetRouteLength(now_stop, prev_stop, true);

        return res;
    }
} // namespace transport_catalogue
