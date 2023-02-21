#include "map_renderer.h"

using namespace std;

namespace transport_catalogue
{
    namespace detail
    {
        bool IsZero(double value)
        {
            return std::abs(value) < EPSILON;
        }
    } // namespace detail

    namespace renderer
    {
        MapRenderer::MapRenderer(const RenderSettings &render_settings)
            : render_settings_(move(render_settings)) {}

        void MapRenderer::SetOrUpdateRenderSettings(const RenderSettings &render_settings)
        {
            render_settings_ = move(render_settings);
        }

        bool MapRenderer::HasRenderSettings() const
        {
            return render_settings_.has_value();
        }

        svg::Point MapRenderer::SphereProjector::operator()(geo::Coordinates coords) const
        {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
        }

        svg::Document MapRenderer::RenderMap(const vector<Bus> &buses) const
        {
            if (!render_settings_.has_value())
                throw runtime_error("Render settings weren't set"s);

            auto &rs = *render_settings_;
            svg::Document doc;

            vector<geo::Coordinates> coords;
            for_each(buses.begin(), buses.end(), [&coords](const auto &bus)
                     { transform(bus.route.begin(), bus.route.end(), back_inserter(coords),
                                 [](const Stop *stop)
                                 { return stop->coord; }); });

            const SphereProjector proj{
                coords.begin(), coords.end(),
                rs.width, rs.height, rs.padding};

            vector<svg::Circle> svg_stops_circles;
            vector<vector<svg::Text>> svg_stops_text;
            vector<vector<svg::Text>> svg_buses_names;

            auto cmp = [](const Stop *a, const Stop *b)
            { return a->name < b->name; };
            set<const Stop *, decltype(cmp)> stops(cmp);

            size_t k = 0;
            for (const auto &bus : buses)
            {
                if (bus.route.size() < 1)
                    continue;

                svg::Polyline polyline;
                svg::Color bus_color = rs.color_palette[k % rs.color_palette.size()];

                polyline.SetStrokeColor(bus_color);
                polyline.SetFillColor(svg::NoneColor);
                polyline.SetStrokeWidth(rs.line_width);
                polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

                for_each(bus.route.begin(), bus.route.end(),
                         [&](const Stop *stop)
                         {
                             svg::Point p = proj(stop->coord);
                             polyline.AddPoint(p);
                             stops.insert(stop);
                         });

                if (!bus.is_roundtrip)
                {
                    for_each(bus.route.rbegin() + 1, bus.route.rend(), [&](const Stop *stop)
                             {
                        svg::Point p = proj(stop->coord);
                        polyline.AddPoint(p); });
                }

                vector<Stop *> tmp;
                tmp.push_back(bus.route[0]);
                if (!bus.is_roundtrip && bus.route[0] != bus.route[bus.route.size() - 1])
                    tmp.push_back(bus.route[bus.route.size() - 1]);

                for_each(tmp.begin(), tmp.end(), [&](const Stop *stop)
                         {
                             svg::Point p = proj(stop->coord);
                             svg_buses_names.push_back({});

                             for (int i = 0; i < 2; ++i)
                             {
                                 svg_buses_names.back().push_back({});
                                 auto &svg_text = svg_buses_names.back().back();

                                 svg_text.SetPosition(p);
                                 svg_text.SetData(bus.name);
                                 svg_text.SetFontSize(rs.bus_label_font_size);
                                 svg_text.SetFontFamily("Verdana"s);
                                 svg_text.SetFontWeight("bold"s);
                                 svg_text.SetOffset(rs.bus_label_offset);
                             }
                             svg_buses_names.back()[0].SetFillColor(rs.underlayer_color);
                             svg_buses_names.back()[0].SetStrokeWidth(rs.underlayer_width);
                             svg_buses_names.back()[0].SetStrokeColor(rs.underlayer_color);
                             svg_buses_names.back()[0].SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                             svg_buses_names.back()[0].SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

                             svg_buses_names.back()[1].SetFillColor(bus_color); });

                ++k;

                doc.Add(polyline);
            }

            for (const auto &svg_bus_name : svg_buses_names)
            {
                doc.Add(svg_bus_name[0]);
                doc.Add(svg_bus_name[1]);
            }

            for_each(stops.begin(), stops.end(), [&](const Stop *stop)
                     {
                          svg::Point p = proj(stop->coord);

                          svg_stops_circles.push_back({});

                          svg_stops_circles.back().SetCenter(p);
                          svg_stops_circles.back().SetRadius(rs.stop_radius);
                          svg_stops_circles.back().SetFillColor("white"s);

                          svg_stops_text.push_back({});

                          for (int i = 0; i < 2; ++i)
                          {
                              svg_stops_text.back().push_back({});
                              auto &svg_stop_text = svg_stops_text.back().back();

                              svg_stop_text.SetPosition(p);
                              svg_stop_text.SetData(stop->name);
                              svg_stop_text.SetFontFamily("Verdana"s);
                              svg_stop_text.SetFontSize(rs.stop_label_font_size);
                              svg_stop_text.SetOffset(rs.stop_label_offset);
                          }
                          svg_stops_text.back()[0].SetFillColor(rs.underlayer_color);
                          svg_stops_text.back()[0].SetStrokeWidth(rs.underlayer_width);
                          svg_stops_text.back()[0].SetStrokeColor(rs.underlayer_color);
                          svg_stops_text.back()[0].SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                          svg_stops_text.back()[0].SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

                          svg_stops_text.back()[1].SetFillColor("black"s); });

            for (const auto &svg_stop_circle : svg_stops_circles)
            {
                doc.Add(svg_stop_circle);
            }

            for (const auto &svg_stop_text : svg_stops_text)
            {
                doc.Add(svg_stop_text[0]);
                doc.Add(svg_stop_text[1]);
            }

            return doc;
        }

    } // namespace renderer

} // namespace transport_catalogue
