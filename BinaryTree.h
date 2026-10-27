#ifndef BINARYTREE_H
#define BINARYTREE_H
#include <iostream>

template<typename T>
class BinaryTree{
    private:
        //Key
        T key;

        //Pointers to left and right childeren nodes
        BinaryTree<T>* leftChild;
        BinaryTree<T>* rightChild;

    public:
        //Constructor
        BinaryTree(T rootObj);

        //Insert Children Insert Left and Right
        void insertLeft(T newNode);
        void insertRight(T newNode);

        // To access the child nodes
        BinaryTree<T>* getLeftChild();
        BinaryTree<T>* getRightChild();

        // get root value & set root node
        T getRootVal();
        void setRootVal(T obj);

        //node checker if leaf
        bool isLeaf();

        // ADDED: re-link a subtree. Needed to detach a category (delete) or move it
        // under a different parent; insertLeft/insertRight can only create new nodes.
        void setLeftChild(BinaryTree<T>* node);
        void setRightChild(BinaryTree<T>* node);
};

template <typename T>
BinaryTree<T>::BinaryTree(T rootObj){
    key = rootObj;
    leftChild = nullptr;
    rightChild = nullptr;
}

template <typename T>
BinaryTree<T> *BinaryTree<T>::getLeftChild(){
    return leftChild;
}

template <typename T>
BinaryTree<T> *BinaryTree<T>::getRightChild(){
    return rightChild;
}

template <typename T>
void BinaryTree<T>::setRootVal(T obj){
    key = obj;
}

template <typename T>
bool BinaryTree<T>::isLeaf(){
    return leftChild == nullptr && rightChild == nullptr;
}

template <typename T>
T BinaryTree<T>::getRootVal(){
    return key;
}

// ADDED
template <typename T>
void BinaryTree<T>::setLeftChild(BinaryTree<T>* node){
    leftChild = node;
}

// ADDED
template <typename T>
void BinaryTree<T>::setRightChild(BinaryTree<T>* node){
    rightChild = node;
}

template <typename T>
void BinaryTree<T>::insertLeft(T newNode){
    //if there is currently no child
    if(leftChild == nullptr){
        leftChild = new BinaryTree<T>(newNode);
    }
        //if we want to insert a certain node
    else{
        //create a new node:
        BinaryTree<T>* newChild = new BinaryTree<T>(newNode);

        //MOVE the existing left child below the new node
        newChild->leftChild = leftChild;

        //make the new node the left child
        leftChild = newChild;
    }
}

template <typename T>
void BinaryTree<T>::insertRight(T newNode){
    //if there is currently no child
    if(rightChild == nullptr){
        rightChild = new BinaryTree<T>(newNode);
    }
        //if we want to insert a certain node
    else{
        //create a new node:
        BinaryTree<T>* newChild = new BinaryTree<T>(newNode);

        //MOVE the existing right child below the new node
        newChild->rightChild = rightChild;

        //make the new node the right child
        rightChild = newChild;
    }
}

// Preorder Traversal: Root -> Left -> Right
template <typename T>
void preorder(BinaryTree<T>* root) {
    if (root != nullptr) {
        std::cout << root->getRootVal() << " ";
        preorder(root->getLeftChild());
        preorder(root->getRightChild());
    }
}

// Inorder Traversal: Left -> Root -> Right
template <typename T>
void inorder(BinaryTree<T>* root) {
    if (root != nullptr) {
        inorder(root->getLeftChild());
        std::cout << root->getRootVal() << " ";
        inorder(root->getRightChild());
    }
}

// Postorder Traversal: Left -> Right -> Root
template <typename T>
void postorder(BinaryTree<T>* root) {
    if (root != nullptr) {
        postorder(root->getLeftChild());
        postorder(root->getRightChild());
        std::cout << root->getRootVal() << " ";
    }
}

template <typename T>
bool findData(BinaryTree<T>* root, int CHOICE, const T& KEY) {
    if (root == nullptr) {
        return false;
    }

    switch (CHOICE) {
        case 1: // Preorder: Root -> Left -> Right
            if (root->getRootVal() == KEY) {
                return true;
            }
            if (findData(root->getLeftChild(), CHOICE, KEY)) {
                return true;
            }
            if (findData(root->getRightChild(), CHOICE, KEY)) {
                return true;
            }
            break;

        case 2: // Inorder: Left -> Root -> Right
            if (findData(root->getLeftChild(), CHOICE, KEY)) {
                return true;
            }
            if (root->getRootVal() == KEY) {
                return true;
            }
            if (findData(root->getRightChild(), CHOICE, KEY)) {
                return true;
            }
            break;

        case 3: // Postorder: Left -> Right -> Root
            if (findData(root->getLeftChild(), CHOICE, KEY)) {
                return true;
            }
            if (findData(root->getRightChild(), CHOICE, KEY)) {
                return true;
            }
            if (root->getRootVal() == KEY) {
                return true;
            }
            break;

        default:
            return false;
    }
    return false;
}

// Display the general tree using the Left-Child Right-Sibling representation
template <typename T>
void displayTree(BinaryTree<T>* node, int level = 0) {
    if (node == nullptr) {
        return;
    }

    // Display the current node
    for (int i = 0; i < level; i++) {
        std::cout << "    ";
    }

    std::cout << node->getRootVal() << std::endl;

    // Display the first child at the next level
    displayTree(node->getLeftChild(), level + 1);

    // Display the next sibling at the same level
    displayTree(node->getRightChild(), level);
}

#endif
