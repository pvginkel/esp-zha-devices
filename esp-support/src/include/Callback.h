#pragma once

#include <functional>

#include "Queue.h"

template <typename Arg>
class Callback {
    struct Node {
        std::function<void(Arg)> func;
        Node* next;

        Node(const std::function<void(Arg)>& func, Node* next) : func(func), next(next) {}
    };

    Node* _node;

public:
    Callback() : _node(nullptr) {}

    ~Callback() {
        while (_node) {
            auto node = _node;
            _node = node->next;
            delete node;
        }
    }

    void add(const std::function<void(Arg)>& func) { _node = new Node(func, _node); }

    bool call(Arg arg) {
        auto node = _node;
        if (!node) {
            return false;
        }

        while (node) {
            node->func(arg);
            node = node->next;
        }

        return true;
    }

    template <typename T = Arg, std::enable_if_t<std::is_trivially_copyable_v<T>, int> = 0>
    void queue(Queue* queue, Arg arg) {
        queue->enqueue([this, arg] { call(arg); });
    }
};

template <>
class Callback<void> {
    struct Node {
        std::function<void()> func;
        Node* next;

        Node(const std::function<void()>& func, Node* next) : func(func), next(next) {}
    };

    Node* _node;

public:
    Callback() : _node(nullptr) {}

    ~Callback() {
        while (_node) {
            auto node = _node;
            _node = node->next;
            delete node;
        }
    }

    void add(const std::function<void()>& func) { _node = new Node(func, _node); }

    bool call() {
        auto node = _node;
        if (!node) {
            return false;
        }

        while (node) {
            node->func();
            node = node->next;
        }

        return true;
    }

    void queue(Queue* queue) {
        queue->enqueue([this] { call(); });
    }
};
