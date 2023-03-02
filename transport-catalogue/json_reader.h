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
    namespace iodata
    {
        class JsonReader
        {
        public:
            JsonReader(TransportCatalogue &db, RequestHandler &req_handler, std::ostream &out);

            void LoadFile(std::istream &input);

            const json::Document &GetJsonDocument() const;

        private:
            void InputDataBase();
            void InputRenderSettings();
            void OutputData();

            svg::Color ParseColor(const json::Node &node) const;

            std::optional<json::Document> doc_;
            TransportCatalogue &db_;
            RequestHandler &req_handler_;
            std::ostream &out_;
        };
    } // namespace json_reader

} // namespace transport_catalogue
