#include "json_builder.h"


namespace json {
    using namespace std::literals;

    //Builder's helpers

    //BaseContext

    Node BaseContext::Build(){
        return std::move(builder_ -> Build());
    }

    Builder& BaseContext::Value(Node::Value value){
        return builder_ -> Value(std::move(value));
    }

    DictKeyContext BaseContext::StartDict(){
        return std::move(builder_ -> StartDict());
    }

    Builder& BaseContext::EndDict(){
        return builder_ -> EndDict();
    }

    ArrayContext BaseContext::StartArray(){
        return std::move(builder_ -> StartArray());
    }

    Builder& BaseContext::EndArray(){
        return builder_ -> EndArray();
    }

    DictValueContext BaseContext::Key(std::string key){
        return std::move(builder_ -> Key(std::move(key)));
    }

    //DictValueContext

    DictKeyContext DictValueContext::Value(Node::Value value){
        return DictKeyContext{BaseContext::Value(std::move(value))};
    }

    //ArrayContext

    ArrayContext ArrayContext::Value(Node::Value value){
        return ArrayContext{BaseContext::Value(std::move(value))};
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


    Builder& Builder::Value(Node::Value value){
        AddObject(std::move(value), false);
        return *this;
    }

    DictKeyContext Builder::StartDict(){
        AddObject(Dict{}, true);
        return std::move(DictKeyContext{*this});
    }

    ArrayContext Builder::StartArray(){
        AddObject(Array{}, true);
        return std::move(ArrayContext{*this});
    }

    Builder& Builder::EndDict(){
        if (!std::holds_alternative<Dict>(GetCurrentValue())){
            throw std::logic_error("Try to close Dict"s);
        }

        nodes_stack_.pop_back();

        return *this;
    }

    Builder& Builder::EndArray(){
        if (!std::holds_alternative<Array>(GetCurrentValue())){
            throw std::logic_error("Try to close Array"s);
        }

        nodes_stack_.pop_back();
        return *this;
    }

}