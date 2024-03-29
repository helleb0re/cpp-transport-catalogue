#pragma once

#include "json.h"
#include "json_builder.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <vector>
#include <unordered_map>
#include <iostream>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <optional>
#include <sstream>

namespace transport_catalogue
{
    namespace detail {
        svg::Color ParseColor(const json::Node &node);
    } // detail

    namespace iodata
    {
        class JsonReader
        {
        public:
            JsonReader(TransportCatalogue &db, RequestHandler &req_handler);

            void LoadFile(std::istream &input);
            void LoadFile(const json::Document& document);

            void InputData();
            void SaveResponseFile(std::ostream &out);

            const json::Document &GetJsonDocument() const;

        private:
            void InputDataBase();
            void InputRenderSettings();
            void InputRoutingSettings();
            void OutputData(std::ostream &out);

            std::optional<json::Document> doc_;
            TransportCatalogue &db_;
            RequestHandler &req_handler_;
        };
    } // namespace json_reader

} // namespace transport_catalogue
