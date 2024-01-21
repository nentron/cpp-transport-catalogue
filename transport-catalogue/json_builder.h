#pragma once

#include "json.h"

#include <string>
#include <vector>


namespace json {
    class Builder;

    class DictKeyContext;
    class ValueContext;
    class DictValueContext;
    class EndContext;
    class End;

    class BaseContext {
    private:
        Builder* builder_;
    public:
        BaseContext(): builder_(nullptr){}

        BaseContext(BaseContext&& base)
            : builder_(base.builder_){
        }
        BaseContext(const BaseContext& base)
            : builder_(base.builder_){}

        BaseContext(Builder& builder)
            : builder_(&builder){}

        Node Build();

        EndContext Value(Node::Value value);

        DictKeyContext StartDict();

        EndContext EndDict();

        ValueContext StartArray();

        EndContext EndArray();

        DictValueContext Key(std::string key);
    };

    class SubContext: BaseContext{
    private:
        BaseContext builder_;
    public:
        explicit SubContext(const BaseContext& context)
            : builder_(context)
        {
        }

        explicit SubContext(Builder& builder)
            : builder_(BaseContext{builder}){}

        explicit SubContext(DictKeyContext context);
        explicit SubContext(DictValueContext context);
        explicit SubContext(ValueContext context);
        explicit SubContext(EndContext context);
        explicit SubContext(End context);

        Node Build();

        EndContext Value(Node::Value value);

        DictKeyContext StartDict();

        EndContext EndDict();

        ValueContext StartArray();

        EndContext EndArray();

        DictValueContext Key(std::string key);
    };

    class DictKeyContext: public SubContext{
    public:
        using SubContext::SubContext;

        Node Build() = delete;
        EndContext Value(Node::Value value) = delete;
        DictKeyContext StartDict() = delete;
        ValueContext StartArray() = delete;
        EndContext EndArray() = delete;
    };


    class DictValueContext: public SubContext{
    public:
        using SubContext::SubContext;

        DictKeyContext Value(Node::Value value);
        Node Build() = delete;
        EndContext EndDict() = delete;
        EndContext EndArray() = delete;
        ValueContext Key(std::string key) = delete;
    };


    class ValueContext: public SubContext{
    public:
        using SubContext::SubContext;
        ValueContext Value(Node::Value value);
        Node Build() = delete;
        EndContext EndDict() = delete;
        ValueContext Key(std::string key) = delete;
    };


    class EndContext: public SubContext{
    public:
        using SubContext::SubContext;
    };

    class End: public SubContext{
    public:
        using SubContext::SubContext;

        EndContext Value(Node::Value value) = delete;

        DictKeyContext StartDict() = delete;

        EndContext EndDict() = delete;

        ValueContext StartArray() = delete;

        EndContext EndArray() = delete;

        ValueContext Key(std::string key) = delete;
    };


    class Builder{
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

        EndContext AddValue(Node::Value value);

        End Value(Node::Value value);

        DictKeyContext StartDict();

        EndContext EndDict();

        ValueContext StartArray();

        EndContext EndArray();

        DictValueContext Key(std::string key);
    };
}