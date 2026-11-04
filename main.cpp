// Main menu and application controller.

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <random>
#include <string>

#include "CategoryTree.h"
#include "CheckoutManager.h"
#include "Common.h"
#include "FileManager.h"
#include "Inventory.h"
#include "Reports.h"
#include "Transaction.h"

using util::readInt;
using util::readLine;

namespace{

struct Config{
    std::string dataDir = "data";
    int lanes = 3;
    int delayMs = 150; // simulated scanning/payment time per customer
    int customers = -1; // customers per session; -1 = random (4-10)
    long long seed = -1; // -1 = different every run
};

void heading(const std::string& title){
    std::cout << "\n" << std::string(44, '=') << "\n " << title << "\n" << std::string(44, '=') << "\n";
}

void say(const util::Result& r){
    std::cout << (r.ok ? "  OK: " : "  Not done: ") << r.message << "\n";
}

}

class App{
public:
    explicit App(const Config& cfg) :
        cfg_(cfg),
        rng_(cfg.seed >= 0 ? static_cast<unsigned>(cfg.seed) : std::random_device{}()),
        checkout_(inventory_, transactions_, cfg.lanes, cfg.delayMs){}

    int run(){
        startup();
        mainMenu();
        shutdown();
        return 0;
    }

private:
    // startup / shutdown
    std::string path(const std::string& file) const{
        return cfg_.dataDir + "/" + file;
    }

    void startup(){
        std::string err;

        if (!FileManager::ensureDirectory(cfg_.dataDir, err)){
            std::cout << err << "\n";
        }

        std::vector<Product> products;
        bool haveProducts = FileManager::loadProducts(path("products.txt"), products);
        std::vector<CategoryRecord> records;
        bool haveCategories = FileManager::loadCategories(path("categories.txt"), records);
        std::vector<Transaction> txns;
        FileManager::loadTransactions(path("transactions.txt"), txns);

        if (haveCategories && !records.empty()){
            categories_.build(records);
        }else{
            categories_.seedDefault();
        }

        // unknown categories fall back to the root
        for (auto& p : products){
            if (!categories_.exists(p.categoryId)){
                p.categoryId = categories_.rootId();
            }
        }

        inventory_.load(products);
        transactions_.load(txns);
        checkout_.setNextCartId(std::max(100, transactions_.maxCartId()) + 1);

        std::cout << "Loaded " << inventory_.size() << " product(s), " << categories_.size()
                  << " categor" << (categories_.size() == 1 ? "y" : "ies") << ", "
                  << transactions_.size() << " transaction(s) from '" << cfg_.dataDir << "'.\n";

        if (!haveProducts){
            std::cout << "No product file yet - starting with an empty inventory.\n";
        }
        generateCustomers(cfg_.customers);
    }

    void shutdown(){
        checkout_.joinAll();
        std::cout << "\nSaving data...\n";
        saveAll();
    }

    // Items held by carts still waiting in a lane are added back to the saved stock,
    // because those carts are not saved.
    bool saveAll(){
        if (checkout_.active()){
            std::cout << "  Cannot save while checkout lanes are running.\n";
            return false;
        }

        std::vector<Product> products = inventory_.snapshot();

        for (const auto& held : checkout_.queuedItems()){
            for (auto& p : products){
                if (p.id == held.productId){
                    p.quantity += held.quantity;
                    break;
                }
            }
        }

        std::string err;
        return FileManager::saveProducts(path("products.txt"), products, err) &&
               FileManager::saveCategories(path("categories.txt"), categories_.records(), err) &&
               FileManager::saveTransactions(path("transactions.txt"), transactions_.history(), err);
    }

    // main menu
    void mainMenu(){
        while (!util::inputClosed()){
            std::cout << "\n========================================\n"
                         " GROCERY INVENTORY AND CHECKOUT SYSTEM\n"
                         "========================================\n\n"
                         "1. Manage Products\n"
                         "2. Manage Categories\n"
                         "3. Storehouse and Shelf Restocking\n"
                         "4. Process Checkout Lanes\n"
                         "5. View Transaction History\n"
                         "6. View Sales Report\n"
                         "0. Exit (data is saved automatically)\n\n";

            long long choice;

            if (!readInt("Enter choice: ", 0, 6, choice)){
                if (util::inputClosed()){
                    return;
                }
                continue;
            }

            switch (choice){
                case 1: productMenu(); break;
                case 2: categoryMenu(); break;
                case 3: storehouseMenu(); break;
                case 4: laneMenu(); break;
                case 5: historyMenu(); break;
                case 6: Reports::salesReport(std::cout, inventory_, categories_, transactions_); break;
                case 0: return;
            }
        }
    }

