#include "json.h"

namespace json
{

    namespace
    {
        using namespace std;

        Node LoadNode(istream &input);

        bool LoadBool(istream &input)
        {
            string res = ""s;
            char c;
            for (; input >> c && !isspace(c) && !ispunct(c);)
                res += c;

            input.putback(c);
            if (res != "true"s && res != "false"s)
                throw ParsingError("Boolean data couldn't be read");

            return res == "true"s;
        }

        Node *LoadNull(istream &input)
        {
            string res = ""s;
            char c;
            for (int i = 0; input >> c && i < 4; ++i)
                res += c;

            if (res != "null"s)
                throw ParsingError("Null couldn't be read");

            return nullptr;
        }

        using Number = std::variant<int, double>;

        Number LoadNumber(istream &input)
        {
            string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input]
            {
                parsed_num += static_cast<char>(input.get());
                if (!input)
                {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char]
            {
                if (!isdigit(input.peek()))
                {
                    throw ParsingError("A digit is expected"s);
                }
                while (isdigit(input.peek()))
                {
                    read_char();
                }
            };

            if (input.peek() == '-')
            {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0')
            {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else
            {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.')
            {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E')
            {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-')
                {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try
            {
                if (is_int)
                {
                    // Сначала пробуем преобразовать строку в int
                    try
                    {
                        return stoi(parsed_num);
                    }
                    catch (...)
                    {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return stod(parsed_num);
            }
            catch (...)
            {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        string LoadString(istream &input)
        {
            auto it = istreambuf_iterator<char>(input);
            auto end = istreambuf_iterator<char>();
            string s;
            while (true)
            {
                if (it == end)
                {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"')
                {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\')
                {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end)
                    {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char)
                    {
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
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r')
                {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else
                {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Array LoadArray(istream &input)
        {
            Array result;

            char c;
            for (; input >> c && c != ']';)
            {
                if (c != ',')
                {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            if (c != ']')
                throw ParsingError("A closing brace ] was expected"s);

            return result;
        }

        Dict LoadDict(istream &input)
        {
            Dict result;
            char c;
            for (; input >> c && c != '}';)
            {
                if (c == ',')
                {
                    input >> c;
                }

                string key = LoadString(input);

                while (input >> c && isspace(c))
                {
                }
                result.insert({move(key), LoadNode(input)});
            }

            if (c != '}')
                throw ParsingError("A closing brace } was expected"s);

            return result;
        }

        Node LoadNode(istream &input)
        {
            for (char c; input >> c;)
            {
                if (c == '[')
                {
                    return {LoadArray(input)};
                }
                else if (c == ']')
                {
                    throw ParsingError("A opening brace [ was expected");
                }
                else if (c == '{')
                {
                    return {LoadDict(input)};
                }
                else if (c == '}')
                {
                    throw ParsingError("A opening brace { was expected");
                }
                else if (c == '"')
                {
                    return {LoadString(input)};
                }
                else if (c == 't' || c == 'f')
                {
                    input.putback(c);
                    return {LoadBool(input)};
                }
                else if (c == 'n')
                {
                    input.putback(c);
                    LoadNull(input);
                    return {nullptr};
                }
                else if (isdigit(c) || c == '-')
                {
                    input.putback(c);
                    auto res = LoadNumber(input);
                    if (holds_alternative<int>(res))
                        return {get<int>(res)};
                    return {get<double>(res)};
                }
            }
            throw ParsingError("File isn't a JSON"s);
        }

    } // namespace

    Node::Node(nullptr_t) : value_(nullptr) {}
    Node::Node(int value) : value_(value) {}
    Node::Node(double value) : value_(value) {}
    Node::Node(bool value) : value_(value) {}
    Node::Node(const std::string &value) : value_(value) {}
    Node::Node(const Array &value) : value_(value) {}
    Node::Node(const Dict &value) : value_(value) {}

    const Node::Value &Node::GetValue() const { return value_; }

    bool Node::IsInt() const
    {
        return holds_alternative<int>(value_);
    }

    bool Node::IsPureDouble() const
    {
        return holds_alternative<double>(value_);
    }

    bool Node::IsDouble() const
    {
        return IsInt() || IsPureDouble();
    }

    bool Node::IsBool() const
    {
        return holds_alternative<bool>(value_);
    }

    bool Node::IsString() const
    {
        return holds_alternative<string>(value_);
    }

    bool Node::IsNull() const
    {
        return holds_alternative<nullptr_t>(value_);
    }

    bool Node::IsArray() const
    {
        return holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const
    {
        return holds_alternative<Dict>(value_);
    }

    int Node::AsInt() const
    {
        if (!IsInt())
            throw logic_error("Node value isn't a integer");
        return get<int>(value_);
    }

    bool Node::AsBool() const
    {
        if (!IsBool())
            throw logic_error("Node value isn't a boolean");
        return get<bool>(value_);
    }

    double Node::AsDouble() const
    {
        if (!IsDouble())
            throw logic_error("Node value isn't a integer or a double");
        return static_cast<double>((IsInt() ? get<int>(value_) : get<double>(value_)));
    }

    const string &Node::AsString() const
    {
        if (!IsString())
            throw logic_error("Node value isn't a string");
        return get<string>(value_);
    }

    const Array &Node::AsArray() const
    {
        if (!IsArray())
            throw logic_error("Node value isn't an array");
        return get<Array>(value_);
    }

    const Dict &Node::AsMap() const
    {
        if (!IsMap())
            throw logic_error("Node value isn't a map");
        return get<Dict>(value_);
    }

    bool operator==(const Node &lhs, const Node &rhs)
    {
        if (lhs.IsInt() && rhs.IsInt())
            return lhs.AsInt() == rhs.AsInt();
        else if (lhs.IsPureDouble() && rhs.IsPureDouble())
            return lhs.AsDouble() == rhs.AsDouble();
        else if (lhs.IsBool() && rhs.IsBool())
            return lhs.AsBool() == rhs.AsBool();
        else if (lhs.IsNull() && rhs.IsNull())
            return true;
        else if (lhs.IsString() && rhs.IsString())
            return lhs.AsString() == rhs.AsString();
        else if (lhs.IsArray() && rhs.IsArray())
            return lhs.AsArray() == rhs.AsArray();
        else if (lhs.IsMap() && rhs.IsMap())
            return lhs.AsMap() == rhs.AsMap();

        return false;
    }

    bool operator!=(const Node &lhs, const Node &rhs)
    {
        return !(lhs == rhs);
    }

    Document::Document(Node root)
        : root_(move(root))
    {
    }

    const Node &Document::GetRoot() const
    {
        return root_;
    }

    bool operator==(const Document &lhs, const Document &rhs)
    {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator!=(const Document &lhs, const Document &rhs)
    {
        return !(lhs.GetRoot() == rhs.GetRoot());
    }

    Document Load(istream &input)
    {
        return Document{LoadNode(input)};
    }

    void PrintValue(nullptr_t, ostream &out)
    {
        out << "null"sv;
    }

    void PrintValue(bool value, ostream &out)
    {
        out << boolalpha << value;
    }

    namespace detail
    {
        void PrintWithEscapeSequences(string_view text, ostream &out)
        {
            const std::unordered_map<char, string_view> symbol_to_change_ =
                {
                    {'"', "\\\""sv},
                    {'\\', "\\\\"sv},
                    {'\n', "\\n"sv},
                    {'\r', "\\r"s}};

            for_each(text.begin(), text.end(), [&symbol_to_change_, &out](char c)
                     {
                        if (symbol_to_change_.count(c) > 0) {
                            out << symbol_to_change_.at(c);
                        } else {
                            out << c;
                        } });
            ;
        }
    } // namespace detail

    void PrintValue(const string &value, ostream &out)
    {
        out << "\"";
        detail::PrintWithEscapeSequences(value, out);
        out << "\"";
    }

    void PrintValue(const Array &value, std::ostream &out)
    {
        out << "["sv;
        for (size_t i = 0; i < value.size(); ++i)
        {
            PrintNode(value[i], out);
            out << (i == value.size() - 1 ? ""sv : ","sv);
        }
        out << "]"sv;
    }

    void PrintValue(const Dict &value, std::ostream &out)
    {
        out << "{"sv;
        for (size_t i = 0; i < value.size(); ++i)
        {
            auto &[key, node] = *next(value.begin(), i);
            PrintValue(key, out);
            out << ":"sv;
            PrintNode(node, out);
            out << (i == value.size() - 1 ? ""sv : ","sv);
        }
        out << "}"sv;
    }

    void PrintNode(const Node &node, ostream &out)
    {
        visit(
            [&out](const auto &value)
            { PrintValue(value, out); },
            node.GetValue());
    }

    void Print(const Document &doc, ostream &output)
    {
        PrintNode(doc.GetRoot(), output);
    }

} // namespace json