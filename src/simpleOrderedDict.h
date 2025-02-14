#pragma once

#include <unordered_map>
#include <utility>
#include <stdexcept>

using namespace std;

template <typename K, typename T>
class SimpleOrderedDict {


    struct Node {
        K key;
        T value;
        Node * next;
        Node * prev;
    };

    Node * head;
    Node * tail;
    unordered_map<K, Node *> keyToNode;

  public:

    SimpleOrderedDict() {
        head = nullptr; tail = nullptr;
    };

    ~SimpleOrderedDict() {
        Node * cur = head;
        while (cur != nullptr) {
            Node * tmp = cur->next;
            delete cur;
            cur = tmp;
        }
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
            keyToNode.at(key)->value = move(newObject);
            return;
        }

        Node * newNode = new Node{key, move(newObject), nullptr, nullptr};
        keyToNode[key] = newNode;

        if (head == nullptr && tail == nullptr) {
            head = newNode;
            tail = newNode;
        } else {
            newNode->prev = tail;
            tail->next = newNode;
            tail = newNode;
        }
    };

    void remove(K key) {
        if (!keyToNode.contains(key)) {
            return;
        }

        Node * nodeToRemove = keyToNode.at(key);
        Node * prev = nodeToRemove->prev;
        Node * next = nodeToRemove->next;
        if (prev != nullptr) {
            prev->next = next;
        } else {
            // must be the head if it has no previous
            head = nodeToRemove->next;
        }
        if (next != nullptr) {
            next->prev = prev;
        } else {
            // must be the tail if it has no next
            tail = nodeToRemove->prev;
        }


        delete nodeToRemove;
        keyToNode.erase(key);

    };

    K getCircularNextKey(K key) const {
        Node * next = keyToNode.at(key)->next;
        if (next == nullptr) {
            return head->key;
        }
        return next->key;
    }

    K lastKey() const {
        if (tail == nullptr) {
            throw out_of_range("Cannot get last key of empty dict");
        }
        return tail->key;
    }

    T& operator[](K key) {
        return keyToNode[key]->value;
    };

    T& operator[](K key) const {
        return keyToNode.at(key)->value;
    };

    size_t size() const {
        return keyToNode.size();
    }

    // throws if k is not in range
    T& getKth(size_t k) const {
        if (k >= size()) {
            throw out_of_range("index k out of range");
        }

        Node * cur = head;
        while (k > 0) {
            cur = cur->next;
            k--;
        }
        return cur->value;
    }

};