    // shared prompts
    void showCategoryTree(){
        categories_.display(std::cout, inventory_.categoryCounts());
    }

    bool askCategory(const std::string& prompt, int& out, bool showTree = true){
        if (showTree){
            showCategoryTree();
        }
        long long id;

        while (true){
            if (!readInt(prompt, 1, 10000, id)){
                return false;
            }
            if (categories_.exists(static_cast<int>(id))){
                out = static_cast<int>(id);
                return true;
            }
            std::cout << "  No category with ID " << id << ".\n";
        }
    }

    bool askProduct(const std::string& prompt, Product& out){
        long long id;

        if (!readInt(prompt, 1, 2000000000, id)){
            return false;
        }
        if (!inventory_.get(static_cast<int>(id), out)){
            std::cout << "  No product with ID " << id << ".\n";
            return false;
        }
        return true;
    }

    // 1. products
    void productMenu(){
        while (!util::inputClosed()){
            heading("MANAGE PRODUCTS");
            std::cout << "1. Add a product\n2. Display all products\n3. Search for a product (by ID or name)\n"
                         "4. Update product details\n5. Remove a product\n"
                         "6. Adjust shelf stock (set exact count)\n"
                         "7. Check available quantity\n0. Back\n\n";

            long long c;

            if (!readInt("Enter choice: ", 0, 7, c) || c == 0){
                return;
            }

            switch (c){
                case 1: addProduct(); break;
                case 2: Reports::productTable(std::cout, inventory_.snapshot(), categories_); break;
                case 3: searchProduct(); break;
                case 4: updateProduct(); break;
                case 5: removeProduct(); break;
                case 6: adjustStock(); break;
                case 7: {
                    Product p;

                    if (askProduct("Product ID: ", p)){
                        std::cout << "  " << p.name << ": " << p.quantity << " on the shelf, "
                                  << p.storage << " in the storehouse.\n";
                    }
                    break;
                }
            }
        }
    }

    // A number is looked up as a product ID first; anything else is searched as part of a name.
    void searchProduct(){
        std::string s;

        if (!readLine("Product ID or part of its name: ", s)){
            return;
        }
        long long id;
        Product p;

        if (util::parseInt(s, id) && id > 0 && id <= 2000000000 && inventory_.get(static_cast<int>(id), p)){
            Reports::productTable(std::cout, {p}, categories_);
            return;
        }
        Reports::productTable(std::cout, inventory_.findByName(s), categories_);
    }

    void addProduct(){
        std::cout << "  (type /c at any prompt to cancel)\n";

        std::string name;
        util::Money price;
        long long qty, store;
        int cat;

        if (!readLine("Name: ", name)){
            return;
        }
        if (!askCategory("Category ID: ", cat)){
            return;
        }
        if (!util::readMoney("Price (PHP): ", price)){
            return;
        }
        if (!readInt("Starting quantity on the shelf: ", 0, 1000, qty)){
            return;
        }
        if (!readInt("Starting quantity in the storehouse: ", 0, 10000, store)){
            return;
        }

        util::Result r = inventory_.addProduct(name, cat, price, static_cast<int>(qty), static_cast<int>(store));
        say(r);

        if (r.ok){
            std::cout << "  New product ID: " << r.id << "\n";
            saveAll();
        }
    }

