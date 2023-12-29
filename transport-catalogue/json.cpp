#include "json.h"

using namespace std;
namespace json {

    bool operator==(const Node& lhs, const Node& rhs){
        return lhs.GetValue() == rhs.GetValue();
    }

    bool operator!=(const Node& lhs, const Node& rhs){
        return lhs.GetValue() != rhs.GetValue();
    }

    bool operator==(const Document& lhs, const Document& rhs){
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator!=(const Document& lhs, const Document& rhs){
        return lhs.GetRoot() != rhs.GetRoot();
    }

namespace {
using namespace std::literals;

Node LoadNode(std::istream& input);
Node LoadString(std::istream& input);

std::string LoadLiteral(std::istream& input) {
    std::string s;
    while (std::isalpha(input.peek())) {
        s.push_back(static_cast<char>(input.get()));
    }
    return s;
}

Node LoadArray(std::istream& input) {
    std::vector<Node> result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (!input) {
        throw ParsingError("Array parsing error"s);
    }
    return Node(std::move(result));
}

Node LoadDict(std::istream& input) {
    Dict dict;

    for (char c; input >> c && c != '}';) {
        if (c == '"') {
            std::string key = LoadString(input).AsString();
            if (input >> c && c == ':') {
                if (dict.find(key) != dict.end()) {
                    throw ParsingError("Duplicate key '"s + key + "' have been found");
                }
                dict.emplace(std::move(key), LoadNode(input));
            } else {
                throw ParsingError(": is expected but '"s + c + "' has been found"s);
            }
        } else if (c != ',') {
            throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
        }
    }
    if (!input) {
        throw ParsingError("Dictionary parsing error"s);
    }
    return Node(std::move(dict));
}

Node LoadString(std::istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            ++it;
            break;
        } else if (ch == '\\') {
            ++it;
            if (it == end) {
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            throw ParsingError("Unexpected end of line"s);
        } else {
            s.push_back(ch);
        }
        ++it;
    }

    return Node(std::move(s));
}

Node LoadBool(std::istream& input) {
    const auto s = LoadLiteral(input);
    if (s == "true"sv) {
        return Node{true};
    } else if (s == "false"sv) {
        return Node{false};
    } else {
        throw ParsingError("Failed to parse '"s + s + "' as bool"s);
    }
}

Node LoadNull(std::istream& input) {
    if (auto literal = LoadLiteral(input); literal == "null"sv) {
        return Node{nullptr};
    } else {
        throw ParsingError("Failed to parse '"s + literal + "' as null"s);
    }
}

Node LoadNumber(std::istream& input) {
    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadNode(std::istream& input) {
    char c;
    if (!(input >> c)) {
        throw ParsingError("Unexpected EOF"s);
    }
    switch (c) {
        case '[':
            return LoadArray(input);
        case '{':
            return LoadDict(input);
        case '"':
            return LoadString(input);
        case 't':
            // Атрибут [[fallthrough]] (провалиться) ничего не делает, и является
            // подсказкой компилятору и человеку, что здесь программист явно задумывал
            // разрешить переход к инструкции следующей ветки case, а не случайно забыл
            // написать break, return или throw.
            // В данном случае, встретив t или f, переходим к попытке парсинга
            // литералов true либо false
            [[fallthrough]];
        case 'f':
            input.putback(c);
            return LoadBool(input);
        case 'n':
            input.putback(c);
            return LoadNull(input);
        default:
            input.putback(c);
            return LoadNumber(input);
    }
}
}

    const Node::Value& Node::GetValue() const {
        return value_;
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(value_);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(value_);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }

    bool Node::IsDouble() const {
        return IsPureDouble() || IsInt();
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(value_);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(value_);
    }

    int Node::AsInt() const {
        if (!IsInt()){
            throw std::logic_error("Wrong type requested"s);
        }
        return std::get<int>(value_);
    }

    bool Node::AsBool() const {
        if (!IsBool()){
            throw std::logic_error("Wrong type requested"s);
        }

        return std::get<bool>(value_);
    }

    double Node::AsDouble() const {
        if (!IsDouble()){
            throw std::logic_error("Wrong type requested"s);
        }
        if (IsPureDouble()){
            return std::get<double>(value_);
        }
        return static_cast<double>(std::get<int>(value_));
    }

    const std::string& Node::AsString() const {
        if (!IsString()){
            throw std::logic_error("Wrong type"s);
        }
        return std::get<std::string>(value_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()){
            throw std::logic_error("Wrong type requested"s);
        }

        return std::get<Dict>(value_);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()){
            throw std::logic_error("Wrong type requested"s);
        }

        return std::get<Array>(value_);
    }

    Document::Document(Node root)
        : root_(std::move(root)){}

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(std::istream& input){
        return Document{LoadNode(input)};
    }

    void PrintNode(const Node& node, std::ostream& out);
    void PrintValue(double value, std::ostream& out){
        out << value;
    }

    void PrintValue(int value, std::ostream& out){
        out << value;
    }

    void PrintValue(std::nullptr_t, std::ostream& out){
        out << "null"s;
    }

    void PrintValue(bool value, std::ostream& out){
        out << std::boolalpha;
        out << value;
    }

    void PrintValue(const std::string& value, std::ostream& out){
        out << '\"';
        for (const char c : value){
            switch (c){
                case '\\':
                    out << "\\\\"s;
                    break;
                case '\"':
                    out << "\\\""s;
                    break;
                case '\r':
                    out << "\\r"s;
                    break;
                case '\n':
                    out << "\\n"s;
                    break;
                case '\t':
                    out << "\\t"s;
                    break;
                default:
                    out << c;
                    break;
            }
        }
        out << '\"';
    }

    void PrintValue(const Array& array, std::ostream& out){
        out << '[';
        bool is_next = false;
        for (const auto& obj : array){
            if (is_next){
                out << ", "s;
            }
            is_next = true;
            PrintNode(obj, out);
        }
        out << ']';
    }

    void PrintValue(const Dict& map, std::ostream& out){
        out << '{';
        bool is_next = false;
        for (const auto& [key, value] : map){
            if (is_next){
                out << ", "s;
            }
            PrintValue(key, out);
            out << ": "s;
            PrintNode(value, out);
            is_next = true;
        }
        out << '}';
    }


    void PrintNode(const Node& node, std::ostream& out){
        std::visit(
        [&out](const auto& value){ PrintValue(value, out); },
        node.GetValue());
    }


    void Print(const Document& doc, std::ostream& out){
        PrintNode(doc.GetRoot(), out);
    }
}