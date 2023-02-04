#pragma once

#include <string>
#include <istream>
#include <ostream>
#include <iostream>

#include "transport_catalogue.h"

namespace transport_catalogue
{
    namespace output_reader
    {
        class OutputReader
        {
        public:
            void RequestInfo(std::istream &input, std::ostream &output, TransportCatalogue &transport_catalogue);

        private:
        };
    } // namespace output_reader
} // namespace transport_catalogue
