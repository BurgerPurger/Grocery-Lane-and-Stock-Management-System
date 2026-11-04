// Each measurement times the operation only (data is built first), repeated 5 times; the average is printed.

#include <chrono>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

#include "Cart.h"
#include "Inventory.h"
#include "Transaction.h"

using Clock = std::chrono::steady_clock;

static const int kRuns = 5;
static const int kLookups = 1000;
static volatile long long g_sink = 0; // keeps the compiler from removing the work

static double msSince(Clock::time_point t0){
    return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
}

// 1. Add n products to the inventory linked list (append after the tail).
static double benchAppend(int n){
    double total = 0;

    for (int r = 0; r < kRuns; ++r){
        Inventory inv;
        auto t0 = Clock::now();

        for (int i = 0; i < n; ++i){
            inv.addProduct("Item", 1, 1000, 5);
        }
        total += msSince(t0);
    }
    return total / kRuns;
}

// 2. Linear search: 1,000 lookups of the LAST product (worst case) in a list of n products.
static double benchLinear(int n){
    Inventory inv;

    for (int i = 0; i < n; ++i){
        inv.addProduct("Item", 1, 1000, 5);
    }

    double total = 0;

    for (int r = 0; r < kRuns; ++r){
        Product p;
        auto t0 = Clock::now();

        for (int q = 0; q < kLookups; ++q){
            inv.get(n, p);
            g_sink += p.id;
        }
        total += msSince(t0);
    }
    return total / kRuns;
}

// 3. Binary search: 1,000 lookups of random ids among n transactions.
static double benchBinary(int n){
    TransactionManager tm;
    std::vector<TxnItem> items{{1, "Item", 1, 1000}};

    for (int i = 0; i < n; ++i){
        tm.recordSale(101 + i, 1, items);
    }

    std::mt19937 rng(12345);
    std::vector<int> keys(kLookups);

    for (auto& k : keys){
        k = std::uniform_int_distribution<int>(1, n)(rng);
    }

    double total = 0;

    for (int r = 0; r < kRuns; ++r){
        Transaction t;
        auto t0 = Clock::now();

        for (int k : keys){
            tm.findById(k, t);
            g_sink += t.id;
        }
        total += msSince(t0);
    }
    return total / kRuns;
}

// 3b. The binary search alone: 1,000,000 lookups in an array of n.
static double benchBinaryPure(int n){
    std::vector<Transaction> arr(static_cast<size_t>(n));

    for (int i = 0; i < n; ++i){
        arr[static_cast<size_t>(i)].id = i + 1;
    }

    std::mt19937 rng(777);
    std::vector<int> keys(1000000);

    for (auto& k : keys){
        k = std::uniform_int_distribution<int>(1, n)(rng);
    }

    double total = 0;

    for (int r = 0; r < kRuns; ++r){
        Transaction probe;
        auto t0 = Clock::now();

        for (int k : keys){
            probe.id = k;
            g_sink += binarySearchIndex(arr.data(), n, probe);
        }
        total += msSince(t0);
    }
    return total / kRuns;
}

// 4. Queue: n enqueues followed by n dequeues on one lane.
static double benchQueue(int n){
    double total = 0;

    for (int r = 0; r < kRuns; ++r){
        LaneQueue q(1);
        Cart c;
        c.id = 1;
        auto t0 = Clock::now();

        for (int i = 0; i < n; ++i){
            q.enqueueCart(c);
        }

        Cart out;

        while (q.dequeueCart(out)){
            g_sink += out.id;
        }
        total += msSince(t0);
    }
    return total / kRuns;
}

int main(int argc, char** argv){
    std::printf("BENCHMARK (average of %d runs, time in milliseconds)\n\n", kRuns);

    std::printf("%9s | %12s | %14s | %14s | %16s | %14s\n",
        "n",
        "Append n",
        "Linear x1,000",
        "Binary x1,000",
        "Binary x1,000,000",
        "Queue n in/out"
    );

    std::printf("----------+--------------+----------------+----------------+-------------------+---------------\n");

    for (int n : {100, 1000, 10000, 100000}){
        double append = benchAppend(n);
        double linear = benchLinear(n);
        double binary = benchBinary(n);
        double pure = benchBinaryPure(n);
        double queue = benchQueue(n);

        std::printf("%9d | %12.4f | %14.4f | %14.4f | %17.4f | %14.4f\n",
            n,
            append,
            linear,
            binary,
            pure,
            queue
        );

        std::fflush(stdout);
    }

    return 0;
}