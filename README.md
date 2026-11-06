# Grocery Inventory and Checkout Management System (C++17)

## Project Description

The Grocery Inventory and Checkout Management System is a console-based C++17 application designed to manage grocery products, stock levels, customer carts, checkout lanes, and sales transactions. It uses different data structures and algorithms to organize inventory, process customer queues, search products, and manage transactions.

The system also supports multithreaded checkout processing and file-based data persistence.

## Features

- Manage product inventory using separate storehouse and shelf stock counts.
- Record deliveries in the storehouse and transfer stock to shelves.
- Check and deduct shelf stock when products are added to customer carts.
- Organize product categories using a tree structure.
- Search for products using linear and binary search.
- Manage customer carts and checkout lanes using queues.
- Process checkout tasks using C++ threads and mutexes.
- Record transactions and generate reports.
- Save and load system data through files.

## Requirements

- C++ compiler that supports C++17 (GCC or MinGW).
- Standard C++ library with thread support.
- All project header files and required data files.

## Compilation and Execution Instructions

### Compilation

Open a terminal in the project directory and run:

```bash
g++ -std=c++17 -pthread main.cpp -o grocery_system
```

The `-pthread` option enables thread support. Only `main.cpp` needs to be compiled because the project uses header files.

### Execution

**Linux:**

```bash
./grocery_system
```

**Windows:**

```cmd
grocery_system.exe
```

Run the program from the project directory so that its required data files can be accessed.

## Sample Input/Output

The following is an illustrative example of the system workflow.

**Sample Input / Workflow:**

1. Start the program.
2. Select the inventory management option.
3. Add a delivery to the storehouse.
4. Transfer available products from the storehouse to the shelves.
5. Generate customers and process their carts through checkout lanes.
6. View the transaction and inventory reports.

**Expected Output:**

- Delivery is successfully recorded in the storehouse.
- Products are transferred from the storehouse to the shelves.
- Customer carts contain products available on the shelves.
- Completed checkouts are recorded as sales transactions.
- Receipts are displayed after checkout.

*Note: This is an illustrative workflow, not a verbatim console transcript.*

## Stock Management: Storehouse and Shelves

Every product has two stock counts:

- **Shelf:** The quantity currently available for customers to purchase.
- **Store:** The quantity kept in the storehouse.

A delivery (menu option 3 → 1) is added to the storehouse, not directly to the shelves.

Employees transfer stock to the shelves through menu option 3 → 3, one product at a time.

When an item is added to a customer's cart, its shelf quantity is checked and deducted together under a lock. This prevents multiple customers from taking the same remaining unit.

Checkout lanes process carts and record sales. Carts still waiting when the program exits are not saved; their items are returned to the saved shelf stock.

## Project Structure

The project uses header files for its modules, while `main.cpp` contains the program entry point.

| Module | File | Purpose / Related Header |
|---|---|---|
| Shared Helpers | `Common.h` | Common helper functions |
| Inventory and Products | `Inventory.h` | Uses `simply_ll.h` |
| Category Tree | `CategoryTree.h` | Uses `BinaryTree.h` (left-child/right-sibling) |
| Search and Sorting | `SearchSort.h` | Linear and Binary Search |
| Carts and Lane Queues | `Cart.h` | Uses `queue_ll.h` |
| Transactions | `Transaction.h` | Transaction records and searching by ID |
| Threaded Checkout | `CheckoutManager.h` | `std::thread`, `std::mutex`, and `join()` |
| Reports | `Reports.h` | Tree traversal |
| File Handling | `FileManager.h` | Loads and saves system data |
| Program Entry Point | `main.cpp` | Main menu and program execution |

## Team Members

- **qedizor-lgtm** – Dizor, Elijah I.
- **Payascpe** – Payas, John Paul S.
- **BurgerPurger** – Magtaan, Dylan Paolo M.
- **JaredDC21** – Dela Cruz, Jared Isaiah P.
- **RPDevs** – Panti, Dominic Rodnee A.

## GitHub Repository

[Grocery Lane and Stock Management System](https://github.com/BurgerPurger/Grocery-Lane-and-Stock-Management-System.git)
