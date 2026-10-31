
#ifndef CATEGORYTREE_H
#define CATEGORYTREE_H

#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "BinaryTree.h"
#include "Common.h"

// This uses the BinaryTree using the Left-Child / Right-Sibling representation:
//     left  = first sub-category
//     right = next sibling

struct CategoryNode{
    int id = 0;
    std::string name;

    // findData() in BinaryTree.h
    bool operator==(const CategoryNode& o) const{
        return id == o.id;
    }
};

std::ostream& operator<<(std::ostream& os, const CategoryNode& c);

struct CategoryRecord{
    int id = 0;
    int parentId = 0; // 0 for the root
    std::string name;
    int depth = 0;
};

class CategoryTree{
public:
    CategoryTree();
    ~CategoryTree();
    CategoryTree(const CategoryTree&) = delete;
    CategoryTree& operator=(const CategoryTree&) = delete;

    void seedDefault();
    void build(const std::vector<CategoryRecord>& records);

    int rootId() const{
        return rootId_;
    }

    size_t size() const;
    bool exists(int id) const;
    bool get(int id, CategoryNode& out, int* parentId = nullptr) const;
    std::string nameOf(int id) const;
    int childCount(int id) const;

    util::Result addCategory(const std::string& name, int parentId);
    util::Result rename(int id, const std::string& name);
    util::Result reparent(int id, int newParentId); // assign a parent
    util::Result remove(int id, int productCount); // leaf + no products only

    std::vector<int> subtreeIds(int id) const; // id and all descendants (preorder)
    std::vector<CategoryRecord> records() const; // also in preorder
    std::vector<CategoryNode> searchByName(const std::string& fragment) const;

    // Recursive, indented display
    void display(std::ostream& os, const std::map<int, int>& productCounts) const;

private:
    void clear();
    BinaryTree<CategoryNode>* root_;
    int rootId_;
    int nextId_;
};

using util::failure;
using util::Result;
using util::success;

namespace{

using Tree = BinaryTree<CategoryNode>;

Tree* findNode(Tree* node, int id){ // preorder search
    if (node == nullptr){
        return nullptr;
    }

    if (node->getRootVal().id == id){
        return node;
    }

    if (Tree* hit = findNode(node->getLeftChild(), id)){
        return hit;
    }

    return findNode(node->getRightChild(), id);
}

// Locates the child with id somewhere below parent
bool locateChild(Tree* parent, int id, Tree*& parentOut, Tree*& owner, bool& viaLeft){
    Tree* prev = nullptr;

    for (Tree* c = parent->getLeftChild(); c != nullptr; prev = c, c = c->getRightChild()){
        if (c->getRootVal().id == id){
            parentOut = parent;
            owner = prev ? prev : parent;
            viaLeft = (prev == nullptr);
            return true;
        }

        if (locateChild(c, id, parentOut, owner, viaLeft)){
            return true;
        }
    }

    return false;
}

void attachChild(Tree* parent, Tree* child){
    Tree* c = parent->getLeftChild();

    if (c == nullptr){
        parent->setLeftChild(child);
        return;
    }

    while (c->getRightChild() != nullptr){
        c = c->getRightChild();
    }

    c->setRightChild(child);
}

void freeSubtree(Tree* node){ // postorder delete
    if (node == nullptr){
        return;
    }

    freeSubtree(node->getLeftChild());
    freeSubtree(node->getRightChild());
    delete node;
}

void collectIds(Tree* node, std::vector<int>& out){ // node, then its descendants (preorder)
    out.push_back(node->getRootVal().id);

    for (Tree* c = node->getLeftChild(); c != nullptr; c = c->getRightChild()){
        collectIds(c, out);
    }
}

void collectRecords(Tree* parent, int depth, std::vector<CategoryRecord>& out){
    for (Tree* c = parent->getLeftChild(); c != nullptr; c = c->getRightChild()){
        CategoryNode n = c->getRootVal();
        out.push_back({n.id, parent->getRootVal().id, n.name, depth});
        collectRecords(c, depth + 1, out);
    }
}

void collectMatches(Tree* node, const std::string& needle, std::vector<CategoryNode>& out){
    if (node == nullptr){
        return;
    }

    CategoryNode n = node->getRootVal();

    if (util::toLower(n.name).find(needle) != std::string::npos){
        out.push_back(n);
    }

    collectMatches(node->getLeftChild(), needle, out);
    collectMatches(node->getRightChild(), needle, out);
}

size_t countNodes(Tree* node){
    if (node == nullptr){
        return 0;
    }

    return 1 + countNodes(node->getLeftChild()) + countNodes(node->getRightChild());
}

void printNode(std::ostream& os, Tree* node, int depth, const std::map<int, int>& counts){
    CategoryNode n = node->getRootVal();
    auto it = counts.find(n.id);
    int direct = it == counts.end() ? 0 : it->second;

    os << std::string(static_cast<size_t>(depth) * 4, ' ') << (depth ? "- " : "") << "["
       << n.id << "] " << n.name << "  (" << direct << " product" << (direct == 1 ? "" : "s") << ")\n";

    for (Tree* c = node->getLeftChild(); c != nullptr; c = c->getRightChild()){
        printNode(os, c, depth + 1, counts);
    }
}

Result validName(const std::string& name){
    if (name.empty() || name.size() > 40){
        return failure("Category name must be 1-40 characters.");
    }

    return success();
}

bool siblingNameTaken(Tree* parent, const std::string& name, int ignoreId){
    std::string lower = util::toLower(name);

    for (Tree* c = parent->getLeftChild(); c != nullptr; c = c->getRightChild()){
        CategoryNode n = c->getRootVal();

        if (n.id != ignoreId && util::toLower(n.name) == lower){
            return true;
        }
    }

    return false;
}

}

