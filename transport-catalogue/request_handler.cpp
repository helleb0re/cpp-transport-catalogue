#include "request_handler.h"

using namespace std;

namespace transport_catalogue
{
    using namespace renderer;
    using namespace router;

    RequestHandler::RequestHandler(const TransportCatalogue &db,
                                   MapRenderer &map_renderer,
                                   TransportRouter &transport_router)
        : db_(db), map_renderer_(map_renderer), transport_router_(transport_router) {}

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

        unordered_set<Stop *> tmp(bus->route.begin(), bus->route.end());
        bus_info.unique_stops = tmp.size();

        bus_info.route_length = bus->route_length;
        bus_info.curvature = bus->route_length / bus->geo_length;

        return bus_info;
    }

    const unordered_set<string_view> *RequestHandler::GetBusesByStop(const string_view &stop_name) const
    {
        return db_.GetBusSchedules(stop_name);
    }

    void RequestHandler::SetRenderSettings(renderer::RenderSettings &settings)
    {
        map_renderer_.SetOrUpdateRenderSettings(settings);
    }

    svg::Document RequestHandler::RenderMap() const
    {
        const auto &deq_buses = db_.GetAllBuses();

        vector<Bus> buses(deq_buses.begin(), deq_buses.end());
        sort(buses.begin(), buses.end(), [](const auto &bus_a, const auto &bus_b)
             { return bus_a.name < bus_b.name; });

        return map_renderer_.RenderMap(buses);
    }

    optional<PathData> RequestHandler::GetShortWayBetween(std::string_view start_stop, std::string_view end_stop)
    {
        Stop *start_stop_ptr = db_.FindStop(start_stop);
        Stop *end_stop_ptr = db_.FindStop(end_stop);

        if (!start_stop_ptr || !end_stop_ptr)
            return nullopt;

        if (transport_router_.GetVertexAmount() == 0)
        {
            transport_router_.SetVertexAmount(db_.GetAllStops().size() * 2);
            transport_router_.FillDataToGraph(db_.GetAllBuses(), db_.GetDistancesMap());
        }

        return transport_router_.GetShortWayBetween(start_stop_ptr, end_stop_ptr);
    }

    void RequestHandler::SetRoutingSettings(RoutingSettings &settings)
    {
        transport_router_.SetOrUpdateRoutingSettings(settings);
    }
} // namespace transport_catalogue