    void updateProduct(){
        Product p;

        if (!askProduct("Product ID to update: ", p)){
            return;
        }

        std::cout << "  Press Enter to keep the current value.\n";

        std::string s;
        std::string name = p.name;
        util::Money price = p.price;
        int cat = p.categoryId;

        if (!readLine("Name [" + p.name + "]: ", s, true)){
            return;
        }
        if (!s.empty()){
            name = s;
        }

        if (!readLine("Price [" + util::formatMoney(p.price) + "]: ", s, true)){
            return;
        }
        if (!s.empty()){
            if (!util::parseMoney(s, price)){
                std::cout << "  Invalid price.\n";
                return;
            }
        }

        if (!readLine("Category ID [" + std::to_string(p.categoryId) + " " + categories_.nameOf(p.categoryId) + "] (? to list): ", s, true)){
            return;
        }
        if (s == "?"){
            if (!askCategory("Category ID: ", cat)){
                return;
            }
        }else if (!s.empty()){
            long long v;

            if (!util::parseInt(s, v) || !categories_.exists(static_cast<int>(v))){
                std::cout << "  No such category.\n";
                return;
            }
            cat = static_cast<int>(v);
        }

        util::Result r = inventory_.update(p.id, name, cat, price);
        say(r);

        if (r.ok){
            saveAll();
        }
    }

    void removeProduct(){
        Product p;

        if (!askProduct("Product ID to remove: ", p)){
            return;
        }
        if (checkout_.productQueued(p.id)){
            std::cout << "  Not done: the product is in a cart that is waiting in a checkout lane.\n";
            return;
        }
        if (!util::confirm("Remove '" + p.name + "'")){
            return;
        }

        util::Result r = inventory_.remove(p.id);
        say(r);

        if (r.ok){
            saveAll();
        }
    }

    void adjustStock(){
        Product p;
        long long qty;

        if (!askProduct("Product ID: ", p)){
            return;
        }

        std::cout << "  Current shelf count for " << p.name << ": " << p.quantity << "\n";

        if (!readInt("Counted quantity on the shelf: ", 0, 1000, qty)){
            return;
        }

        util::Result r = inventory_.setStock(p.id, static_cast<int>(qty));
        say(r);

        if (r.ok){
            saveAll();
        }
    }

    // 2. categories
    void categoryMenu(){
        while (!util::inputClosed()){
            heading("MANAGE CATEGORIES");
            std::cout << "1. Add a category\n2. Assign a parent (move a category)\n3. Rename a category\n"
                         "4. Search for a category\n5. Display the category tree\n"
                         "6. Delete a category\n0. Back\n\n";

            long long c;

            if (!readInt("Enter choice: ", 0, 6, c) || c == 0){
                return;
            }

            switch (c){
                case 1: {
                    std::string name;
                    int parent;

                    if (!readLine("New category name: ", name)){
                        break;
                    }
                    if (!askCategory("Parent category ID: ", parent)){
                        break;
                    }

                    util::Result r = categories_.addCategory(name, parent);
                    say(r);

                    if (r.ok){
                        std::cout << "  New category ID: " << r.id << "\n";
                        saveAll();
                    }
                    break;
                }
                case 2: {
                    int id, parent;

                    if (!askCategory("Category to move (ID): ", id)){
                        break;
                    }
                    if (!askCategory("New parent (ID): ", parent, false)){
                        break;
                    }

                    util::Result r = categories_.reparent(id, parent);
                    say(r);

                    if (r.ok){
                        saveAll();
                    }
                    break;
                }
                case 3: {
                    int id;
                    std::string name;

                    if (!askCategory("Category to rename (ID): ", id)){
                        break;
                    }
                    if (!readLine("New name: ", name)){
                        break;
                    }

                    util::Result r = categories_.rename(id, name);
                    say(r);

                    if (r.ok){
                        saveAll();
                    }
                    break;
                }
                case 4: searchCategory(); break;
                case 5: showCategoryTree(); break;
                case 6: {
                    int id;

                    if (!askCategory("Category to delete (ID): ", id)){
                        break;
                    }

                    int direct = 0;
                    auto counts = inventory_.categoryCounts();

                    if (counts.count(id)){
                        direct = counts[id];
                    }

                    util::Result r = categories_.remove(id, direct);
                    say(r);

                    if (r.ok){
                        saveAll();
                    }
                    break;
                }
            }
        }
    }

