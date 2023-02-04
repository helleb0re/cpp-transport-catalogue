#include "transport_catalogue.h"

using namespace std;

namespace transport_catalogue
{
    void TransportCatalogue::AddStop(string_view name, double lat, double lng)
    {
        stops_.push_back({string(name), {lat, lng}});
        stopname_to_stop_.insert({stops_.back().name, &stops_.back()});
        stop_to_buses_.insert({&stops_.back(), {}});
    }

    void TransportCatalogue::AddBus(std::string_view name, const std::vector<std::string_view> &route_stops)
    {
        buses_.push_back({string(name), {}});
        Stop *now_stop = nullptr;
        Stop *prev_stop = nullptr;
        for (const auto &route_stop : route_stops)
        {
            now_stop = stopname_to_stop_.at(route_stop);

            buses_.back().route.push_back(now_stop);
            stop_to_buses_[now_stop].insert(buses_.back().name);

            if (prev_stop)
            {
                buses_.back().geo_length += geo::ComputeDistance(now_stop->coord, prev_stop->coord);
                if (stops_ptr_to_distance_.count({prev_stop, now_stop}) == 0)
                {
                    buses_.back().route_length += stops_ptr_to_distance_.at({now_stop, prev_stop});
                }
                else
                {
                    buses_.back().route_length += stops_ptr_to_distance_.at({prev_stop, now_stop});
                }
            }

            prev_stop = now_stop;
        }
        busname_to_bus_.insert({buses_.back().name, &buses_.back()});
    }

    Stop *TransportCatalogue::FindStop(const std::string_view name)
    {
        if (stopname_to_stop_.count(name) == 0)
        {
            throw std::invalid_argument("Invalid stop name"s);
        }
        return stopname_to_stop_.at(name);
    }

    Bus *TransportCatalogue::FindBus(const std::string_view name)
    {
        if (busname_to_bus_.count(name) == 0)
        {
            throw std::invalid_argument("Invalid bus name"s);
        }
        return busname_to_bus_.at(name);
    }

    BusInfo TransportCatalogue::GetBusInfo(const std::string_view name)
    {
        Bus *bus = FindBus(name);
        BusInfo bus_info;

        bus_info.all_stops = bus->route.size();

        unordered_set<Stop *> tmp(bus->route.begin(), bus->route.end());
        bus_info.unique_stops = tmp.size();

        bus_info.route_length = bus->route_length;
        bus_info.curvature = bus->route_length / bus->geo_length;

        return bus_info;
    }

    StopInfo TransportCatalogue::GetStopInfo(const std::string_view name)
    {
        Stop *stop = FindStop(name);
        StopInfo stop_info;

        copy(stop_to_buses_.at(stop).begin(),
             stop_to_buses_.at(stop).end(),
             back_inserter(stop_info.buses));

        return stop_info;
    }

    void TransportCatalogue::AddStopDistances(string_view name,
                                              const vector<pair<string_view, double>> &data)
    {
        Stop *stop1 = FindStop(name);
        for (const auto &[second_stop_name, dist] : data)
        {
            Stop *stop2 = FindStop(second_stop_name);

            stops_ptr_to_distance_.insert({{stop1, stop2}, dist});
        }
    }
} // namespace transport_catalogue
