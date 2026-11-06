#ifndef INVENTORY_H
#define INVENTORY_H

// Products and inventory (module 1).

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "Common.h"
#include "simply_ll.h"
#include "SearchSort.h"

// Purchased item with saved price and name.
struct LineItem{
    int productId = 0;
    std::string name;
    int quantity = 0;
    util::Money unitPrice = 0;
};

using CartItem = LineItem;
using TxnItem = LineItem;

// Product record.
struct Product{
    int id = 0;
    std::string name;
    int categoryId = 0;
    util::Money price = 0;
    int quantity = 0;
    int storage = 0;

    bool operator==(const Product& o) const{
        return id == o.id;
    }

    bool operator!=(const Product& o) const{
        return id != o.id;
    }
};

struct PurchaseResult{
    bool ok = false;
    std::vector<std::string> problems;
};

// Thread-safe inventory using a singly linked list.
class Inventory{
public:
    Inventory();
    ~Inventory();

    Inventory(const Inventory&) = delete;
    Inventory& operator=(const Inventory&) = delete;

    // Catalogue.
    util::Result addProduct(const std::string& name, int categoryId, util::Money price, int quantity, int storage = 0);
    util::Result update(int id, const std::string& name, int categoryId, util::Money price);
    util::Result remove(int id);
    bool get(int id, Product& out) const;
    std::vector<Product> snapshot() const;
    std::vector<Product> findByName(const std::string& fragment) const;
    std::map<int, int> categoryCounts() const;
    size_t size() const;

    // Stock.
    util::Result receiveStock(int id, int amount);
    util::Result moveToShelf(int id, int amount);
    util::Result setStock(int id, int quantity);
    PurchaseResult commitPurchase(const std::vector<LineItem>& items);

    // Persistence.
    void load(const std::vector<Product>& products);

private:
    SimplyList<Product>* findLocked(int id) const;
    void appendLocked(const Product& p);

    mutable std::mutex mtx_;
    SimplyList<Product>* first_;
    SimplyList<Product>* last_;
    int nextId_;
};

using util::failure;
using util::Result;
using util::success;

namespace{

const int kMaxStock = 1000000;
const util::Money kMaxPrice = 100000000LL;

// Validates product fields.
Result validateFields(const std::string& name, util::Money price, int qty, int storage = 0){
    if(name.empty() || name.size() > 60){
        return failure("Name must be 1-60 characters.");
    }

    if(name.find_first_of("|\r\n") != std::string::npos){
        return failure("Name cannot contain '|' or line breaks.");
    }

    if(price <= 0){
        return failure("Price must be greater than zero.");
    }

    if(price > kMaxPrice){
        return failure("Price is unreasonably large.");
    }

    if(qty < 0){
        return failure("Quantity cannot be negative.");
    }

    if(qty > kMaxStock){
        return failure("Quantity is unreasonably large.");
    }

    if(storage < 0 || storage > kMaxStock){
        return failure("Storehouse quantity out of range.");
    }

    return success();
}

}

Inventory::Inventory() : first_(nullptr), last_(nullptr), nextId_(1){}

Inventory::~Inventory(){
    sllDeleteList(&first_);
}

// Private helpers.
SimplyList<Product>* Inventory::findLocked(int id) const{
    return linearSearchList(first_, [id](const Product& p){ return p.id == id; });
}

void Inventory::appendLocked(const Product& p){
    if(first_ == nullptr){
        sllinsertHead(p, &first_);
        last_ = first_;
    }
    else{
        sllGeneralInsert(p, last_);
        last_ = last_->next;
    }
}

// Catalogue.
Result Inventory::addProduct(const std::string& name, int categoryId, util::Money price, int quantity, int storage){
    Result v = validateFields(name, price, quantity, storage);

    if(!v.ok){
        return v;
    }

    std::lock_guard<std::mutex> lock(mtx_);

    Product p;
    p.id = nextId_++;
    p.name = name;
    p.categoryId = categoryId;
    p.price = price;
    p.quantity = quantity;
    p.storage = storage;

    appendLocked(p);
    return success("Product added.", p.id);
}

Result Inventory::update(int id, const std::string& name, int categoryId, util::Money price){
    std::lock_guard<std::mutex> lock(mtx_);
    SimplyList<Product>* node = findLocked(id);

    if(!node){
        return failure("No product with ID " + std::to_string(id) + ".");
    }

    Result v = validateFields(name, price, node->data.quantity, node->data.storage);

    if(!v.ok){
        return v;
    }

    node->data.name = name;
    node->data.categoryId = categoryId;
    node->data.price = price;

    return success("Product updated.", id);
}

Result Inventory::remove(int id){
    std::lock_guard<std::mutex> lock(mtx_);
    SimplyList<Product>* node = findLocked(id);

    if(!node){
        return failure("No product with ID " + std::to_string(id) + ".");
    }

    if(node->data.quantity > 0 || node->data.storage > 0){
        return failure("Product still has " + std::to_string(node->data.quantity) +
                       " unit(s) on the shelf and " + std::to_string(node->data.storage) +
                       " in the storehouse. Set both counts to 0 first (write-off) if it "
                       "really must be removed.");
    }

    Product probe;
    probe.id = id;
    sllDeleteNode(probe, &first_);

    last_ = first_;

    while(last_ != nullptr && last_->next != nullptr){
        last_ = last_->next;
    }

    return success("Product removed.", id);
}

bool Inventory::get(int id, Product& out) const{
    std::lock_guard<std::mutex> lock(mtx_);
    SimplyList<Product>* node = findLocked(id);

    if(!node){
        return false;
    }

    out = node->data;
    return true;
}