    void searchCategory(){
        std::string s;

        if (!readLine("Category ID or part of its name: ", s)){
            return;
        }
        long long id;

        if (util::parseInt(s, id)){
            CategoryNode n;
            int parent = 0;

            if (categories_.get(static_cast<int>(id), n, &parent)){
                std::cout << "  [" << n.id << "] " << n.name << "  (parent: "
                          << (parent ? categories_.nameOf(parent) : std::string("none"))
                          << ", sub-categories: " << categories_.childCount(n.id) << ")\n";
                return;
            }
        }

        auto hits = categories_.searchByName(s);

        if (hits.empty()){
            std::cout << "  No matching category.\n";
        }
        for (const auto& h : hits){
            std::cout << "  " << h << "\n";
        }
    }

    // 3. storehouse
    // Deliveries go into the storehouse. Customers can only buy what is on the shelf,
    // so employees have to move stock from the storehouse onto the shelves.
    void storehouseMenu(){
        while (!util::inputClosed()){
            heading("STOREHOUSE AND SHELF RESTOCKING");
            std::cout << "1. Fill storehouse (receive a delivery)\n"
                         "2. View shelf and storehouse stock\n"
                         "3. Employee: move stock from the storehouse to a shelf\n0. Back\n\n";

            long long c;

            if (!readInt("Enter choice: ", 0, 3, c) || c == 0){
                return;
            }

            switch (c){
                case 1: receiveDelivery(); break;
                case 2: Reports::productTable(std::cout, inventory_.snapshot(), categories_); break;
                case 3: moveToShelf(); break;
            }
        }
    }

    void receiveDelivery(){
        std::cout << "  Enter the delivery, one product at a time. (type /c at a prompt to cancel)\n";

        std::map<int, int> lines; // productId -> units delivered

        while (true){
            long long id, amount;

            if (!readInt("Product ID (0 = delivery complete): ", 0, 2000000000, id)){
                std::cout << "  Delivery cancelled.\n";
                return;
            }
            if (id == 0){
                break;
            }

            Product p;

            if (!inventory_.get(static_cast<int>(id), p)){
                std::cout << "  No product with ID " << id << ".\n";
                continue;
            }
            if (!readInt("Units delivered of " + p.name + ": ", 1, 10000, amount)){
                continue;
            }
            lines[p.id] += static_cast<int>(amount);
        }

        if (lines.empty()){
            std::cout << "  Nothing was delivered.\n";
            return;
        }

        std::cout << "\n  Delivery note:\n";

        for (const auto& l : lines){
            Product p;
            inventory_.get(l.first, p);
            std::cout << "    " << util::padRight(std::to_string(l.first), 5) << util::padRight(p.name, 30)
                      << util::padLeft(std::to_string(l.second), 8) << "\n";
        }

        if (!util::confirm("Book this delivery into the storehouse")){
            return;
        }

        for (const auto& l : lines){
            Product p;
            inventory_.get(l.first, p);
            util::Result r = inventory_.receiveStock(l.first, l.second);
            std::cout << "  " << p.name << ": " << (r.ok ? "" : "NOT booked - ") << r.message << "\n";
        }
        saveAll();
    }

    void moveToShelf(){
        Product p;
        long long amount;

        if (!askProduct("Product ID: ", p)){
            return;
        }

        std::cout << "  " << p.name << ": " << p.quantity << " on the shelf, " << p.storage << " in the storehouse.\n";

        if (p.storage == 0){
            std::cout << "  Nothing in the storehouse to move.\n";
            return;
        }
        if (!readInt("Units to put on the shelf (1-" + std::to_string(p.storage) + "): ", 1, p.storage, amount)){
            return;
        }

        util::Result r = inventory_.moveToShelf(p.id, static_cast<int>(amount));
        say(r);

        if (r.ok){
            saveAll();
        }
    }

