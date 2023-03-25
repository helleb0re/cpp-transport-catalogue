#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <unordered_set>
#include <utility>
#include <string>
#include <vector>
#include <algorithm>
#include <optional>

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
namespace transport_catalogue
{
    class RequestHandler
    {
    public:
        RequestHandler(const TransportCatalogue &db,
                       renderer::MapRenderer &map_renderer,
                       router::TransportRouter &transport_router);

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<BusStat> GetBusStat(const std::string_view &bus_name) const;

        // Возвращает маршруты, проходящие через
        const std::unordered_set<std::string_view> *GetBusesByStop(const std::string_view &stop_name) const;

        svg::Document RenderMap() const;
        std::optional<PathData> GetShortWayBetween(std::string_view start_stop, std::string_view end_stop);

        void SetRenderSettings(renderer::RenderSettings &settings);
        void SetRoutingSettings(router::RoutingSettings &settings);

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const TransportCatalogue &db_;
        renderer::MapRenderer &map_renderer_;
        router::TransportRouter &transport_router_;
    };
} // namespace transport_catalogue
