
#ifndef CHECKOUTMANAGER_H
#define CHECKOUTMANAGER_H

#include <atomic>
#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "Cart.h"
#include "Inventory.h"
#include "Transaction.h"

// One std::thread per lane runs laneWorker()

struct CartOutcome{
    util::Money total = 0;
};

class CheckoutManager{
public:
    CheckoutManager(Inventory& inventory, TransactionManager& transactions, int laneCount, int processingDelayMs);
    ~CheckoutManager(); // joins any running lane threads
    CheckoutManager(const CheckoutManager&) = delete;
    CheckoutManager& operator=(const CheckoutManager&) = delete;

    int laneCount() const{
        return static_cast<int>(lanes_.size());
    }
    LaneQueue& lane(int number); // 1-based

    int nextCartId(){
        return nextCartId_++;
    }
    void setNextCartId(int id){
        nextCartId_ = id;
    }

    void enqueueCart(int laneNumber, const Cart& cart);
    bool productQueued(int productId) const;

    // Start one worker thread per listed lane
    // false if lanes are already running
    bool start(const std::vector<int>& laneNumbers);
    void joinAll(); // wait for every worker to finish
    bool active() const{
        return running_.load() > 0;
    }
    std::vector<CartOutcome> takeOutcomes(); // sales completed since the last call
    std::vector<LineItem> queuedItems() const; // items held by carts still waiting

private:
    void laneWorker(int laneNumber);
    void processCart(int laneNumber, const Cart& cart);

    Inventory& inventory_;
    TransactionManager& transactions_;
    int delayMs_;
    std::vector<std::unique_ptr<LaneQueue>> lanes_;
    std::vector<std::thread> threads_;
    std::atomic<int> running_;
    std::atomic<int> nextCartId_;

    mutable std::mutex outMtx_;
    std::vector<CartOutcome> outcomes_;
};

CheckoutManager::CheckoutManager(Inventory& inventory, TransactionManager& transactions, int laneCount, int processingDelayMs) :
    inventory_(inventory),
    transactions_(transactions),
    delayMs_(processingDelayMs),
    running_(0),
    nextCartId_(101){

    if (laneCount < 1){
        laneCount = 1;
    }
    for (int i = 1; i <= laneCount; ++i){
        lanes_.emplace_back(new LaneQueue(i));
    }
}
CheckoutManager::~CheckoutManager(){
    joinAll();
}
LaneQueue& CheckoutManager::lane(int number){
    return *lanes_.at(static_cast<size_t>(number - 1));
}
void CheckoutManager::enqueueCart(int laneNumber, const Cart& cart){
    lane(laneNumber).enqueueCart(cart);
}

bool CheckoutManager::productQueued(int productId) const{
    for (const auto& q : lanes_){
        for (const auto& c : q->snapshot()){
            for (const auto& line : c.items){
                if (line.productId == productId){
                    return true;
                }
            }
        }
    }
    return false;
}

bool CheckoutManager::start(const std::vector<int>& laneNumbers){
    if (running_.load() > 0 || !threads_.empty()){
        return false;
    }
    for (int n : laneNumbers){
        if (n < 1 || n > laneCount()){
            continue;
        }
        ++running_;
        threads_.emplace_back(&CheckoutManager::laneWorker, this, n);
    }
    return true;
}

void CheckoutManager::joinAll(){
    for (auto& t : threads_){
        if (t.joinable()){
            t.join();
        }
    }
    threads_.clear();
}

std::vector<CartOutcome> CheckoutManager::takeOutcomes(){
    std::lock_guard<std::mutex> lock(outMtx_);
    std::vector<CartOutcome> out;
    out.swap(outcomes_);

    return out;
}

std::vector<LineItem> CheckoutManager::queuedItems() const{
    std::vector<LineItem> out;

    for (const auto& q : lanes_){
        for (const auto& cart : q->snapshot()){
            out.insert(out.end(), cart.items.begin(), cart.items.end());
        }
    }
    return out;
}

// worker thread
void CheckoutManager::laneWorker(int laneNumber){
    try{
        Cart cart;

        while (lane(laneNumber).dequeueCart(cart)){
            processCart(laneNumber, cart);
        }
    }catch (const std::exception& e){
        std::lock_guard<std::mutex> io(util::ioMutex());
        std::cerr << "[Lane " << laneNumber << "] stopped by an error: " << e.what() << "\n";
    }
    --running_;
}

void CheckoutManager::processCart(int laneNumber, const Cart& cart){
    if (delayMs_ > 0){
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs_));
    }
    // records the sale
    Transaction t = transactions_.recordSale(cart.id, laneNumber, cart.items);

    CartOutcome out;
    out.total = t.total;

    std::ostringstream os;
    os << "[Lane " << laneNumber << "] Cart " << cart.id << " paid -> Transaction #" << t.id << "\n" << formatReceipt(t);
    std::string text = os.str();
    {
        std::lock_guard<std::mutex> lock(outMtx_);
        outcomes_.push_back(out);
    }
    {
        std::lock_guard<std::mutex> io(util::ioMutex());
        std::cout << text << std::flush;
    }
}

#endif