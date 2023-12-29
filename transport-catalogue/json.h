#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>


namespace json {
    class Node;

    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;


    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {

    using Value = std::variant<
            std::nullptr_t, bool, int,
            double, std::string, 
            Dict, Array>;
    public:

        Node() = default;

        Node(const Node&) = default;
        Node(Node&&) noexcept = default;

        Node& operator=(const Node&) = default;

        Node(Value value)
            : value_(std::move(value)){}

        Node(Array array)
            : value_(std::move(array)){}
        Node(Dict map)
            : value_(std::move(map)){}
        Node(int value)
            : value_(value){}
        Node(std::string value)
            : value_(std::move(value)){
            }
        Node(std::nullptr_t null)
            : value_(null){}
        Node(double value)
            : value_(value){}

        Node(bool value)
            : value_(value){}


        const Value& GetValue() const;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Dict& AsMap() const;
        const Array& AsArray() const;

    private:
        Value value_;
    };

    bool operator==(const Node& lhs, const Node& rhs);

    bool operator!=(const Node& lhs, const Node& rhs);

    class Document{
    private:
        Node root_;
    public:

        Document() = default;
        template <typename T>
        Document(T value)
            : root_(std::move(Node{value})){}

        Document(Node root);

        const Node& GetRoot() const;
    };

    bool operator==(const Document& lhs, const Document& rhs);

    bool operator!=(const Document& lhs, const Document& rhs);

    Document Load(std::istream& input);


    void PrintValue(double value, std::ostream& out);

    void PrintValue(int value, std::ostream& out);

    void PrintValue(std::nullptr_t, std::ostream& out);

    void PrintValue(bool value, std::ostream& out);

    void PrintValue(const std::string& value, std::ostream& out);

    void PrintValue(const Array& array, std::ostream& out);

    void PrintValue(const Dict& map, std::ostream& out);


    void PrintNode(const Node& node, std::ostream& out);

    void Print(const Document& doc, std::ostream& output);

}