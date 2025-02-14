#pragma once

#include <unordered_map>
#include <utility>

using namespace std;

template <typename K, typename T>
class SimpleOrderedDict {


    struct Node {
        T value;
        Node * next;
    };

    Node * head;
    Node * tail;
    unordered_map<K, Node *> keyToNode;

  public:

    SimpleOrderedDict() {
        head = nullptr; tail = nullptr;
    };

    ~SimpleOrderedDict() {

    };

    SimpleOrderedDict(const SimpleOrderedDict&) = delete;
    SimpleOrderedDict& operator=(const SimpleOrderedDict&) = delete;
    SimpleOrderedDict(SimpleOrderedDict&&) = delete;
    SimpleOrderedDict& operator=(SimpleOrderedDict&&) = delete;

    bool contains(K key) const {
        return keyToNode.contains(key);
    }

    void upsert(K key, T&& newObject) {
        if (keyToNode.contains(key)) {
            keyToNode[key]->value = move(newObject);
            return;
        }

        Node * newNode = new Node{move(newObject), nullptr};
        keyToNode[key] = newNode;

        if (head == nullptr && tail == nullptr) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    };

    void remove(K key) {
        if (!keyToNode.contains(key)) {
            return;
        }

        // TODO: implement this
    };

    // throws if K not found
    T& operator[](K key) {

    };

};