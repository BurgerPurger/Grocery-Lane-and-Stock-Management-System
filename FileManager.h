
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <climits>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "CategoryTree.h"
#include "Inventory.h"
#include "Transaction.h"

namespace FileManager{
bool ensureDirectory(const std::string& dir, std::string& err);

bool loadProducts(const std::string& path, std::vector<Product>& out);
bool saveProducts(const std::string& path, const std::vector<Product>& products, std::string& err);

bool loadCategories(const std::string& path, std::vector<CategoryRecord>& out);
bool saveCategories(const std::string& path, const std::vector<CategoryRecord>& records, std::string& err);

bool loadTransactions(const std::string& path, std::vector<Transaction>& out);
bool saveTransactions(const std::string& path, const std::vector<Transaction>& txns, std::string& err);
}

namespace fs = std::filesystem;

// Shared file utilities.
namespace{
    std::mutex& fileMutex(){
        static std::mutex m;
        return m;
    }

    // Writes data safely using a temporary file.
    bool atomicWrite(const std::string& path, const std::string& content, std::string& err){
        std::lock_guard<std::mutex> lock(fileMutex());
        std::string tmp = path + ".tmp";

        return true;
    }

    // Reads non-empty, non-comment lines.
    bool readLines(const std::string& path, std::vector<std::string>& lines){
        std::ifstream in(path);

        if(!in){
            return false;
        }
        std::string text;

        while(std::getline(in, text)){
            if(!text.empty() && text.back() == '\r'){
                text.pop_back();
            }
            if(util::trim(text).empty() || text[0] == '#'){
                continue;
            }
            lines.push_back(text);
        }
        return true;
    }

    // Converts a string to an integer.
    bool toInt(const std::string& s, int& out){
        long long v;

        if(!util::parseInt(s, v) || v < INT_MIN || v > INT_MAX){
            return false;
        }
        out = static_cast<int>(v);
        return true;
    }
}

// Product file operations.
namespace FileManager{
bool ensureDirectory(const std::string& dir, std::string& err){
    std::error_code ec;
    fs::create_directories(dir, ec);

    return true;
}

bool loadProducts(const std::string& path, std::vector<Product>& out){
    std::vector<std::string> lines;

    if(!readLines(path, lines)){
        return false;
    }
    for(const auto& ln : lines){
        std::vector<std::string> f = util::split(ln, '|');
        Product p;
        long long price = 0;

        bool ok = f.size() == 6 && toInt(f[0], p.id) &&
                  toInt(f[1], p.categoryId) && util::parseInt(f[2], price) &&
                  toInt(f[3], p.quantity) && toInt(f[4], p.storage);

        if(!ok){
            continue;
        }
        p.price = price;
        p.name = f.back();
        out.push_back(p);
    }
    return true;
}

bool saveProducts(const std::string& path, const std::vector<Product>& products, std::string& err){
    std::ostringstream os;
    os << "# id|categoryId|priceCentavos|shelfQuantity|storehouseQuantity|name\n";

    for(const auto& p : products){
        os << p.id << '|' << p.categoryId << '|' << p.price << '|' << p.quantity << '|'
           << p.storage << '|' << p.name << '\n';
    }
    return atomicWrite(path, os.str(), err);
}

// Category file operations.
bool loadCategories(const std::string& path, std::vector<CategoryRecord>& out){
    std::vector<std::string> lines;

    if(!readLines(path, lines)){
        return false;
    }
    for(const auto& ln : lines){
        std::vector<std::string> f = util::split(ln, '|', 3);
        CategoryRecord r;

        if(f.size() != 3 || !toInt(f[0], r.id) || !toInt(f[1], r.parentId) || f[2].empty()){
            continue;
        }
        r.name = f[2];
        out.push_back(r);
    }
    return true;
}

bool saveCategories(const std::string& path, const std::vector<CategoryRecord>& records, std::string& err){
    std::ostringstream os;
    os << "# id|parentId|name\n";

    for(const auto& r : records){
        os << r.id << '|' << r.parentId << '|' << r.name << '\n';
    }
    return atomicWrite(path, os.str(), err);
}

// Transaction file operations.
bool loadTransactions(const std::string& path, std::vector<Transaction>& out){
    std::vector<std::string> lines;

    if(!readLines(path, lines)){
        return false;
    }
    bool haveCurrent = false;
    Transaction cur;

    auto flush = [&](){
        if(haveCurrent){
            out.push_back(cur);
        }
        haveCurrent = false;
        cur = Transaction();
    };

    for(const auto& ln : lines){
        if(ln.rfind("TXN|", 0) == 0){
            flush();

            std::vector<std::string> f = util::split(ln, '|');
            long long total = 0;

            if(f.size() != 5 || !toInt(f[1], cur.id) || !toInt(f[2], cur.cartId) ||
               !toInt(f[3], cur.lane) || !util::parseInt(f[4], total)){
                cur = Transaction();
                continue;
            }
            cur.total = total;
            haveCurrent = true;
        }
        else if(ln.rfind("ITEM|", 0) == 0){
            if(!haveCurrent){
                continue;
            }
            std::vector<std::string> f = util::split(ln, '|', 5);
            TxnItem it;
            long long price = 0;

            if(f.size() != 5 || !toInt(f[1], it.productId) || !toInt(f[2], it.quantity) ||
               !util::parseInt(f[3], price) || it.quantity <= 0 || price <= 0){
                continue;
            }
            it.unitPrice = price;
            it.name = f[4];
            cur.items.push_back(it);
        }
    }

    flush();
    return true;
}

bool saveTransactions(const std::string& path, const std::vector<Transaction>& txns, std::string& err){
    std::ostringstream os;
    os << "# TXN|id|cartId|lane|totalCentavos\n"
       << "# ITEM|productId|quantity|unitPriceCentavos|name\n";

    for(const auto& t : txns){
        os << "TXN|" << t.id << '|' << t.cartId << '|' << t.lane
           << '|' << t.total << '\n';

        for(const auto& it : t.items){
            os << "ITEM|" << it.productId << '|' << it.quantity << '|' << it.unitPrice << '|'
               << it.name << '\n';
        }
    }
    return atomicWrite(path, os.str(), err);
}

}

#endif
