# Grocery Inventory and Checkout Management System (C++17)

## 1. Project Description

The Grocery Inventory and Checkout Management System is a C++17 console-based application designed to manage grocery products, inventory, customer carts, checkout lanes, and sales transactions. It uses different data structures and algorithms to organize products, manage customer queues, search inventory, and process transactions.

The system also supports multithreaded checkout processing and file-based data persistence.

**Key Features:**
- Product and category management.
- Storehouse and shelf inventory management.
- Customer cart generation.
- Checkout lane and queue management.
- Multithreaded checkout processing.
- Transaction recording and receipt generation.
- Inventory and sales reports.
- File-based data saving and loading.

## 2. Compilation and Execution Instructions

### Requirements
- C++ compiler that supports C++17.
- GCC or MinGW compiler.
- C++ Standard Library with thread support.

### Compilation

Open a terminal in the project directory and execute:

```bash
g++ -std=c++17 -pthread main.cpp -o grocery_system
```

### Execution

**Linux:**
```bash
./grocery_system
```

**Windows:**
```cmd
grocery_system.exe
```

To specify the data directory:

```bash
./grocery_system --data .
```

## 3. Sample Input/Output

### Compilation Command

```bash
g++ -std=c++17 -pthread main.cpp -o grocery_system
```

### Execution Command

```bash
./grocery_system --data . --customers 0 --seed 1
```

### Sample Terminal Interaction

```text
Grocery Inventory and Checkout Management System

Loaded product, category, and transaction data.
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

**Note:** The sample demonstrates program startup and exit. Actual displayed messages and loaded record counts depend on the contents of the data files.

## 4. Team Members

- **qedizor-lgtm** – Dizor, Elijah I.
- **Payascpe** – Payas, John Paul S.
- **BurgerPurger** – Magtaan, Dylan Paolo M.
- **JaredDC21** – Dela Cruz, Jared Isaiah P.
- **RPDevs** – Panti, Dominic Rodnee A.

## GitHub Repository

https://github.com/BurgerPurger/Grocery-Lane-and-Stock-Management-System
