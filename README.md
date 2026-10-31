# Grocery Inventory and Checkout Management System (C++17)

Build: `build.bat` (Windows) or `make` → `./grocery` (options: `--data FOLDER --lanes 1-8 --delay MS --customers N --seed N`).
Tests: `make test`, `make tsan`, `make asan`. Sample data is in `data/`.

## Customers (automatic)
Customers are not created by hand any more. At the start of every session a random group (4-10, or
`--customers N`) arrives. Each one fills a random basket from what is on the shelves and joins a random
lane; then you open the lanes from menu 4. More customers can be brought in any time with menu 4 -> 2 (Simulate customers). `--seed N` makes a session repeatable.

## Stock: storehouse and shelves
- Every product has two counts: **Shelf** (what customers can buy) and **Store** (the storehouse).
- A delivery (menu 3 -> 1) is booked into the **storehouse**, never straight onto the shelf.
- Employees put stock on the shelves (menu 3 -> 3, one product at a time).
- Stock is taken from the shelf when a customer's item goes into the cart (check and deduction happen
  together under one lock, so two customers can never get the same last unit).
- Checkout lanes never reject a cart: they just record the sale and print the receipt.
- Carts still waiting when the program exits are not saved; their items are added back to the saved shelf stock.
- `data/products.txt`: `id|categoryId|priceCentavos|shelfQty|storehouseQty|name` (old 5-field files still load).

## Outline -> files (everything is header-only; only `main.cpp` is compiled)
| Module | File | Your headers used |
|---|---|---|
| Shared helpers | Common.h | – |
| Inventory (+ Product) | Inventory.h | simply_ll.h |
| Category tree | CategoryTree.h | BinaryTree.h (left-child/right-sibling) |
| Search | SearchSort.h | linear + binary search |
| Carts + lane queues | Cart.h | queue_ll.h |
| Transactions (+ manager) | Transaction.h | – (array + binary search by id) |
| Threaded checkout | CheckoutManager.h | std::thread, std::mutex, join |
| Reports | Reports.h | tree traversal |
| Persistence | FileManager.h | – |
| Menu | main.cpp | – |

Build by hand: `g++ -std=c++17 -pthread -Isrc src/main.cpp -o grocery`

## Changes made to your headers (now in `src/`)
queue_ll.h: enqueue on an empty queue linked the node to itself; dequeue/showFront crashed when empty.
simply_ll.h: added `<iostream>`. BinaryTree.h: added setLeftChild/setRightChild (to move/delete categories).

## Complexity
Linear search O(n) · binary search O(log n) · queue enqueue/dequeue O(1) · append to inventory O(1) ·
traverse products O(n) · traverse categories O(c) · remove product O(n).
