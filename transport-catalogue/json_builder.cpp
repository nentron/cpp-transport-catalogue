#include "json_builder.h"


namespace json {
    using namespace std::literals;

    //Builder's helpers

    // SubContext

    SubContext::SubContext(EndContext context)
        : builder_(std::move(context.builder_)){}

    SubContext::SubContext(DictKeyContext context)
        : builder_(std::move(context.builder_)){}

    SubContext::SubContext(DictValueContext context)
        : builder_(std::move(context.builder_)){}

    SubContext::SubContext(ValueContext context)
        : builder_(std::move(context.builder_)){}

    SubContext::SubContext(End context)
        : builder_(std::move(context.builder_)){}

    Node SubContext::Build(){
        return builder_.Build();
    }

    EndContext SubContext::Value(Node::Value value){
        return builder_.Value(std::move(value));
    }

    DictKeyContext SubContext::StartDict(){
        return builder_.StartDict();
    }

    EndContext SubContext::EndDict(){
        return builder_.EndDict();
    }

    ValueContext SubContext::StartArray(){
        return builder_.StartArray();
    }

    EndContext SubContext::EndArray(){
        return builder_.EndArray();
    }

    DictValueContext SubContext::Key(std::string key){
        return builder_.Key(std::move(key));
    }

    //BaseContext

    Node BaseContext::Build(){
        return std::move(builder_ -> Build());
    }

    EndContext BaseContext::Value(Node::Value value){
        return std::move(builder_ -> AddValue(std::move(value)));
    }

    DictKeyContext BaseContext::StartDict(){
        return std::move(builder_ -> StartDict());
    }

    EndContext BaseContext::EndDict(){
        return std::move(builder_ -> EndDict());
    }

    ValueContext BaseContext::StartArray(){
        return std::move(builder_ -> StartArray());
    }

    EndContext BaseContext::EndArray(){
        return std::move(builder_ -> EndArray());
    }

    DictValueContext BaseContext::Key(std::string key){
        return std::move(builder_ -> Key(std::move(key)));
    }

    //ArrayValueContext

    ValueContext ValueContext::Value(Node::Value value){
        return ValueContext{SubContext::Value(std::move(value))};
    }

    //DictValueContext

    DictKeyContext DictValueContext::Value(Node::Value value){
        return DictKeyContext{SubContext::Value(std::move(value))};
    }

    //Builder

    Node::Value& Builder::GetCurrentValue(){
        if (nodes_stack_.empty()){
            throw std::logic_error("Empty stacks"s);
        }
        return nodes_stack_.back() -> GetValue();
    }

    const Node::Value& Builder::GetCurrentValue() const {
        return const_cast<Builder*>(this) -> GetCurrentValue();
    }

    void Builder::AddObject(Node::Value value, bool is_added){
        Node::Value& host_value = GetCurrentValue();

        if (std::holds_alternative<Array>(host_value)){
            Node& node
                = std::get<Array>(host_value).emplace_back(std::move(value));
            
            if (is_added){
                nodes_stack_.push_back(&node);
            }
        } else {
            if (!std::holds_alternative<std::nullptr_t>(GetCurrentValue())){
                throw std::logic_error("Wrong context"s);
            }

            host_value = std::move(value);
            if (!is_added){
                nodes_stack_.pop_back();
            }
        }
    }

    Node Builder::Build(){
        if (!nodes_stack_.empty()){
            throw std::logic_error("Nodes stacks not empty"s);
        }

        return std::move(root_);
    }

    DictValueContext Builder::Key(std::string key){
        Node::Value& host_value = GetCurrentValue();

        if (!std::holds_alternative<Dict>(host_value)){
            throw std::logic_error("Called in wrong context"s);
        }

        nodes_stack_.push_back(
            &std::get<Dict>(host_value)[std::move(key)]
        );

        return std::move(DictValueContext{*this});
    }

    EndContext Builder::AddValue(Node::Value value){
        AddObject(std::move(value), false);
        return std::move(EndContext{*this});
    }

    End Builder::Value(Node::Value value){
        return std::move(End{AddValue(std::move(value))});
    }

    DictKeyContext Builder::StartDict(){
        AddObject(Dict{}, true);
        return std::move(DictKeyContext{*this});
    }

    ValueContext Builder::StartArray(){
        AddObject(Array{}, true);
        return std::move(ValueContext{*this});
    }

    EndContext Builder::EndDict(){
        if (!std::holds_alternative<Dict>(GetCurrentValue())){
            throw std::logic_error("Try to close Dict"s);
        }

        nodes_stack_.pop_back();

        return std::move(EndContext{*this});
    }

    EndContext Builder::EndArray(){
        if (!std::holds_alternative<Array>(GetCurrentValue())){
            throw std::logic_error("Try to close Array"s);
        }

        nodes_stack_.pop_back();
        return std::move(EndContext{*this});
    }

}