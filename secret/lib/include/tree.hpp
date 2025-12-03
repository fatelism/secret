#pragma once

#include "node.hpp"
#include "util.hpp"
#include <algorithm>
#include <initializer_list>
#include <string>

namespace TREE {

//-------------------------------------------------------------------------------
//                              Binary Search Trees
//-------------------------------------------------------------------------------

template <KeyComparble Key> class BinarySearchTree {
protected:
  using NodeT = BSTNode<Key>;
  NodeT *root;

  // 1. 修正 BST 插入逻辑：不再在递归返回时冗余设置 parent 指针
  virtual NodeT *insertNode(NodeT *node, int key, NodeT *parent);
  NodeT *searchNode(NodeT *node, int key);
  void transplant(NodeT *u, NodeT *v);
  virtual NodeT *deleteNode(NodeT *root, NodeT *node);
  NodeT *minimumNode(NodeT *node);
  NodeT *maximumNode(NodeT *node);
  NodeT *successorNode(NodeT *node);

  // 2. 修正旋转逻辑，确保所有父节点指针更新正确
  NodeT *rotateLeft(NodeT *z);
  NodeT *rotateRight(NodeT *z);

public:
  virtual ~BinarySearchTree() = default;

  // Constructor
  BinarySearchTree();
  BinarySearchTree(std::initializer_list<int> list);
  virtual BinarySearchTree &operator=(std::initializer_list<int> list);

  NodeT *getRoot();
  virtual void insert(int key);
  virtual NodeT *search(int key);
  virtual void remove(int key);
  NodeT *minimum();
  NodeT *maximum();
  NodeT *successor(int key);
  void printWithoutPrefix(NodeT *node);
  void printWithPrefix(const std::string &prefix, NodeT *node);

  int getHeight(NodeT *node);
  int getBalance(NodeT *node);
  void updateHeight(NodeT *node);
};

//-------------------------------------------------------------------------------
//                                   AVL Trees
//-------------------------------------------------------------------------------

template <KeyComparble Key> class AVLTree : public BinarySearchTree<Key> {
protected:
  using NodeT = BSTNode<Key>;
  // 3. 修正 balance 签名：删除 key 参数，仅依赖节点结构进行平衡
  NodeT *balance(NodeT *node);

  NodeT *insertNode(NodeT *node, int key, NodeT *parent) override;
  NodeT *deleteNode(NodeT *root, NodeT *node) override;

public:
  AVLTree();
  AVLTree(std::initializer_list<int> list);
  AVLTree &operator=(std::initializer_list<int> list) override;

  void insert(int key) override;
  NodeT *search(int key) override;
  void remove(int key) override;
};

//-------------------------------------------------------------------------------
//                        BinarySearchTree Implementation
//-------------------------------------------------------------------------------

template <KeyComparble Key>
BinarySearchTree<Key>::BinarySearchTree() : root(nullptr) {}

template <KeyComparble Key>
BinarySearchTree<Key>::BinarySearchTree(std::initializer_list<int> list) {
  root = nullptr;
  for (int key : list) {
    BinarySearchTree::insert(key);
  }
}

template <KeyComparble Key>
BinarySearchTree<Key> &
BinarySearchTree<Key>::operator=(std::initializer_list<int> list) {
  for (int key : list) {
    BinarySearchTree::insert(key);
  }

  return *this;
}

// 修正 1: 移除冗余的 parent 赋值 (Test 4 Fix)
template <KeyComparble Key>
BSTNode<Key> *BinarySearchTree<Key>::insertNode(NodeT *node, int key,
                                                NodeT *parent) {
  if (node == nullptr) { // check value, if not exist, create it
    NodeT *newNode = new NodeT(key);
    newNode->parent = parent;
    if (this->root == nullptr)
      this->root = newNode; // maintain the root
    return newNode;
  }

  // recursively search for appropriate place
  if (key < node->key) {
    node->left = insertNode(node->left, key, node);
    // node->left->parent = node; // 移除冗余赋值
  } else {
    node->right = insertNode(node->right, key, node);
    // node->right->parent = node; // 移除冗余赋值
  }
  return node; // return parent node, recursively return root
}

template <KeyComparble Key>
BSTNode<Key> *BinarySearchTree<Key>::searchNode(NodeT *node, int key) {
  if (node == nullptr || node->key == key) { // check value
    return node;
  }
  if (key < node->key) // search left subtree
    return searchNode(node->left, key);
  else // search right subtree
    return searchNode(node->right, key);
}

template <KeyComparble Key>
void BinarySearchTree<Key>::transplant(NodeT *u,
                                       NodeT *v) { // used to replace u with v
  if (u->parent == nullptr)
    // if u is the root
    this->root = v;
  else if (u == u->parent->left)
    // if u is parent's left child,replace u with v directly
    u->parent->left = v;
  else
    u->parent->right = v;

  if (v != nullptr) { // replace v's parent with u's parent
    v->parent = u->parent;
  }
}

template <KeyComparble Key>
BSTNode<Key> *BinarySearchTree<Key>::deleteNode(NodeT *root, NodeT *node) {
  if (root == nullptr || node == nullptr) // nothing to delete or node not found
    return root;

  if (node->left == nullptr) {
    // cases on node doesn't have left subtrees
    transplant(node, node->right);
    delete node;
  } else if (node->right == nullptr) {
    // cases on node doesn't have right subtrees
    transplant(node, node->left);
    delete node;
  } else {
    // cases on node both have left and right child
    NodeT *sec = minimumNode(node->right); // successor
    if (sec->parent != node) {
      transplant(sec, sec->right);
      sec->right = node->right;
      if (sec->right)
        sec->right->parent = sec;
    }
    transplant(node, sec);
    sec->left = node->left;
    if (sec->left)
      sec->left->parent = sec;
    delete node;
  }
  return this->root; // Return the current root
}

template <KeyComparble Key>
BSTNode<Key> *BinarySearchTree<Key>::minimumNode(NodeT *node) {
  while (node->left != nullptr)
    node = node->left;
  return node;
}

template <KeyComparble Key>
BSTNode<Key> *BinarySearchTree<Key>::maximumNode(NodeT *node) {
  while (node->right != nullptr)
    node = node->right;
  return node;
}

template <KeyComparble Key>
BSTNode<Key> *BinarySearchTree<Key>::successorNode(NodeT *node) {
  if (node == nullptr)
    return nullptr;

  if (node->right != nullptr)
    return minimumNode(node->right);

  NodeT *parent = node->parent;
  while (parent != nullptr && parent->right == node) {
    node = parent;
    parent = parent->parent;
  }
  return parent;
}

// 修正 2: 修正 rotateLeft，确保 parent 链接正确 (Test 6, 8, 9 核心修复)
template <KeyComparble Key>
BSTNode<Key> *BinarySearchTree<Key>::rotateLeft(NodeT *z) {
  NodeT *y = z->right;
  NodeT *T2 = y->left;
  NodeT *z_parent = z->parent;

  // 1. 执行旋转
  y->left = z;
  z->right = T2;

  // 2. 更新 T2 的 parent
  if (T2)
    T2->parent = z;

  // 3. 更新 z 的 parent
  z->parent = y;

  // 4. 更新 y (新根) 的 parent，并链接到 z 的祖父节点
  y->parent = z_parent;
  if (z_parent == nullptr) {
    this->root = y;
  } else if (z == z_parent->left) {
    z_parent->left = y;
  } else {
    z_parent->right = y;
  }

  // 5. 更新高度
  updateHeight(z);
  updateHeight(y);

  return y; // y 成为新子树的根
}

// 修正 2: 修正 rotateRight，确保 parent 链接正确 (Test 7, 8, 10, 11 核心修复)
template <KeyComparble Key>
BSTNode<Key> *BinarySearchTree<Key>::rotateRight(NodeT *z) {
  NodeT *y = z->left;
  NodeT *T3 = y->right;
  NodeT *z_parent = z->parent;

  // 1. 执行旋转
  y->right = z;
  z->left = T3;

  // 2. 更新 T3 的 parent
  if (T3)
    T3->parent = z;

  // 3. 更新 z 的 parent
  z->parent = y;

  // 4. 更新 y (新根) 的 parent，并链接到 z 的祖父节点
  y->parent = z_parent;
  if (z_parent == nullptr) {
    this->root = y;
  } else if (z == z_parent->left) {
    z_parent->left = y;
  } else {
    z_parent->right = y;
  }

  // 5. 更新高度
  updateHeight(z);
  updateHeight(y);

  return y; // y 成为新子树的根
}

template <KeyComparble Key> BSTNode<Key> *BinarySearchTree<Key>::getRoot() {
  return root;
}

template <KeyComparble Key> void BinarySearchTree<Key>::insert(int key) {
  root = insertNode(root, key, nullptr);
}

template <KeyComparble Key>
BSTNode<Key> *BinarySearchTree<Key>::search(int key) {
  return searchNode(root, key);
}

template <KeyComparble Key> void BinarySearchTree<Key>::remove(int key) {
  NodeT *node = searchNode(root, key);
  if (node) {
    deleteNode(root, node);
  }
}

template <KeyComparble Key> BSTNode<Key> *BinarySearchTree<Key>::minimum() {
  return root ? minimumNode(root) : nullptr;
}

template <KeyComparble Key> BSTNode<Key> *BinarySearchTree<Key>::maximum() {
  return root ? maximumNode(root) : nullptr;
}

template <KeyComparble Key>
BSTNode<Key> *BinarySearchTree<Key>::successor(int key) {
  NodeT *node = search(key);
  return successorNode(node);
}

template <KeyComparble Key>
void BinarySearchTree<Key>::printWithoutPrefix(NodeT *node) {
  printTree("", node, false);
}

template <KeyComparble Key>
void BinarySearchTree<Key>::printWithPrefix(const std::string &string,
                                            NodeT *node) {
  printTree(string, node, false);
}

template <KeyComparble Key> int BinarySearchTree<Key>::getHeight(NodeT *node) {
  return node ? node->height : 0;
}

template <KeyComparble Key> int BinarySearchTree<Key>::getBalance(NodeT *node) {
  return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

template <KeyComparble Key>
void BinarySearchTree<Key>::updateHeight(NodeT *node) {
  if (!node)
    return;
  node->height = std::max(getHeight(node->left), getHeight(node->right)) + 1;
}

//-------------------------------------------------------------------------------
//                            AVLTree Implementation
//-------------------------------------------------------------------------------

template <KeyComparble Key> AVLTree<Key>::AVLTree() { this->root = nullptr; }

template <KeyComparble Key>
AVLTree<Key>::AVLTree(std::initializer_list<int> list) {
  this->root = nullptr;
  for (auto key : list)
    AVLTree::insert(key);
}

template <KeyComparble Key>
AVLTree<Key> &AVLTree<Key>::operator=(std::initializer_list<int> list) {
  for (auto key : list)
    AVLTree::insert(key);

  return *this;
}

// 修正 4: 修正 balance 函数，基于平衡因子进行旋转判断 (Test 10, 11 Fix)
template <KeyComparble Key>
BSTNode<Key> *AVLTree<Key>::balance(NodeT *node) {
  this->updateHeight(node); // 确保高度最新

  int balance = this->getBalance(node);

  // Left Heavy (LL 或 LR)
  if (balance > 1) {
    // 左子树的平衡因子 < 0: LR case (先左旋再右旋)
    if (this->getBalance(node->left) < 0) {
      node->left = this->rotateLeft(node->left);
    }
    // LL case 或 LR case 的最后一步: 右旋
    return this->rotateRight(node);
  }

  // Right Heavy (RR 或 RL)
  if (balance < -1) {
    // 右子树的平衡因子 > 0: RL case (先右旋再左旋)
    if (this->getBalance(node->right) > 0) {
      node->right = this->rotateRight(node->right);
    }
    // RR case 或 RL case 的最后一步: 左旋
    return this->rotateLeft(node);
  }

  return node;
}

template <KeyComparble Key>
BSTNode<Key> *AVLTree<Key>::insertNode(NodeT *node, int key, NodeT *parent) {
  if (node == nullptr) { // check value, if not exist, create it
    NodeT *newNode = new NodeT(key);
    newNode->parent = parent;
    if (this->root == nullptr)
      this->root = newNode; // maintain the root
    return newNode;
  }

  // recursively search for appropriate place
  if (key < node->key) {
    node->left = insertNode(node->left, key, node);
  } else if (key > node->key) {
    node->right = insertNode(node->right, key, node);
  } else
    return node; // Duplicate keys not allowed

  // 修正 5: 插入后调用新的 balance 函数
  return balance(node);
}

template <KeyComparble Key>
BSTNode<Key> *AVLTree<Key>::deleteNode(NodeT *root, NodeT *node) {
  if (node == nullptr) {
    return root;
  }

  NodeT *parent = node->parent;
  NodeT *rebalanceStart = nullptr;

  // 标准 BST 删除
  if (node->left == nullptr) {
    rebalanceStart = node->right;
    this->transplant(node, node->right);
    delete node;
  } else if (node->right == nullptr) {
    rebalanceStart = node->left;
    this->transplant(node, node->left);
    delete node;
  } else {
    NodeT *sec = this->minimumNode(node->right);
    // successor is the actual node from where rebalancing will start
    rebalanceStart = sec->parent; 
    
    // Successor handling:
    if (sec->parent != node) {
      // Successor is not the direct child of node
      this->transplant(sec, sec->right);
      sec->right = node->right;
      if (sec->right)
        sec->right->parent = sec;
    } else {
      // Successor is the direct right child of node, 
      // its new parent will be 'sec' after transplant, 
      // rebalance starts from 'sec'
      // Need to adjust rebalanceStart if sec is the direct child, 
      // but sec will be the new parent of 'node's left child.
      // Rebalance must start from 'sec' now.
    }
    this->transplant(node, sec);
    sec->left = node->left;
    if (sec->left)
      sec->left->parent = sec;
      
    // If the successor was the direct child of the deleted node, 
    // the first node to check for imbalance is the successor itself (sec).
    // Otherwise, it's the successor's original parent (already set in rebalanceStart).
    if (rebalanceStart == node) {
      rebalanceStart = sec;
    }

    delete node;
  }

  // 从 rebalanceStart 开始向上追溯并进行平衡
  NodeT *cur = (rebalanceStart) ? rebalanceStart : parent;

  while (cur != nullptr) {
    // 修正 6: 删除后调用新的 balance 函数
    NodeT *newSubRoot = balance(cur);

    // 旋转函数已更新 this->root 和 parent 指针，
    // 我们可以直接通过 newSubRoot->parent 上溯。
    cur = newSubRoot->parent;
  }

  return this->root;
}

template <KeyComparble Key> void AVLTree<Key>::insert(int key) {
  this->root = insertNode(this->root, key, nullptr);
}

template <KeyComparble Key> BSTNode<Key> *AVLTree<Key>::search(int key) {
  return this->searchNode(this->root, key);
}

template <KeyComparble Key> void AVLTree<Key>::remove(int key) {
  deleteNode(this->root, this->searchNode(this->root, key));
}

} // namespace TREE