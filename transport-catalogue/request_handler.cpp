#include "request_handler.h"

using namespace std;

namespace transport_catalogue
{
    RequestHandler::RequestHandler(const TransportCatalogue &db, renderer::MapRenderer &renderer)
        : db_(db), renderer_(renderer) {}

    optional<BusStat> RequestHandler::GetBusStat(const string_view &bus_name) const
    {
        Bus *bus = db_.FindBus(bus_name);

        if (!bus)
            return nullopt;

        BusStat bus_info;

        bus_info.all_stops = bus->route.size();

        if (!bus->is_roundtrip)
        {
            bus_info.all_stops = 2 * bus->route.size() - 1;
        }

        unordered_set<Stop*> tmp(bus->route.begin(), bus->route.end());
        bus_info.unique_stops = tmp.size();

        bus_info.route_length = bus->route_length;
        bus_info.curvature = bus->route_length / bus->geo_length;

        return bus_info;
    }

    const unordered_set<string_view> *RequestHandler::GetBusesByStop(const string_view &stop_name) const
    {
        return db_.GetBusSchedules(stop_name);
    }

    void RequestHandler::SetRenderSettings(const renderer::RenderSettings &settings)
    {
        renderer_.SetOrUpdateRenderSettings(settings);
    }

    svg::Document RequestHandler::RenderMap() const
    {
        const auto &deq_buses = db_.GetAllBuses();

        vector<Bus> buses(deq_buses.begin(), deq_buses.end());
        sort(buses.begin(), buses.end(), [](const auto &bus_a, const auto &bus_b)
             { return bus_a.name < bus_b.name; });

        return renderer_.RenderMap(buses);
    }
} // namespace transport_catalogue
