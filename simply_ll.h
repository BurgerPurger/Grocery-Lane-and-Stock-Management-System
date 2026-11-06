#ifndef SIMPLY_LL_H
#define SIMPLY_LL_H

#include <iostream>   

template <typename T>
class SimplyList{
    public:
        T data;
        SimplyList<T> *next = nullptr;
};

template <typename T>
void sllinsertHead(T newData, SimplyList<T>** currentHead){
    // 1. Allocae memory for the new node
    SimplyList<T>* newNode = new SimplyList<T>;
    //2. Put our data into the new node
    newNode -> data = newData;
    //3. Set Next of the new node to the point to the previous head
    newNode -> next = *currentHead;
    //4. Reset Head to point to the new node
    *currentHead = newNode;

}

template <typename T>
void sllGeneralInsert(T newData, SimplyList<T>* prevNode){
    if(prevNode == nullptr){
        std::cout<< "Previous Node Cannot Be Null\n";
        return;
    }
    SimplyList<T>* newNode = new SimplyList<T>;

    newNode->data = newData;
    newNode->next = prevNode->next;

    prevNode->next = newNode;
}

template <typename T>
void sllDeleteNode(T findData, SimplyList<T>** head){
    if(*head == nullptr) return;

    SimplyList<T>* currNode = *head;
    SimplyList<T>* prevNode = nullptr;

    while(currNode != nullptr && currNode->data != findData){
        prevNode = currNode;
        currNode = currNode -> next;
    }

    if(currNode == nullptr) return;

    if(prevNode == nullptr){
        *head = currNode->next;
    } else {
        prevNode -> next = currNode -> next;
    }

    delete currNode;
}

template <typename T>
void sllDeleteList(SimplyList<T>** head){
    SimplyList<T>* current = *head;

    while(current != nullptr){
        SimplyList<T>* temp = current;
        current = current -> next;

        delete temp;
    }
    *head = nullptr;
}

#endif
