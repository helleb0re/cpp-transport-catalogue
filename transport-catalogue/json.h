#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <stdexcept>
#include <variant>
#include <unordered_map>
#include <algorithm>

namespace json
{
    namespace detail
    {
        void PrintWithEscapeSequences(std::string_view text, std::ostream &out);
    } // namespace detail

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class Node
    {
    public:
        using Value = std::variant<std::nullptr_t, int, double, bool, std::string, Array, Dict>;

        Node() = default;

        Node(std::nullptr_t);
        Node(int value);
        Node(double value);
        Node(bool value);
        Node(const std::string &value);
        Node(const Array &value);
        Node(const Dict &value);

        const Value &GetValue() const;

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
        const std::string &AsString() const;
        const Array &AsArray() const;
        const Dict &AsMap() const;
        
    private:
        Value value_;
    };

    bool operator==(const Node &lhs, const Node &rhs);
    bool operator!=(const Node &lhs, const Node &rhs);

    void PrintValue(std::nullptr_t, std::ostream &out);

    // Шаблон, подходящий для вывода double и int
    template <typename Value>
    void PrintValue(const Value &value, std::ostream &out)
    {
        out << value;
    }

    void PrintValue(bool value, std::ostream &out);

    void PrintValue(const std::string &value, std::ostream &out);

    void PrintValue(const Array &value, std::ostream &out);

    void PrintValue(const Dict &value, std::ostream &out);

    void PrintNode(const Node &node, std::ostream &out);

    class Document
    {
    public:
        explicit Document(Node root);

        const Node &GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream &input);

    bool operator==(const Document& lhs, const Document& rhs);
    bool operator!=(const Document& lhs, const Document& rhs);

    void Print(const Document &doc, std::ostream &output);


} // namespace json