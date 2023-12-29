#include "svg.h"

#include <iomanip>
#include <unordered_map>


namespace svg {

    using namespace std::literals;


    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap){
        switch (line_cap){
            case StrokeLineCap::BUTT:
                out << "butt"sv;
                break;
            case StrokeLineCap::ROUND:
                out << "round"sv;
                break;
            case StrokeLineCap::SQUARE:
                out << "square"sv;
                break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join){
        switch (line_join){
            case StrokeLineJoin::ARCS:
                out << "arcs"sv;
                break;
            case StrokeLineJoin::BEVEL:
                out << "bevel"sv;
                break;
            case StrokeLineJoin::MITER:
                out << "miter"sv;
                break;
            case StrokeLineJoin::MITER_CLIP:
                out << "miter-clip"sv;
                break;
            case StrokeLineJoin::ROUND:
                out << "round"sv;
                break;
        }
        return out;
    }
    

    void OstreamColorPrinter::operator()(std::monostate){
            out << NoneColor;
        }

    void OstreamColorPrinter::operator()(const std::string& color){
        out << color;
    }

    void OstreamColorPrinter::operator()(const Rgb& rgb){
        out<< "rgb("s << +rgb.red << ","s << +rgb.green << ","s << +rgb.blue << ")"s;
    }

    void OstreamColorPrinter::operator()(const Rgba& rgba){
        out<< "rgba("s << +rgba.red << ","s << +rgba.green << ","s << +rgba.blue
            << ","s << rgba.opacity << ")"s;
    }

    std::ostream& operator<<(std::ostream& out, const Color& color){
        visit(OstreamColorPrinter{out}, color);
        return out;
    }


    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context);
        context.out << std::endl;
    }

    //-------Circle-------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius){
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y;
        out << "\" r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << " />"sv;
    }

    //-------------------Polyline------------------
    Polyline& Polyline::AddPoint(Point point){
        deque_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;

        bool is_next = false;
        for (const auto& p : deque_){
            if (is_next){
                out << " "sv;
            }
            out << p.x << ',' << p.y;
            is_next = true;
        }

        out << "\""sv;
        RenderAttrs(out);
        out << " />";
    }

    //-------------------Text---------------

    Text& Text::SetPosition(Point p){
        position_ = std::move(p);
        return *this;
    }

    Text& Text::SetOffset(Point p){
        offset_ = std::move(p);
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size){
        size_ = size;
        return *this;
    }

    Text& Text::SetFontWeight(std::string weight){
        weight_ = std::move(weight);
        return *this;
    }

    Text& Text::SetFontFamily(std::string family){
        family_ = std::move(family);
        return *this;
    }

    Text& Text::SetData(std::string data){
        data_ = std::move(data);
        return *this;
    }

    inline static const std::unordered_map<char, std::string> WORD_CONVE{
        {'"', "&quot;"s},
        {'\'', "&apos;"s},
        {'<', "&lt;"s},
        {'>', "&gt;"s},
        {'&', "&amp;"s}
    };

    void Text::AddTextArgs(std::ostream& out) const {
        out << "<text "sv;
        out << "x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_ << "\""sv;

        if (!weight_.empty()){
            out << " font-weight=\""sv << weight_ << "\""sv;
        }

        if(!family_.empty()){
            out << " font-family=\""sv << family_ << "\""sv;
        }
        RenderAttrs(out);
        out << ">"sv;
    }

    void Text::RenderObject(const RenderContext& context ) const {
        auto& out = context.out;
        AddTextArgs(out);
        for (const char& c : data_){
            if (WORD_CONVE.count(c) != 0){
                out << WORD_CONVE.at(c);
            }
            out << c;
        }
        out << "</text>";
    }

    //---------------------Document-----------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj_ptr){
        docs_.emplace_back(std::move(obj_ptr));
    }

    void Document::Render(std::ostream& out) const {
        out << std::setprecision(6);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;

        for (const auto& doc : docs_){
            doc -> Render(RenderContext(out, 0, 2));
        }

        out << "</svg>"sv;
    }
}