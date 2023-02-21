#include "svg.h"

namespace svg
{

    using namespace std::literals;

    void OstreamColorPrinter::operator()(std::monostate) const
    {
        out << "none"sv;
    }
    void OstreamColorPrinter::operator()(const std::string &str_color) const
    {
        out << str_color;
    }
    void OstreamColorPrinter::operator()(const Rgb &rgb_color) const
    {
        out << "rgb("sv << +rgb_color.red << ","sv << +rgb_color.green << ","sv << +rgb_color.blue << ")"sv;
    }
    void OstreamColorPrinter::operator()(const Rgba &rgba_color) const
    {
        out << "rgba("sv << +rgba_color.red << ","sv << +rgba_color.green << ","sv << +rgba_color.blue << ","sv
            << rgba_color.opacity << ")"sv;
    }

    std::ostream &operator<<(std::ostream &out, Color color)
    {
        std::visit(OstreamColorPrinter{out}, color);
        return out;
    }

    void Object::Render(const RenderContext &context) const
    {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle &Circle::SetCenter(Point center)
    {
        center_ = center;
        return *this;
    }

    Circle &Circle::SetRadius(double radius)
    {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    Polyline &Polyline::AddPoint(Point point)
    {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out;
        out << "<polyline points=\""sv;
        for (size_t i = 0; i < points_.size(); ++i)
        {
            out << points_[i].x << ","sv << points_[i].y << (i == points_.size() - 1 ? ""sv : " "sv);
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    Text &Text::SetPosition(Point pos)
    {
        pos_ = pos;
        return *this;
    }

    Text &Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    Text &Text::SetFontSize(uint32_t size)
    {
        font_size_ = size;
        return *this;
    }

    Text &Text::SetFontFamily(std::string font_family)
    {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text &Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text &Text::SetData(std::string data)
    {
        data_ = std::move(data);
        return *this;
    }

    namespace detail
    {
        void ReplaceViewSymbols(std::ostream &out, std::string_view text)
        {
            const std::unordered_map<char, std::string_view> symbol_to_change_ =
                {
                    {'"', "&quot;"sv},
                    {'\'', "&apos;"sv},
                    {'<', "&lt;"sv},
                    {'>', "&gt;"sv},
                    {'&', "&amp;"sv}};
            
            std::for_each(text.begin(), text.end(), [&](char c)
                          {
            if (symbol_to_change_.count(c) > 0) {
                out << symbol_to_change_.at(c);
            } else {
                out << c;
            } });
        }
    } // namespace detail

    void Text::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out;
        out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\""sv;
        out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
        out << " font-size=\""sv << font_size_ << "\""sv;

        if (!font_family_.empty())
            out << " font-family=\""sv << font_family_ << "\""sv;

        if (!font_weight_.empty())
            out << " font-weight=\""sv << font_weight_ << "\""sv;

        RenderAttrs(out);
        out << ">"sv;

        if (!data_.empty())
            detail::ReplaceViewSymbols(out, data_);

        out << "</text>"sv;
    }

    void Document::AddPtr(std::unique_ptr<Object> &&obj)
    {
        elements_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream &out) const
    {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (const auto &elem : elements_)
        {
            elem->Render({out});
        }
        out << "</svg>" << std::endl;
    }

    std::ostream &operator<<(std::ostream &out, StrokeLineCap stroke_line_cap)
    {
        using namespace std::string_view_literals;
        std::string_view stroke_line_cap_sv;
        switch (stroke_line_cap)
        {
        case StrokeLineCap::BUTT:
            stroke_line_cap_sv = "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            stroke_line_cap_sv = "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            stroke_line_cap_sv = "square"sv;
            break;
        default:
            break;
        }

        out << stroke_line_cap_sv;
        return out;
    }

    std::ostream &operator<<(std::ostream &out, StrokeLineJoin stroke_line_join)
    {
        using namespace std::string_view_literals;
        std::string_view stroke_line_join_sv;
        switch (stroke_line_join)
        {
        case StrokeLineJoin::ARCS:
            stroke_line_join_sv = "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            stroke_line_join_sv = "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            stroke_line_join_sv = "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            stroke_line_join_sv = "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            stroke_line_join_sv = "round"sv;
            break;
        default:
            break;
        }

        out << stroke_line_join_sv;

        return out;
    }

} // namespace svg