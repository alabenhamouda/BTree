#ifndef GUARD_BTREE
#define GUARD_BTREE

#include <stdexcept>
#include <algorithm>
#include "helpers.h"

template <class T>
class BTree{
    private:
        class Node;
        int t;
        Node* root;
    public:
        BTree(int _t): t{_t}, root{NULL}{}
        BTree(Node* tree): root{tree}, t{tree->t}{}
        void insert(T key);
        void traverse(std::ostream& out) const {
            if(!root)
                throw std::logic_error("Tree is empty");
            else 
                root->traverse(out);
        }
};

template <class T>
class BTree<T>::Node{
    friend class BTree;
    public:
        Node(int _t, bool _leaf): t{_t}, leaf{_leaf}, n{0}{
            keys = new T[2*t - 1];
            children = new Node*[2*t];
        }
        Node(Node* from, int b, int e): t{from->t}, leaf{from->leaf}, n{e-b}{
            keys = new T[2*t - 1];
            children = new Node*[2*t];
            std::copy(from->keys + b, from->keys + e, keys);
            std::copy(from->children + b, from->children + e + 1, children);
        }
        ~Node(){
            delete[] keys;
            delete[] children;
        }
        T key(int i) const {
            if(i < n){
                return keys[i];
            }
            else 
                throw std::out_of_range{"Out Of Range"};
        }
        void splitChild(int idx);
        operator bool() const {
            return n < 2*t - 1 && n >= 0;
        }
        void traverse(std::ostream& out) const ;
    private:
        T *keys;
        Node **children;
        int t;
        int n;
        bool leaf;
};

template <typename T>
void BTree<T>::Node::splitChild(int idx) {
    if(idx >= n && idx < 0)
        throw std::out_of_range("Out of range");
    if(n == 2*t - 1)
        throw std::logic_error("Node is full can't add key to it");
    if(!leaf && children[idx]->n == 2*t - 1){
        Node* child = children[idx];
        insertInArr(child->key(child->n / 2), keys, n, idx);
        Node* l = new Node(child, 0, child->n / 2);
        Node* r = new Node(child, child->n / 2 + 1, child->n);
        delete child;
        children[idx] = l;
        insertInArr(r, children, n + 1, idx + 1);
        n++;
    }
    else 
        throw std::logic_error("Node must have children and child must be full");
}

template <typename T>
void BTree<T>::insert(T key){
    if(!root){
        root = new Node(t, true);
        root->keys[0] = key;
        root->n = 1;
        return;
    }
    if(!*root){
        Node* _root = new Node(t, false);
        _root->children[0] = root;
        _root->splitChild(0);
        root = _root;
    }
    if(root->leaf){
        int i;
        for(i = root->n - 1; i >= 0 && root->key(i) > key; i--){
            root->keys[i + 1] = root->keys[i];
        }
        root->keys[i + 1] = key;
        root->n++;
    }
    else {
        int idx = std::lower_bound(root->keys, root->keys + root->n, key) - root->keys;
        if(idx < root->n && root->key(idx) == key)
            throw std::logic_error("key already exists!");
        Node* child = root->children[idx];
        if(!*child){
            root->splitChild(idx);
            if(key < root->key(idx))
                child = root->children[idx];
            else if (key > root->key(idx))
                child = root->children[idx + 1];
            else
                throw std::logic_error("key already exists!");
        }
        BTree tree(child);
        tree.insert(key);
    }
}

template <typename T>
void BTree<T>::Node::traverse(std::ostream& out) const {
    for(int i = 0; i < n; i++){
        if(!leaf){
            children[i]->traverse(out);
        }
        out << keys[i] << " ";
    }
    if(!leaf)
        children[n]->traverse(out);
}

#endif