std::ostream& operator<<(std::ostream& os, const CategoryNode& c){
    return os << "[" << c.id << "] " << c.name;
}

CategoryTree::CategoryTree() : root_(nullptr), rootId_(0), nextId_(1){}

CategoryTree::~CategoryTree(){
    clear();
}

void CategoryTree::clear(){
    freeSubtree(root_);
    root_ = nullptr;
    rootId_ = 0;
    nextId_ = 1;
}

void CategoryTree::seedDefault(){
    clear();
    rootId_ = 1;
    root_ = new Tree(CategoryNode{1, "Grocery"});
    nextId_ = 2;

    const struct{ const char* name; int parent; } seed[] = {
        {"Dairy", 1}, {"Produce", 1}, {"Bakery", 1}, {"Beverages", 1}, {"Pantry", 1},
        {"Snacks", 1},
        {"Milk", 2}, {"Cheese", 2}, {"Yogurt", 2},
        {"Fruits", 3}, {"Vegetables", 3},
        {"Bread", 4}, {"Pastries", 4}
    };

    for (const auto& s : seed){
        addCategory(s.name, s.parent);
    }
}

void CategoryTree::build(const std::vector<CategoryRecord>& records){
    clear();

    // the root
    for (const auto& r : records){
        if (r.parentId == 0 && r.id > 0 && validName(r.name).ok){
            root_ = new Tree(CategoryNode{r.id, r.name});
            rootId_ = r.id;
            nextId_ = r.id + 1;
            break;
        }
    }

    if (root_ == nullptr){
        seedDefault();
        return;
    }

    // attach the rest; repeat passes so the file order does not have to be parents-first
    std::vector<CategoryRecord> pending;

    for (const auto& r : records){
        if (!(r.parentId == 0 && r.id == rootId_)){
            pending.push_back(r);
        }
    }

    bool progress = true;

    while (!pending.empty() && progress){
        progress = false;
        std::vector<CategoryRecord> next;

        for (const auto& r : pending){
            Tree* parent = findNode(root_, r.parentId);

            if (parent == nullptr){
                next.push_back(r);
                continue;
            }

            if (r.id <= 0 || findNode(root_, r.id) != nullptr || !validName(r.name).ok){
                progress = true;
                continue;
            }

            Tree* node = new Tree(CategoryNode{r.id, r.name});
            attachChild(parent, node);

            if (r.id >= nextId_){
                nextId_ = r.id + 1;
            }

            progress = true;
        }

        pending.swap(next);
    }

    // unknown parent: moved under the root
    for (const auto& r : pending){
        if (r.id > 0 && findNode(root_, r.id) == nullptr && validName(r.name).ok){
            attachChild(root_, new Tree(CategoryNode{r.id, r.name}));

            if (r.id >= nextId_){
                nextId_ = r.id + 1;
            }
        }
    }
}

size_t CategoryTree::size() const{
    return countNodes(root_);
}

bool CategoryTree::exists(int id) const{
    CategoryNode probe;
    probe.id = id;
    return findData(root_, 1, probe); // preorder findData()
}

bool CategoryTree::get(int id, CategoryNode& out, int* parentId) const{
    Tree* node = findNode(root_, id);

    if (!node){
        return false;
    }

    out = node->getRootVal();

    if (parentId){
        *parentId = 0;
        Tree *p = nullptr, *owner = nullptr;
        bool viaLeft = false;

        if (root_ && id != rootId_ && locateChild(root_, id, p, owner, viaLeft)){
            *parentId = p->getRootVal().id;
        }
    }

    return true;
}

std::string CategoryTree::nameOf(int id) const{
    Tree* node = findNode(root_, id);
    return node ? node->getRootVal().name : "?";
}

