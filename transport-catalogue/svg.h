#pragma once

#include <cinttypes>
#include <deque>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>


namespace svg {

    struct Rgb{
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;

        Rgb() = default;

        Rgb(uint8_t r, uint8_t g, uint8_t b)
            : red(r)
            , green(g)
            , blue(b){}
    };


    struct Rgba : Rgb {
        double opacity = 1.0;

        Rgba() = default;

        Rgba(uint8_t r, uint8_t g, uint8_t b, double a)
            : Rgb(r, g, b)
            , opacity(a){}
    };
 

    using Color = std::variant<
        std::monostate, std::string,
        Rgb, Rgba
    >;


    inline const Color NoneColor{"none"};

    struct OstreamColorPrinter{

        std::ostream& out;

        void operator()(std::monostate);

        void operator()(const std::string& color);

        void operator()(const Rgb& rgb);

        void operator()(const Rgba& rgba);
    };

    std::ostream& operator<<(std::ostream& out, const Color& color);

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE
    };


    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap);


    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND
    };

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join);


    template <typename Owner>
    class PathProps{
    private:
        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;

        Owner& AsOwner(){
            return static_cast<Owner&>(*this);
        }
    protected:
        ~PathProps() = default;
        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;
            if (fill_color_){
                out << " fill=\""sv << fill_color_.value() << "\""sv;
            }

            if (stroke_color_){
                out << " stroke=\""sv << stroke_color_.value() << "\""sv;
            }

            if (stroke_width_){
                out << " stroke-width=\""sv << stroke_width_.value() << "\""sv;
            }

            if (line_cap_){
                out << " stroke-linecap=\""sv << line_cap_.value() << "\""sv;
            }

            if (line_join_){
                out << " stroke-linejoin=\""sv << line_join_.value() << "\""sv;
            }
        }
    public:
        Owner& SetFillColor(Color color){
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color stroke){
            stroke_color_ = std::move(stroke);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width){
            stroke_width_ = std::move(width);
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap){
            line_cap_ = std::move(line_cap);
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join){
            line_join_ = std::move(line_join);
            return AsOwner();
        }
    };


    struct Point {
        double x = 0.0;
        double y = 0.0;

        Point() = default;

        Point(double xs, double ys)
            : x(xs), y(ys){}
    };

    struct RenderContext{

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;

        RenderContext(std::ostream& outp)
            : out(outp){}

        RenderContext(std::ostream& outp, int indent_stepp, int indentp = 0)
            : out(outp)
            , indent_step(indent_stepp)
            , indent(indentp){}

        RenderContext Indented() const {
            return {out, indent_step, indent};
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i){
                out.put(' ');
            }
        }
    };


    class Object{
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;
    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class Circle final: public Object, public PathProps<Circle> {
    private:
        Point center_;
        double radius_ = 1.0;

        void RenderObject(const RenderContext& context) const override;
    public:
        Circle& SetCenter(Point center);

        Circle& SetRadius(double radius);

    };


    class Polyline final : public Object, public PathProps<Polyline> {
    private:
        std::deque<Point> deque_;

        void RenderObject(const RenderContext& context) const override;
    public:
        Polyline() = default;

        Polyline& AddPoint(Point point);
    };


    class Text final : public Object, public PathProps<Text> {
    private:
        Point position_{0, 0};
        Point offset_{0, 0};
        uint32_t size_ = 1;
        std::string weight_;
        std::string family_;
        std::string data_;

        void RenderObject(const RenderContext& context) const override;

        void AddTextArgs(std::ostream& out) const;
    public:
        Text() = default;

        Text& SetPosition(Point p);

        Text& SetOffset(Point p);

        Text& SetFontSize(uint32_t size);

        Text& SetFontWeight(std::string weight);

        Text& SetFontFamily(std::string family);

        Text& SetData(std::string data);
    };


    class ObjectContainer{
    public:
        virtual ~ObjectContainer() = default;

        virtual void AddPtr(std::unique_ptr<Object>&& obj_ptr) = 0;

        template <typename T>
        void Add(T obj){
            AddPtr(
                std::move(std::make_unique<T>(std::move(obj)))
            );
        }
    };


    class Document final : public ObjectContainer {
    private:
        std::deque<std::unique_ptr<Object>> docs_;

    public:
        Document() = default;

        void AddPtr(std::unique_ptr<Object>&& obj);

        void Render(std::ostream& out) const;
    };

    class Drawable{
    public:
        virtual ~Drawable() = default;
        virtual void Draw(ObjectContainer& container) const = 0;
    };
}
