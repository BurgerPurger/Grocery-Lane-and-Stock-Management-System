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
void listTraversal(SimplyList<T>* head){

    if(head == nullptr){
        std::cout<< "List is Empty\n";
        return;
    }
    while(head != nullptr){
        std::cout<< head -> data;
        if(head->next != nullptr){
            std::cout<<"->";
        }
        head = head -> next;
    }
}

template <typename T>
void sllinsertHead(T newData, SimplyList<T>** currentHead){
    SimplyList<T>* newNode = new SimplyList<T>;
    newNode -> data = newData;
    newNode -> next = *currentHead;
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
void sllInsertEnd(T newData, SimplyList<T>** head){
    SimplyList<T>* newNode = new SimplyList<T>;
    SimplyList<T>* currentNode = *head;
    newNode -> data = newData;
    newNode -> next = nullptr;
    while(currentNode->next != nullptr){
        currentNode = currentNode->next;
    }
    currentNode -> next = newNode;
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
