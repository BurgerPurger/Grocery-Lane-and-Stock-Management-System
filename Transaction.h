#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <algorithm>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "Common.h"
#include "Inventory.h"
#include "SearchSort.h"

struct Transaction {
    int id = 0;
    int cartId = 0;
    int lane = 0;
    std::vector<TxnItem> items;
    util::Money total = 0;

    bool operator==(const Transaction& o) const { return id == o.id; }
    bool operator<(const Transaction& o) const { return id < o.id; }
};

util::Money computeTotal(const std::vector<TxnItem>& items);
std::string formatReceipt(const Transaction& t);


class TransactionManager {
public:
    TransactionManager();
    Transaction recordSale(int cartId, int lane, const std::vector<TxnItem>& items);
    bool findById(int id, Transaction& out) const;     
    std::vector<Transaction> history() const;           
    size_t size() const;
    int maxCartId() const;
    void load(const std::vector<Transaction>& txns);

private:
    mutable std::mutex mtx_;
    std::vector<Transaction> history_;                  
    int nextTxnId_;
};

util::Money computeTotal(const std::vector<TxnItem>& items) {
    util::Money sum = 0;
    for (const auto& it : items) sum += it.unitPrice * it.quantity;
    return sum;
}

std::string formatReceipt(const Transaction& t) {
    const std::string bar(46, '-');
    std::ostringstream os;
    os << bar << "\n";
    os << " GROCERY RECEIPT" << util::padLeft("Txn #" + std::to_string(t.id), 30) << "\n";
    os << " Lane " << t.lane << "   Cart " << t.cartId << "\n";
    os << bar << "\n";
    for (const auto& it : t.items) {
        os << " " << util::padRight(it.name, 24) << util::padLeft(std::to_string(it.quantity), 3)
           << " x " << util::padLeft(util::formatMoney(it.unitPrice), 9) << util::padLeft(
                  util::formatMoney(it.unitPrice * it.quantity), 7 + 2) << "\n";
    }
    os << bar << "\n";
    os << " " << util::padRight("TOTAL (PHP)", 26) << util::padLeft(util::formatMoney(t.total), 19)
       << "\n";
    os << bar << "\n";
    return os.str();
}

TransactionManager::TransactionManager() : nextTxnId_(1) {}

Transaction TransactionManager::recordSale(int cartId, int lane,
                                           const std::vector<TxnItem>& items) {
    Transaction t;
    t.cartId = cartId;
    t.lane = lane;
    t.items = items;
    t.total = computeTotal(items);
    std::lock_guard<std::mutex> lock(mtx_);
    t.id = nextTxnId_++;
    history_.push_back(t);        
    return t;
}

bool TransactionManager::findById(int id, Transaction& out) const {
    std::lock_guard<std::mutex> lock(mtx_);
    Transaction probe;
    probe.id = id;
    int idx = binarySearchIndex(history_.data(), static_cast<int>(history_.size()), probe);
    if (idx < 0) return false;
    out = history_[static_cast<size_t>(idx)];
    return true;
}

std::vector<Transaction> TransactionManager::history() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return history_;
}

size_t TransactionManager::size() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return history_.size();
}

int TransactionManager::maxCartId() const {
    std::lock_guard<std::mutex> lock(mtx_);
    int m = 0;
    for (const auto& t : history_) m = std::max(m, t.cartId);
    return m;
}

void TransactionManager::load(const std::vector<Transaction>& txns) {
    std::vector<Transaction> clean;
    for (Transaction t : txns) {
        if (t.id <= 0 || t.items.empty()) continue;
        if (!clean.empty() && t.id <= clean.back().id) continue;  
        t.total = computeTotal(t.items);
        clean.push_back(t);
    }
    std::lock_guard<std::mutex> lock(mtx_);
    history_ = clean;
    nextTxnId_ = history_.empty() ? 1 : history_.back().id + 1;
}

#endif
