
#ifndef CART_H
#define CART_H

#include <mutex>
#include <ostream>
#include <string>
#include <vector>

#include "Common.h"
#include "Inventory.h"
#include "queue_ll.h"

// Carts are created by the customer simulator with products that are already on the shelf
class Cart{
public:
    int id = 0;
    std::vector<CartItem> items;

    void addItem(int productId, const std::string& name, int quantity, util::Money unitPrice){
        items.push_back(CartItem{productId, name, quantity, unitPrice});
    }

    util::Money total() const;

    bool empty() const{
        return items.empty();
    }
};

class LaneQueue{
public:
    explicit LaneQueue(int laneId = 0);
    ~LaneQueue();
    LaneQueue(const LaneQueue&) = delete;
    LaneQueue& operator=(const LaneQueue&) = delete;

    void enqueueCart(const Cart& cart);
    bool dequeueCart(Cart& out); // false when empty
    bool empty() const;
    std::vector<Cart> snapshot() const; // traversal front -> rear, queue unchanged
    void display(std::ostream& os) const; // "Front -> Cart 101 -> Cart 104 -> Rear"

private:
    int lane_;
    qNode<Cart>* front_;
    qNode<Cart>* back_;
    mutable std::mutex mtx_;
};

util::Money Cart::total() const{
    util::Money sum = 0;

    for (const auto& it : items){
        sum += it.unitPrice * it.quantity;
    }

    return sum;
}

LaneQueue::LaneQueue(int laneId) : lane_(laneId), front_(nullptr), back_(nullptr){}

LaneQueue::~LaneQueue(){
    while (front_ != nullptr){
        dequeue(&front_, &back_);
    }
}

void LaneQueue::enqueueCart(const Cart& cart){
    std::lock_guard<std::mutex> lock(mtx_);
    enqueue(cart, &front_, &back_);
}

bool LaneQueue::dequeueCart(Cart& out){
    std::lock_guard<std::mutex> lock(mtx_);

    if (front_ == nullptr){
        return false;
    }

    out = showFront(&front_);
    dequeue(&front_, &back_);

    return true;
}

bool LaneQueue::empty() const{
    std::lock_guard<std::mutex> lock(mtx_);
    return front_ == nullptr;
}

std::vector<Cart> LaneQueue::snapshot() const{
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<Cart> out;

    for (qNode<Cart>* cur = front_; cur != nullptr; cur = cur->next){
        out.push_back(cur->data);
    }

    return out;
}

void LaneQueue::display(std::ostream& os) const{
    std::vector<Cart> carts = snapshot();

    os << "Lane " << lane_ << ": ";

    if (carts.empty()){
        os << "(empty)\n";
        return;
    }

    os << "Front";

    for (const auto& c : carts){
        os << " -> Cart " << c.id << " [" << c.items.size() << " line(s), PHP " << util::formatMoney(c.total()) << "]";
    }

    os << " -> Rear\n";
}

#endif
