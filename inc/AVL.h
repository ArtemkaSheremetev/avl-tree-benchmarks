#pragma once

#include <string>
#include <memory>

struct Node {
    int key;
    std::string value;
    int height;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
    
    Node(int k, const std::string& v) 
        : key(k), value(v), height(1), left(nullptr), right(nullptr) {}
};

class AVL {
private:
    std::unique_ptr<Node> root;

    int getHeight(const std::unique_ptr<Node>& node) const;
    int getBalance(const std::unique_ptr<Node>& node) const;

    void updateHeight(std::unique_ptr<Node>& node);
    
    std::unique_ptr<Node> rotateRight(std::unique_ptr<Node> y);
    std::unique_ptr<Node> rotateLeft(std::unique_ptr<Node> x);
    
    std::unique_ptr<Node>& findMin(std::unique_ptr<Node>& node);

    void insertHelper(std::unique_ptr<Node>& node, int key, const std::string& value);
    bool removeHelper(std::unique_ptr<Node>& node, int key);

public:
    AVL() = default;
    
    void insert(int key, const std::string& value);
    bool remove(int key);
    bool find(int key, std::string& value) const;
    
    bool contains(int key) const;
    bool isEmpty() const;
};