    // customers (automatic)
    // Each customer gets a random basket from what is on the shelves right now.
    // The stock is taken as the items go into the cart, then the cart joins a random lane.
    // count < 0 = a random number of customers (4-10).
    void generateCustomers(int count = -1){
        std::mt19937& rng = rng_;
        auto pick = [&rng](int lo, int hi){ return std::uniform_int_distribution<int>(lo, hi)(rng); };

        std::vector<Product> onShelf;

        for (const auto& p : inventory_.snapshot()){
            if (p.quantity > 0){
                onShelf.push_back(p);
            }
        }

        int customers = count >= 0 ? count : pick(4, 10);

        if (onShelf.empty() || customers == 0){
            std::cout << "No customers arrived";

            if (onShelf.empty()){
                std::cout << " - the shelves are empty, put stock from the storehouse on the shelves first";
            }
            std::cout << ".\n";
            return;
        }

        int arrived = 0;

        for (int i = 0; i < customers; ++i){
            Cart cart;
            cart.id = checkout_.nextCartId();
            std::shuffle(onShelf.begin(), onShelf.end(), rng);

            int lines = pick(1, std::min(5, static_cast<int>(onShelf.size())));

            for (int k = 0; k < lines; ++k){
                Product p;

                if (!inventory_.get(onShelf[static_cast<size_t>(k)].id, p) || p.quantity <= 0){
                    continue;
                }

                int qty = pick(1, std::min(5, p.quantity));
                LineItem line{p.id, p.name, qty, p.price};

                if (!inventory_.commitPurchase({line}).ok){
                    continue;
                }
                cart.addItem(p.id, p.name, qty, p.price);
            }

            if (cart.empty()){
                continue;
            }
            checkout_.enqueueCart(pick(1, checkout_.laneCount()), cart);
            ++arrived;
        }
        std::cout << arrived << " customer(s) arrived and joined the checkout lanes.\n";
    }

    // 4. lanes
    void showLanes(){
        for (int i = 1; i <= checkout_.laneCount(); ++i){
            checkout_.lane(i).display(std::cout);
        }
    }

    void laneMenu(){
        while (!util::inputClosed()){
            heading("PROCESS CHECKOUT LANES");
            std::cout << "1. Show lane queues\n2. Simulate customers (new customers join the lanes)\n"
                         "3. Process ALL lanes (one thread per lane)\n0. Back\n\n";

            long long c;

            if (!readInt("Enter choice: ", 0, 3, c) || c == 0){
                return;
            }

            if (c == 1){
                showLanes();
            }else if (c == 2){
                long long n;

                if (!readInt("How many customers? (0 = random 4-10): ", 0, 200, n)){
                    continue;
                }
                generateCustomers(n == 0 ? -1 : static_cast<int>(n));
                showLanes();
            }else{
                processAllLanes();
            }
        }
    }

    void processAllLanes(){
        std::vector<int> busy;

        for (int n = 1; n <= checkout_.laneCount(); ++n){
            if (!checkout_.lane(n).empty()){
                busy.push_back(n);
            }
        }

        if (busy.empty()){
            std::cout << "  No carts are waiting. Use \"Simulate customers\" to bring in more.\n";
            return;
        }

        std::cout << "\nOpening " << busy.size() << " lane(s)...\n\n";

        checkout_.takeOutcomes();
        checkout_.start(busy);
        checkout_.joinAll();

        std::vector<CartOutcome> outcomes = checkout_.takeOutcomes();
        util::Money sales = 0;

        for (const auto& o : outcomes){
            sales += o.total;
        }

        std::cout << "\nAll lanes finished: " << outcomes.size() << " sale(s), PHP " << util::formatMoney(sales) << ".\n";
        saveAll();
    }

    // 5. history
    void historyMenu(){
        while (!util::inputClosed()){
            heading("TRANSACTION HISTORY");
            std::cout << "1. List all transactions\n2. View a receipt by transaction ID\n0. Back\n\n";

            long long c;

            if (!readInt("Enter choice: ", 0, 2, c) || c == 0){
                return;
            }

            if (c == 1){
                Reports::transactionHistory(std::cout, transactions_);
            }else{
                long long id;

                if (!readInt("Transaction ID: ", 1, 2000000000, id)){
                    continue;
                }

                Transaction t;

                if (transactions_.findById(static_cast<int>(id), t)){ // binary search
                    std::cout << formatReceipt(t);
                }else{
                    std::cout << "  No transaction #" << id << ".\n";
                }
            }
        }
    }

    Config cfg_;
    std::mt19937 rng_;
    Inventory inventory_;
    CategoryTree categories_;
    TransactionManager transactions_;
    CheckoutManager checkout_;
};

int main(int argc, char** argv){
    Config cfg;

    App app(cfg);
    return app.run();
}