int CategoryTree::childCount(int id) const{
    Tree* node = findNode(root_, id);
    int n = 0;

    if (node){
        for (Tree* c = node->getLeftChild(); c != nullptr; c = c->getRightChild()){
            ++n;
        }
    }

    return n;
}

std::vector<int> CategoryTree::subtreeIds(int id) const{
    std::vector<int> out;
    Tree* node = findNode(root_, id);

    if (node){
        collectIds(node, out);
    }

    return out;
}

std::vector<CategoryRecord> CategoryTree::records() const{
    std::vector<CategoryRecord> out;

    if (!root_){
        return out;
    }

    CategoryNode r = root_->getRootVal();
    out.push_back({r.id, 0, r.name, 0});
    collectRecords(root_, 1, out);

    return out;
}

std::vector<CategoryNode> CategoryTree::searchByName(const std::string& fragment) const{
    std::vector<CategoryNode> out;
    collectMatches(root_, util::toLower(fragment), out);

    return out;
}

void CategoryTree::display(std::ostream& os, const std::map<int, int>& productCounts) const{
    if (!root_){
        os << "(no categories)\n";
        return;
    }

    printNode(os, root_, 0, productCounts);
}

Result CategoryTree::addCategory(const std::string& name, int parentId){
    Result v = validName(name);

    if (!v.ok){
        return v;
    }

    Tree* parent = findNode(root_, parentId);

    if (!parent){
        return failure("Parent category " + std::to_string(parentId) + " does not exist.");
    }

    if (siblingNameTaken(parent, name, 0)){
        return failure("'" + name + "' already exists under " + parent->getRootVal().name + ".");
    }

    CategoryNode node{nextId_, name};

    if (parent->getLeftChild() == nullptr){
        parent->insertLeft(node); // first child
    }else{
        Tree* last = parent->getLeftChild();

        while (last->getRightChild() != nullptr){
            last = last->getRightChild();
        }

        last->insertRight(node); // next sibling
    }

    return success("Category added.", nextId_++);
}

Result CategoryTree::rename(int id, const std::string& name){
    Result v = validName(name);

    if (!v.ok){
        return v;
    }

    Tree* node = findNode(root_, id);

    if (!node){
        return failure("Category " + std::to_string(id) + " does not exist.");
    }

    if (id != rootId_){
        Tree *p = nullptr, *owner = nullptr;
        bool viaLeft = false;

        if (locateChild(root_, id, p, owner, viaLeft) && siblingNameTaken(p, name, id)){
            return failure("A sibling category already uses that name.");
        }
    }

    CategoryNode n = node->getRootVal();
    n.name = name;
    node->setRootVal(n);

    return success("Category renamed.", id);
}

Result CategoryTree::reparent(int id, int newParentId){
    if (id == rootId_){
        return failure("The root category cannot be moved.");
    }

    Tree* node = findNode(root_, id);

    if (!node){
        return failure("Category " + std::to_string(id) + " does not exist.");
    }

    Tree* newParent = findNode(root_, newParentId);

    if (!newParent){
        return failure("Parent category " + std::to_string(newParentId) + " does not exist.");
    }

    if (newParentId == id || findNode(node->getLeftChild(), newParentId) != nullptr){
        return failure("A category cannot be its own sub-categories.");
    }

    Tree *oldParent = nullptr, *owner = nullptr;
    bool viaLeft = false;

    if (!locateChild(root_, id, oldParent, owner, viaLeft)){
        return failure("Category not linked.");
    }

    if (oldParent == newParent){
        return success("Already under that parent.", id);
    }

    if (siblingNameTaken(newParent, node->getRootVal().name, id)){
        return failure("The new parent already has a category with that name.");
    }

    // unlink, then attach as the last child of the new parent
    if (viaLeft){
        owner->setLeftChild(node->getRightChild());
    }else{
        owner->setRightChild(node->getRightChild());
    }

    node->setRightChild(nullptr);
    attachChild(newParent, node);

    return success("Category moved.", id);
}

Result CategoryTree::remove(int id, int productCount){
    if (id == rootId_){
        return failure("The root category cannot be deleted.");
    }

    Tree* node = findNode(root_, id);

    if (!node){
        return failure("Category " + std::to_string(id) + " does not exist.");
    }

    if (node->getLeftChild() != nullptr){
        return failure("Move or delete subcategories first.");
    }

    if (productCount > 0){
        return failure("It still contains " + std::to_string(productCount) + " product(s). Reassign them first.");
    }

    Tree *parent = nullptr, *owner = nullptr;
    bool viaLeft = false;

    if (!locateChild(root_, id, parent, owner, viaLeft)){
        return failure("Category not linked.");
    }

    if (viaLeft){
        owner->setLeftChild(node->getRightChild());
    }else{
        owner->setRightChild(node->getRightChild());
    }

    delete node;

    return success("Category deleted.", id);
}

#endif
