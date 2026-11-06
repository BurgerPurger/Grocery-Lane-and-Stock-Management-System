# Grocery Inventory and Checkout Management System (C++17)

## Project Description

The Grocery Inventory and Checkout Management System is a C++17 console-based application designed to manage grocery products, inventory, customer carts, checkout lanes, and sales transactions. It uses different data structures and algorithms to organize products, manage customer queues, search inventory, and process transactions.

The system also implements multithreaded checkout processing and file-based data persistence.

## Features

- **Product Management** – Add, display, search, update, and remove products.
- **Category Management** – Create, rename, search, delete, and organize product categories using a tree structure.
- **Inventory Management** – Track product quantities on shelves and in the storehouse.
- **Stock Restocking** – Receive deliveries and transfer products from the storehouse to shelves.
- **Customer Generation** – Automatically generate customers with randomly selected products and quantities.
- **Checkout Lane Management** – Assign customer carts to checkout lanes using queues.
- **Multithreaded Checkout** – Process checkout lanes using C++ threads and mutexes.
- **Transaction Management** – Record sales transactions and retrieve receipts using transaction IDs.
- **Sales Reports** – View sales and inventory information.
- **File Handling** – Save and load products, categories, and transaction records.

## Requirements

- C++ compiler that supports C++17.
- GCC or MinGW compiler.
- C++ Standard Library with thread support.
- Git (optional, for cloning the repository).

## Compilation and Execution Instructions

### 1. Clone the Repository

Open a terminal and execute:

```bash
git clone https://github.com/BurgerPurger/Grocery-Lane-and-Stock-Management-System.git
```

Navigate to the project directory:

```bash
cd Grocery-Lane-and-Stock-Management-System
```

### 2. Compile the Program

Use the following command:

```bash
g++ -std=c++17 -pthread main.cpp -o grocery_system
```

The `-std=c++17` option enables C++17 features, while `-pthread` enables thread support.

### 3. Run the Program

**Linux:**

```bash
./grocery_system
```

**Windows (MinGW):**

```cmd
grocery_system.exe
```

To use the product and category text files located in the repository's root directory, specify the current directory as the data folder:

**Linux:**

```bash
./grocery_system --data . 
```

**Windows:**

```cmd
grocery_system.exe --data .
```

### 4. Available Command-Line Arguments

| Argument | Description |
|---|---|
| `--data FOLDER` | Specifies the folder used for data files. |
| `--lanes 1-8` | Sets the number of checkout lanes. |
| `--delay MILLISECONDS` | Sets the simulated checkout delay. |
| `--customers N` | Sets the number of customers generated per session. |
| `--seed N` | Sets the random number seed for reproducible generation. |
| `--help` | Displays the available command-line arguments. |

Example:

```bash
./grocery_system --data . --lanes 3 --delay 150 --customers 5 --seed 1
```

## Sample Input/Output

### A. Compilation

**Terminal Command:**

```bash
g++ -std=c++17 -pthread main.cpp -o grocery_system
```

### B. Program Execution

**Terminal Command:**

```bash
./grocery_system --data . --customers 0 --seed 1
```

### C. Sample Console Output

```text
Loaded [number] product(s), [number] categories,
[number] transaction(s) from '.'.
No customers arrived.

========================================
 GROCERY INVENTORY AND CHECKOUT SYSTEM
========================================

1. Manage Products
2. Manage Categories
3. Storehouse and Shelf Restocking
4. Process Checkout Lanes
5. View Transaction History
6. View Sales Report
0. Exit (data is saved automatically)

Enter choice: 0

Saving data...
```

**Sample Input:**

```text
0
```

**Expected Output:**

```text
Saving data...
```

*Note: The number of products, categories, and transactions depends on the saved data files. The sample uses zero customers to demonstrate program startup and exit.*

## System Overview

### 1. Stock Management: Storehouse and Shelves

Every product has two stock quantities:

- **Shelf:** The quantity available for customers to purchase.
- **Store:** The quantity kept in the storehouse.

A delivery (menu option 3 → 1) is added to the storehouse, not directly to the shelves.

Employees transfer stock from the storehouse to the shelves (menu option 3 → 3), one product at a time.

When an item is added to a customer's cart, its shelf quantity is checked and deducted under a lock. This prevents multiple customers from taking the same remaining unit.

### 2. Customer Generation

The system automatically generates customers with randomly selected products and quantities based on available shelf stock.

Each customer's cart is assigned to a randomly selected checkout lane. The process continues until all configured customers have been generated.

### 3. Checkout Processing

Customer carts are placed in checkout lane queues. Multiple checkout lanes can process transactions using separate threads.

The system records completed sales and generates receipts for customers.

### 4. Transaction and Data Persistence

The system stores product information, category records, and transaction history in text files.

Completed transactions can be viewed through the transaction history menu. Carts still waiting when the program exits are not saved; their items are returned to the saved shelf stock.

## Project Structure

The project uses header files for its different modules, while `main.cpp` serves as the main entry point.

| Module | File | Description |
|---|---|---|
| Shared Helpers | `Common.h` | Common utilities and input handling |
| Inventory and Products | `Inventory.h` | Product and inventory management |
| Linked List | `simply_ll.h` | Singly linked list implementation |
| Category Management | `CategoryTree.h` | Category organization |
| Binary Tree | `BinaryTree.h` | Left-child/right-sibling tree implementation |
| Search and Sorting | `SearchSort.h` | Linear and binary search |
| Cart Management | `Cart.h` | Customer cart handling |
| Queue Management | `queue_ll.h` | Checkout lane queues |
| Transaction Management | `Transaction.h` | Transaction records and ID searching |
| Checkout Processing | `CheckoutManager.h` | Multithreaded checkout using `std::thread` and `std::mutex` |
| File Handling | `FileManager.h` | Loading and saving data |
| Main Program | `main.cpp` | Main menu and system operations |
| Product Data | `products.txt` | Saved product information |
| Category Data | `categories.txt` | Saved category information |
| Benchmark | `benchmark.cpp` | Performance benchmarking |

**Note:** `main.cpp` also includes `Reports.h`; make sure this header is present in the project directory when compiling.

## Team Members

- **qedizor-lgtm** – Dizor, Elijah I.
- **Payascpe** – Payas, John Paul S.
- **BurgerPurger** – Magtaan, Dylan Paolo M.
- **JaredDC21** – Dela Cruz, Jared Isaiah P.
- **RPDevs** – Panti, Dominic Rodnee A.

## GitHub Repository

[https://github.com/BurgerPurger/Grocery-Lane-and-Stock-Management-System](https://github.com/BurgerPurger/Grocery-Lane-and-Stock-Management-System)
