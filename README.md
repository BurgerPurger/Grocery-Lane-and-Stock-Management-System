# Grocery Inventory and Checkout Management System (C++17)

## Stock: storehouse and shelves
- Every product has two counts: **Shelf** (what customers can buy) and **Store** (the storehouse).
- A delivery (menu 3 -> 1) is booked into the **storehouse**, never straight onto the shelf.
- Employees put stock on the shelves (menu 3 -> 3, one product at a time).
- Stock is taken from the shelf when a customer's item goes into the cart (check and deduction happen
  together under one lock, so two customers can never get the same last unit).
- Checkout lanes never reject a cart: they just record the sale and print the receipt.
- Carts still waiting when the program exits are not saved; their items are added back to the saved shelf stock.

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
