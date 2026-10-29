#ifndef QUEUE_LL_H
#define QUEUE_LL_H

#include <iostream>

template <typename T>
class qNode{
    public:
        T data;
        qNode<T>* next;
};

// create the newNode
template <typename T>
qNode<T> *new_node(T newData){
    qNode<T>* newNode = new qNode<T>;
    newNode->data = newData;
    newNode->next = nullptr;
    return newNode;
}

// enqueue operation
template <typename T>
void enqueue(T newData, qNode<T> **frontPtr, qNode<T> **backPtr){
    //create the new node
    qNode<T>* newNode = new_node(newData);

    //Inserting into an empty queue
    if((*frontPtr) == nullptr && (*backPtr) == nullptr){
        (*frontPtr) = newNode;
        (*backPtr) = newNode;
        return;
    }

    //inserting an item into a non empty queue
    (*backPtr)->next = newNode;
    (*backPtr) = newNode;
}

template <typename T>
void dequeue(qNode<T> **frontPtr, qNode<T> **backPtr){

    //create a temporary node
    qNode<T>* deleteNode = nullptr; // temp variable
    deleteNode = (*frontPtr);

    //add a check if the queue is already empty
    if((*frontPtr) == nullptr && (*backPtr) == nullptr){
        std::cout<<"The queue is already empty\n";
        return;          
    }

    //check if the queue has only 1 node
    if((*frontPtr)->next == nullptr && (*backPtr)->next == nullptr){
        (*frontPtr) = nullptr;
        (*backPtr) = nullptr;
        delete deleteNode;
        return;
    }
    //delete the node
    (*frontPtr) = (*frontPtr)->next;
    deleteNode->next = nullptr;
    delete deleteNode;
}

//check the front
template <typename T>
T showFront(qNode<T> **frontPtr){
    if(*frontPtr == nullptr) return T{}; 
    return (*frontPtr)->data;
}

//check the back
template <typename T>
T showBack(qNode<T> **backPtr){
    if(*backPtr == nullptr) return T{};  
    return (*backPtr)->data;
}

//display all
template <typename T>
void displayQ(qNode<T> **frontPtr, qNode<T> **backPtr){
    if (*frontPtr == nullptr) {
        std::cout << "The queue is already empty\n";
        return;
    }

    qNode<T>* current = *frontPtr;

    while (current != nullptr) {
        std::cout << current->data << " ";
        current = current->next;
    }

    std::cout << '\n';
}

#endif
