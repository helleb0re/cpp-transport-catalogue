#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

#include <unordered_set>
#include <utility>
#include <string>
#include <vector>
#include <algorithm>

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
namespace transport_catalogue
{
    class RequestHandler
    {
    public:
        RequestHandler(const TransportCatalogue &db, renderer::MapRenderer &renderer);

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<BusStat> GetBusStat(const std::string_view &bus_name) const;

        // Возвращает маршруты, проходящие через
        const std::unordered_set<std::string_view> *GetBusesByStop(const std::string_view &stop_name) const;

        svg::Document RenderMap() const;

        void SetRenderSettings(const renderer::RenderSettings &settings);

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const TransportCatalogue &db_;
        renderer::MapRenderer &renderer_;
    };
} // namespace transport_catalogue
