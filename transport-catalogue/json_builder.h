#pragma once

#include <vector>
#include <stdexcept>
#include <memory>
#include <optional>

#include "json.h"

namespace json
{
    class DictItemContext;
    class ArrayItemContext;
    class KeyItemContext;
    class ArrayValueItemContext;
    class ValueItemContext;

    class Builder
    {
    private:
        std::optional<Node> root_;
        std::vector<std::unique_ptr<Node>> nodes_stack_;

    public:
        Builder() = default;

        Builder &Value(Node::Value value);

        DictItemContext StartDict();

        ArrayItemContext StartArray();

        Builder &EndArray();

        KeyItemContext Key(std::string key);

        Builder &EndDict();

        Node Build() const;
    };

    class KeyItemContext : Builder
    {
    private:
        Builder &builder_;

    public:
        KeyItemContext(Builder &builder);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        ValueItemContext Value(Node::Value value);
    };

    class ValueItemContext : Builder
    {
    private:
        Builder &builder_;

    public:
        ValueItemContext(Builder &builder);
        KeyItemContext Key(std::string key);
        Builder &EndDict();
    };

    class DictItemContext : Builder
    {
    private:
        Builder &builder_;

    public:
        DictItemContext(Builder &builder);
        KeyItemContext Key(std::string key);
        Builder &EndDict();
    };

    class ArrayItemContext : Builder
    {
    private:
        Builder &builder_;

    public:
        ArrayItemContext(Builder &builder);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder &EndArray();
        ArrayValueItemContext Value(Node::Value value);
    };

    class ArrayValueItemContext : Builder
    {
    private:
        Builder &builder_;

    public:
        ArrayValueItemContext(Builder &builder);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder &EndArray();
        ArrayValueItemContext Value(Node::Value value);
    };

} // namespace json