std::vector<Product> Inventory::snapshot() const{
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<Product> out;

    for(SimplyList<Product>* cur = first_; cur != nullptr; cur = cur->next){
        out.push_back(cur->data);
    }

    return out;
}

std::vector<Product> Inventory::findByName(const std::string& fragment) const{
    std::string needle = util::toLower(fragment);
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<Product> out;

    for(SimplyList<Product>* cur = first_; cur != nullptr; cur = cur->next){
        if(util::toLower(cur->data.name).find(needle) != std::string::npos){
            out.push_back(cur->data);
        }
    }

    return out;
}

std::map<int, int> Inventory::categoryCounts() const{
    std::lock_guard<std::mutex> lock(mtx_);
    std::map<int, int> counts;

    for(SimplyList<Product>* cur = first_; cur != nullptr; cur = cur->next){
        ++counts[cur->data.categoryId];
    }

    return counts;
}

size_t Inventory::size() const{
    std::lock_guard<std::mutex> lock(mtx_);
    size_t n = 0;

    for(SimplyList<Product>* cur = first_; cur != nullptr; cur = cur->next){
        ++n;
    }

    return n;
}

// Stock.
Result Inventory::receiveStock(int id, int amount){
    if(amount <= 0){
        return failure("Delivery amount must be positive.");
    }

    std::lock_guard<std::mutex> lock(mtx_);
    SimplyList<Product>* node = findLocked(id);

    if(!node){
        return failure("No product with ID " + std::to_string(id) + ".");
    }

    if(static_cast<long long>(node->data.storage) + amount > kMaxStock){
        return failure("That would exceed the maximum storehouse level.");
    }

    node->data.storage += amount;
    return success("Storehouse now holds " + std::to_string(node->data.storage) + ".", id);
}

Result Inventory::moveToShelf(int id, int amount){
    if(amount <= 0){
        return failure("Amount must be positive.");
    }

    std::lock_guard<std::mutex> lock(mtx_);
    SimplyList<Product>* node = findLocked(id);

    if(!node){
        return failure("No product with ID " + std::to_string(id) + ".");
    }

    if(amount > node->data.storage){
        return failure("The storehouse only holds " + std::to_string(node->data.storage) +
                       " unit(s) of " + node->data.name + ".");
    }

    if(static_cast<long long>(node->data.quantity) + amount > kMaxStock){
        return failure("That would exceed the maximum shelf level.");
    }

    node->data.storage -= amount;
    node->data.quantity += amount;

    return success("Shelf now holds " + std::to_string(node->data.quantity) +
                   ", storehouse " + std::to_string(node->data.storage) + ".", id);
}

Result Inventory::setStock(int id, int quantity){
    if(quantity < 0 || quantity > kMaxStock){
        return failure("Quantity out of range.");
    }

    std::lock_guard<std::mutex> lock(mtx_);
    SimplyList<Product>* node = findLocked(id);

    if(!node){
        return failure("No product with ID " + std::to_string(id) + ".");
    }

    // The difference goes to or comes from the storehouse
    long long change = static_cast<long long>(quantity) - node->data.quantity;
    long long newStorage = static_cast<long long>(node->data.storage) - change;

    if(newStorage < 0){
        return failure("The storehouse only has " + std::to_string(node->data.storage) + " to move to the shelf.");
    }
    if(newStorage > kMaxStock){
        return failure("The storehouse cannot hold that many.");
    }

    node->data.quantity = quantity;
    node->data.storage = static_cast<int>(newStorage);

    return success("Shelf now holds " + std::to_string(node->data.quantity) +
                   ", storehouse " + std::to_string(node->data.storage) + ".", id);
}

PurchaseResult Inventory::commitPurchase(const std::vector<LineItem>& items){
    PurchaseResult result;

    if(items.empty()){
        result.problems.push_back("The cart is empty.");
        return result;
    }

    std::map<int, int> needed;

    for(const auto& it : items){
        if(it.quantity <= 0){
            result.problems.push_back("Invalid quantity for product #" +
                                      std::to_string(it.productId) + ".");
        }
        else if(it.unitPrice <= 0){
            result.problems.push_back("Invalid price for product #" +
                                      std::to_string(it.productId) + ".");
        }
        else{
            needed[it.productId] += it.quantity;
        }
    }

    if(!result.problems.empty()){
        return result;
    }

    // Validate and deduct under one lock.
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<std::pair<SimplyList<Product>*, int>> targets;

    for(const auto& entry : needed){
        SimplyList<Product>* node = findLocked(entry.first);

        if(!node){
            result.problems.push_back("Product #" + std::to_string(entry.first) +
                                      " no longer exists.");
        }
        else if(node->data.quantity < entry.second){
            result.problems.push_back(node->data.name + ": requested " +
                                      std::to_string(entry.second) + ", only " +
                                      std::to_string(node->data.quantity) + " available.");
        }
        else{
            targets.emplace_back(node, entry.second);
        }
    }

    if(!result.problems.empty()){
        return result;
    }

    for(auto& t : targets){
        t.first->data.quantity -= t.second;
    }

    result.ok = true;
    return result;
}

// Persistence.
void Inventory::load(const std::vector<Product>& products){
    std::lock_guard<std::mutex> lock(mtx_);

    sllDeleteList(&first_);
    last_ = nullptr;

    int maxId = 0;

    for(const auto& p : products){
        Result v = validateFields(p.name, p.price, p.quantity, p.storage);

        if(!v.ok || p.id <= 0){
            continue;
        }

        if(findLocked(p.id)){
            continue;
        }

        appendLocked(p);

        if(p.id > maxId){
            maxId = p.id;
        }
    }

    nextId_ = maxId + 1;
}

#endif
