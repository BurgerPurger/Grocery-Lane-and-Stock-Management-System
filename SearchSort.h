#ifndef SEARCHSORT_H
#define SEARCHSORT_H

// Searching helpers (linear search for the inventory list, binary search for the transaction array).

#include "simply_ll.h"

// Linked list inventory
template <typename T, typename Pred>
SimplyList<T>* linearSearchList(SimplyList<T>* first, Pred match){
    for (SimplyList<T>* cur = first; cur != nullptr; cur = cur->next){
        if (match(cur->data)){
            return cur;
        }
    }
    return nullptr;
}

template <typename T>
int binarySearchIndex(const T arr[], int n, const T& key){
    int low = 0, up = n - 1;

    while (low <= up){
        int middle = low + (up - low) / 2;

        if (arr[middle] == key){
            return middle;
        }
        if (arr[middle] < key){
            low = middle + 1;
        }else{
            up = middle - 1;
        }
    }
    return -1;
}

#endif
