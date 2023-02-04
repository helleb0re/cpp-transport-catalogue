#pragma once

#include <istream>
#include <vector>
#include <string>
#include <unordered_map>

#include "transport_catalogue.h"

namespace transport_catalogue
{
    namespace input_reader
    {
        class InputReader
        {
        public:
            void Load(std::istream &input, TransportCatalogue &transport_catalogue);

        private:
            std::vector<std::string_view> FillStopsVector(const std::string &stops_route_info, char delim);
        };
    }

    namespace detail
    {
        void RightTrim(std::string_view &word);
        void RightTrim(std::string &word);

        void LeftTrim(std::string_view &word);
        void LeftTrim(std::string &word);

        std::string_view sub_string(
            std::string_view s,
            size_t p = 0,
            size_t n = std::string_view::npos);
    }
}