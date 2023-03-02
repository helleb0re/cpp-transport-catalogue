#include "json_builder.h"

using namespace std::literals;

namespace json
{
    Builder &Builder::Value(Node::Value value)
    {
        if (nodes_stack_.empty())
        {
            if (root_.has_value())
                throw std::logic_error("Value can't be changed"s);

            root_ = std::move(value);
        }
        else if (nodes_stack_.back()->IsString())
        {
            Dict tmp{{std::move(nodes_stack_.back()->AsString()), std::move(value)}};
            nodes_stack_.pop_back();
            tmp.insert(nodes_stack_.back()->AsDict().begin(), nodes_stack_.back()->AsDict().end());
            nodes_stack_.back() = std::make_unique<Node>(std::move(tmp));
        }
        else if (nodes_stack_.back()->IsArray())
        {
            Array tmp{nodes_stack_.back()->AsArray().begin(), nodes_stack_.back()->AsArray().end()};
            nodes_stack_.pop_back();
            tmp.push_back(std::move(value));
            nodes_stack_.push_back(std::make_unique<Node>(std::move(tmp)));
        }
        else
        {
            throw std::logic_error("Value mustn't call after the last method called"s);
        }
        return *this;
    }

    DictItemContext Builder::StartDict()
    {
        if (root_.has_value())
        {
            throw std::logic_error("StartDict can't call after Value and End-point methods \
                 in same hierarchy level"s);
        }
        else if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict())
        {
            throw std::logic_error("StartDict can't call twice in a row"s);
        }

        nodes_stack_.push_back(std::make_unique<Node>(Dict{}));

        return *this;
    }

    ArrayItemContext Builder::StartArray()
    {
        if (root_.has_value())
        {
            throw std::logic_error("StartArray can't call after Value and End-point methods \
                 in same hierarchy level"s);
        }
        else if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict())
        {
            throw std::logic_error("StartArray can't call after StartDict method without key"s);
        }

        nodes_stack_.push_back(std::make_unique<Node>(Array{}));

        return *this;
    }

    Builder &Builder::EndArray()
    {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray())
        {
            throw std::logic_error("StartArray wasn't last uncompleted method"s);
        }

        auto value = (*nodes_stack_.back()).GetValue();
        nodes_stack_.pop_back();
        Value(std::move(value));

        return *this;
    }

    KeyItemContext Builder::Key(std::string key)
    {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict())
        {
            throw std::logic_error("StartDict wasn't last uncompleted method"s);
        }

        nodes_stack_.push_back(std::make_unique<Node>(move(key)));

        return *this;
    }

    Builder &Builder::EndDict()
    {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict())
        {
            throw std::logic_error("StartDict wasn't last uncompleted method"s);
        }

        auto value = (*nodes_stack_.back()).GetValue();
        nodes_stack_.pop_back();
        Value(std::move(value));

        return *this;
    }

    Node Builder::Build() const
    {
        if (!root_.has_value())
        {
            throw std::logic_error("Root node is undefined"s);
        }
        else if (!nodes_stack_.empty())
        {
            throw std::logic_error("Not all Start-point methods were closed by End-point methods"s);
        }

        return root_.value();
    }

    KeyItemContext::KeyItemContext(Builder &builder) : builder_(builder) {}

    DictItemContext KeyItemContext::StartDict()
    {
        return builder_.StartDict();
    }

    ArrayItemContext KeyItemContext::StartArray()
    {
        return builder_.StartArray();
    }

    ValueItemContext KeyItemContext::Value(Node::Value value)
    {
        return builder_.Value(std::move(value));
    }

    ValueItemContext::ValueItemContext(Builder &builder) : builder_(builder) {}

    KeyItemContext ValueItemContext::Key(std::string key)
    {
        return builder_.Key(std::move(key));
    }

    Builder &ValueItemContext::EndDict()
    {
        return builder_.EndDict();
    }

    DictItemContext::DictItemContext(Builder &builder) : builder_(builder) {}

    KeyItemContext DictItemContext::Key(std::string key)
    {
        return builder_.Key(std::move(key));
    }

    Builder &DictItemContext::EndDict()
    {
        return builder_.EndDict();
    }

    ArrayItemContext::ArrayItemContext(Builder &builder) : builder_(builder) {}

    DictItemContext ArrayItemContext::StartDict()
    {
        return builder_.StartDict();
    }

    ArrayItemContext ArrayItemContext::StartArray()
    {
        return builder_.StartArray();
    }

    Builder &ArrayItemContext::EndArray()
    {
        return builder_.EndArray();
    }

    ArrayValueItemContext ArrayItemContext::Value(Node::Value value)
    {
        return builder_.Value(std::move(value));
    }

    ArrayValueItemContext::ArrayValueItemContext(Builder &builder) : builder_(builder) {}

    DictItemContext ArrayValueItemContext::StartDict()
    {
        return builder_.StartDict();
    }

    ArrayItemContext ArrayValueItemContext::StartArray()
    {
        return builder_.StartArray();
    }

    Builder &ArrayValueItemContext::EndArray()
    {
        return builder_.EndArray();
    }

    ArrayValueItemContext ArrayValueItemContext::Value(Node::Value value)
    {
        return builder_.Value(std::move(value));
    }
} // namespace json
