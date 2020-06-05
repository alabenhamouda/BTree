#ifndef GUARD_BTREE
#define GUARD_BTREE

#include <stdexcept>
#include <algorithm>
#include "helpers.h"
#include "handle.h"

template <class T>
class BTree{
    private:
        class Node;
        int t;
        Handle<Node> root;
    public:
        BTree(int _t): t{_t}, root{}{}
        BTree(Handle<Node> tree): root{tree}, t{tree->t}{}
        void insert(T key);
        void deleteKey(T key);
        void traverse(std::ostream& out) const {
            if(!root)
                throw std::logic_error("Tree is empty");
            else 
                root->traverse(out);
        }
        const Node* search(T key) const {
            if(!root)
                throw std::logic_error("Tree is empty");
            else
                return root->search(key);
        }
};

template <class T>
class BTree<T>::Node{
    friend class BTree;
    public:
        Node(int _t, bool _leaf): t{_t}, leaf{_leaf}, n{0}{
            keys = new T[2*t - 1];
            children = new Handle<Node>[2*t];
        }
        Node(Handle<Node> from, int b, int e): t{from->t}, leaf{from->leaf}, n{e-b}{
            keys = new T[2*t - 1];
            children = new Handle<Node>[2*t];
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
        const Node* search(T key) const ;
    private:
        T *keys;
        Handle<Node> *children;
        int t;
        int n;
        bool leaf;

        void merge(int idx);
        T removeBiggestKey();
        T removeSmallestKey();
};

template <typename T>
void BTree<T>::Node::splitChild(int idx) {
    if(idx >= n && idx < 0)
        throw std::out_of_range("Out of range");
    if(n == 2*t - 1)
        throw std::logic_error("Node is full can't add key to it");
    if(!leaf && children[idx]->n == 2*t - 1){
        Handle<Node>& child = children[idx];
        insertInArr(child->key(child->n / 2), keys, n, idx);
        Handle<Node> l = new Node(child, 0, child->n / 2);
        Handle<Node> r = new Node(child, child->n / 2 + 1, child->n);
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
        Handle<Node> _root = new Node(t, false);
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
        Handle<Node> child = root->children[idx];
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

template <class T>
void BTree<T>::deleteKey(T key){
    if(!root)
        throw std::logic_error("Tree is empty");
    int idx = std::lower_bound(root->keys, root->keys + root->n, key) - root->keys;
    if(idx < root->n && root->key(idx) == key){
        // key is in root!
        if(root->leaf){
            // root is leaf => delete key directly no extra work needed
            deleteFromArr(root->keys, root->n, idx);
        }
        else { // Have to rearrange children and possibly root
            // check if the child that precedes the key has at least t keys
            if(root->children[idx]->n >= t){
                // find the predecessor of key and delete it recursively
                root->keys[idx] = root->children[idx]->removeBiggestKey();
            } // check if the right child of key has at least t keys
            else if (root->children[idx + 1]->n >= t){
                // find the successor of key and delete it recursively
                root->keys[idx] = root->children[idx + 1]->removeSmallestKey();
            }
            else {
                // two children of key have t-1 keys => merge the key and right child into left child:
                root->merge(idx);
                // delete recursively key from the left child
                BTree subTree(root->children[idx]);
                subTree.deleteKey(key);
                // check if root became empty after merge
                if(root->n == 0)
                    root = root->children[idx];
            }
        } 
    }
    else {
        // we have to search for key in children if root has any
        if(!root->leaf){ // if root dosn't have children the recursion ends
            Handle<Node>& child = root->children[idx];
            // if root->children[idx] has at least t keys we move to it 
            if(child->n >= t){
                BTree subTree(root->children[idx]);
                subTree.deleteKey(key);
            }
            else { // child has only t - 1 keys
                // check left sibling if it has at least t keys
                if(idx > 0 && root->children[idx - 1]->n >= t){
                    Handle<Node>& left = root->children[idx - 1];
                    insertInArr(root->key(idx - 1), child->keys, child->n, 0);
                    insertInArr(left->children[left->n], child->children, child->n + 1, 0);
                    ++child->n; // child has one more key
                    root->keys[idx - 1] = left->key(left->n - 1);
                    left->children[left->n] = Handle<Node>();
                    --left->n;

                    // delete the key from child
                    BTree subTree(child);
                    subTree.deleteKey(key);
                }
                else if (idx < root->n && root->children[idx + 1]->n >= t){ // right sibling has more than t keys
                    Handle<Node>& right = root->children[idx + 1];
                    child->keys[child->n] = root->key(idx);
                    child->children[child->n + 1] = right->children[0];
                    ++child->n; // child has one more key
                    root->keys[idx] = right->key(0);
                    int len = right->n + 1;
                    deleteFromArr(right->keys, right->n, 0);
                    deleteFromArr(right->children, len, 0);
                    right->children[len] = Handle<Node>();

                    // delete the key from child
                    BTree subTree(child);
                    subTree.deleteKey(key);
                }
                else { // sibling(s) have t - 1 keys
                    if(idx < root->n) { // child has right sibling
                        // merge right sibling into child
                        root->merge(idx);
                        // delete the key from child
                        BTree subTree(child);
                        subTree.deleteKey(key);
                        // check if root became empty after merge
                        if (root->n == 0)
                            root = child;
                    }
                    else { // child has no right sibling => merge with the left one
                        // merge child into left sibling
                        root->merge(idx - 1);
                        // delete key from left sibling
                        BTree subTree(root->children[idx - 1]);
                        subTree.deleteKey(key);
                        // check if root became empty after merge
                        if(root->n == 0)
                            root = root->children[idx - 1];
                    }
                }
            }
        }
    }
}

template <class T>
void BTree<T>::Node::merge(int idx){
    if(children[idx]->n != t - 1 || children[idx + 1]->n != t - 1)
        throw std::logic_error("Cannot merge: children must have minimum number of keys");
    Handle<Node>& l = children[idx];
    Handle<Node>& r = children[idx + 1];
    // add the key at index idx to left child
    l->keys[l->n++] = keys[idx];
    // delete the key from keys
    deleteFromArr(keys, n, idx);
    // copy keys from right child to left child
    std::copy(r->keys, r->keys + r->n, l->keys + l->n);
    // copy children from right child to left child
    std::copy(r->children, r->children + r->n + 1, l->children + l->n);
    l->n += r->n;
    int len = n + 2;
    deleteFromArr(children, len, idx + 1);
    children[len] = Handle<Node>();
}

template<typename T>
T BTree<T>::Node::removeBiggestKey(){
    if(leaf) {
        n--;
        return keys[n];
    }
    else {
        Handle<Node>& child = children[n];
        Handle<Node>& left = children[n - 1];
        if(child->n >= t)
            return child->removeBiggestKey();
        else if (left->n >= t){
            insertInArr(keys[n - 1], child->keys, child->n, 0);
            insertInArr(left->children[left->n], child->children, child->n + 1, 0);
            ++child->n;
            keys[n - 1] = left->keys[left->n - 1];
            left->children[left->n] = Handle<Node>();
            --left->n;
            
            // move to the child
            return child->removeBiggestKey();
        }
        else { // child and left both have t - 1 keys
            // merge into left
            merge(n - 1);

            // move to the child
            return children[n]->removeBiggestKey();
        }
    }
}

template <typename T>
T BTree<T>::Node::removeSmallestKey(){
    if(leaf){
        T key = keys[0];
        deleteFromArr(keys, n, 0);
        return key;
    }
    else { // node has children
        Handle<Node>& child = children[0];
        Handle<Node>& right = children[1];
        if(child->n >= t)
            return child->removeSmallestKey();
        else if(right->n >= t) { // child has t-1 keys and right has at least t keys
            child->keys[child->n] = keys[0];
            child->children[child->n + 1] = right->children[0];
            ++child->n;
            keys[0] = right->keys[0];
            int len = right->n + 1;
            deleteFromArr(right->keys, right->n, 0);
            deleteFromArr(right->children, len, 0);
            right->children[len] = Handle<Node>();

            // move to the child
            return child->removeSmallestKey();
        }
        else { // child and right both have t - 1 keys
            merge(0);

            // move to the child
            return children[0]->removeSmallestKey();
        }
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

template <typename T>
const typename BTree<T>::Node* BTree<T>::Node::search(T key) const {
    if(leaf){
        if(std::binary_search(keys, keys + n, key))
            return this;
        else 
            return NULL;
    }
    else {
        int idx = std::lower_bound(keys, keys + n, key) - keys;
        if(idx < n && keys[idx] == key)
            return this;
        else 
            return children[idx]->search(key);
    }
}

#endif