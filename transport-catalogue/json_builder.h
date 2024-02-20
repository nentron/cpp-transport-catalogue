#pragma once

#include "json.h"

#include <string>
#include <vector>


namespace json {

    class Builder{
    public:
        class DictKeyContext;
        class ArrayContext;
        class DictValueContext;
    private:
        Node root_;
        std::vector<Node*> nodes_stack_;

        void AddObject(Node::Value value, bool is_added);

        Node::Value& GetCurrentValue();

        const Node::Value& GetCurrentValue() const;
    public:
        Builder()
            : root_()
            , nodes_stack_{&root_}
        {}

        Node Build();

        Builder& Value(Node::Value value);

        DictKeyContext StartDict();

        Builder& EndDict();

        ArrayContext StartArray();

        Builder& EndArray();

        DictValueContext Key(std::string key);

        class BaseContext {
        private:
            Builder& builder_;
        public:
            BaseContext(Builder& builder)
                : builder_(builder){}

            BaseContext(BaseContext&& base)
                : builder_(base.builder_){
            }

            BaseContext(const BaseContext& base)
                : builder_(base.builder_){}

            Node Build();

            Builder& Value(Node::Value value);

            DictKeyContext StartDict();

            Builder& EndDict();

            ArrayContext StartArray();

            Builder& EndArray();

            DictValueContext Key(std::string key);
        };

        class DictKeyContext: public BaseContext{
        public:
            using BaseContext::BaseContext;

            Node Build() = delete;
            Builder& Value(Node::Value value) = delete;
            DictKeyContext StartDict() = delete;
            ArrayContext StartArray() = delete;
            Builder& EndArray() = delete;
        };


        class DictValueContext: public BaseContext {
        public:
            using BaseContext::BaseContext;

            DictKeyContext Value(Node::Value value);
            Node Build() = delete;
            Builder& EndDict() = delete;
            Builder& EndArray() = delete;
            DictValueContext Key(std::string key) = delete;
        };


        class ArrayContext: public BaseContext{
        public:
            using BaseContext::BaseContext;
            ArrayContext Value(Node::Value value);
            Node Build() = delete;
            Builder& EndDict() = delete;
            ArrayContext Key(std::string key) = delete;
        };
    };
}