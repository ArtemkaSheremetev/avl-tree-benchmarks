#include "AVL.h"
#include <algorithm>

inline int AVL::getHeight(const std::unique_ptr<Node>& node) const {
    return node ? node->height : 0;
}

inline int AVL::getBalance(const std::unique_ptr<Node>& node) const {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

void AVL::updateHeight(std::unique_ptr<Node>& node) {
    if (node) {
        node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
    }
}

std::unique_ptr<Node> AVL::rotateRight(std::unique_ptr<Node> y) {
    auto x = std::move(y->left);
    y->left = std::move(x->right);
    x->right = std::move(y);
    
    updateHeight(x->right);
    updateHeight(x);
    return x;
}

std::unique_ptr<Node> AVL::rotateLeft(std::unique_ptr<Node> x) {
    auto y = std::move(x->right);
    x->right = std::move(y->left);
    y->left = std::move(x);
    
    updateHeight(y->left);
    updateHeight(y);
    return y;
}

std::unique_ptr<Node>& AVL::findMin(std::unique_ptr<Node>& node) {
    if (!node->left) return node;
    return findMin(node->left);
}

void AVL::insertHelper(std::unique_ptr<Node>& node, int key, const std::string& value) {
    if (!node) {
        node = std::make_unique<Node>(key, value);
        return;
    }
    
    if (key < node->key) {
        insertHelper(node->left, key, value);
    } else if (key > node->key) {
        insertHelper(node->right, key, value);
    } else {
        node->value = value;
        return;
    }
    
    updateHeight(node);
    int balance = getBalance(node);
    
    // ЛЕВО-ЛЕВО
    if (balance > 1 && key < node->left->key) {
        node = rotateRight(std::move(node));
        return;
    }
    // ПРАВО-ПРАВО
    if (balance < -1 && key > node->right->key) {
        node = rotateLeft(std::move(node));
        return;
    }
    // ЛЕВО-ПРАВО
    if (balance > 1 && key > node->left->key) {
        node->left = rotateLeft(std::move(node->left));
        node = rotateRight(std::move(node));
        return;
    }
    // ПРАВО-ЛЕВО
    if (balance < -1 && key < node->right->key) {
        node->right = rotateRight(std::move(node->right));
        node = rotateLeft(std::move(node));
        return;
    }
}

bool AVL::removeHelper(std::unique_ptr<Node>& node, int key) {
    if (!node) return false;
    
    bool found = false;
    
    if (key < node->key) {
        found = removeHelper(node->left, key);
    } else if (key > node->key) {
        found = removeHelper(node->right, key);
    } else {
        found = true;
        if (!node->left || !node->right) {
            // 0 или 1 потомоккк
            node = std::move(node->left ? node->left : node->right);
        } else {
            // 2 потомкааа
            auto& minNode = findMin(node->right);
            node->key = minNode->key;
            node->value = minNode->value;
            removeHelper(node->right, minNode->key);
        }
    }
    
    if (!found || !node) return found;
    
    updateHeight(node);
    int balance = getBalance(node);
    
    if (balance > 1 && getBalance(node->left) >= 0) {
        node = rotateRight(std::move(node));
    } else if (balance < -1 && getBalance(node->right) <= 0) {
        node = rotateLeft(std::move(node));
    } else if (balance > 1 && getBalance(node->left) < 0) {
        node->left = rotateLeft(std::move(node->left));
        node = rotateRight(std::move(node));
    } else if (balance < -1 && getBalance(node->right) > 0) {
        node->right = rotateRight(std::move(node->right));
        node = rotateLeft(std::move(node));
    }
    
    return found;
}

/////////////////////////////////////////////////////////////////////////////////публичка

void AVL::insert(int key, const std::string& value) {
    insertHelper(root, key, value);
}

bool AVL::remove(int key) {
    return removeHelper(root, key);
}

bool AVL::find(int key, std::string& value) const {
    auto* current = root.get();
    while (current) {
        if (key == current->key) {
            value = current->value;
            return true;
        }
        current = (key < current->key) ? current->left.get() : current->right.get();
    }
    return false;
}

bool AVL::contains(int key) const {
    std::string dummy;
    return find(key, dummy);
}

bool AVL::isEmpty() const {
    return !root;
}