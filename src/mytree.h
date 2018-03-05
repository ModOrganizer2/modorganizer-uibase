/*
Mod Organizer shared UI functionality

Copyright (C) 2012 Sebastian Herbord. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef MYTREE_H
#define MYTREE_H

#include "dllimport.h"

#include <QString>

#include <list>
#include <set>
#include <utility>

namespace MOBase {


/**
 * a tree container using seperate structures for leafs and inner nodes
 * duplicates in NodeData or Leaf-data are not allowed
 * @note this is currently only used to represent directory structures in the installation
 *       manager
 **/
template <typename LeafT, typename NodeData>
class MyTree
{

public:

  typedef MyTree<LeafT, NodeData> Node;

private:

  struct ByNodeData
  {
    bool operator()(const Node *lhs, const Node *rhs) const
    {
      return lhs->getData() < rhs->getData();
    }
  };

public:

  typedef typename std::set<LeafT>::iterator leaf_iterator;
  typedef typename std::set<Node*, ByNodeData>::iterator node_iterator;

  typedef typename std::set<LeafT>::const_iterator const_leaf_iterator;
  typedef typename std::set<Node*, ByNodeData>::const_iterator const_node_iterator;

  typedef typename std::set<Node*, ByNodeData>::const_reverse_iterator const_node_reverse_iterator;
  typedef typename std::set<LeafT>::const_reverse_iterator const_leaf_reverse_iterator;

  typedef typename std::list<std::pair<int, int>> Overwrites;

public:

  /**
   * @brief constructor
   **/
  MyTree()
    : m_Parent(nullptr)
  {}

  ~MyTree();

  MyTree(const MyTree<LeafT, NodeData> &reference);

  /**
   * @brief assignment operator
   */
  MyTree &operator=(const MyTree<LeafT, NodeData> &reference);

  /**
   * @return a deep copy of this tree including all subnodes
   */
  MyTree<LeafT, NodeData> *copy() const;

  /**
   * @brief set the data for this node
   *
   * @param data data attached to this node
   **/
  void setData(const NodeData &data) { m_Data = data; }

  /**
   * @return data connected to this node
   **/
  const NodeData &getData() const { return m_Data; }

  /**
   * @brief add a new leaf to this node
   *
   * @param leaf the leaf data to attach
   * @param overwrite if true, the new leaf will overwrite an existing one that
   *compares as "equal"
   * @return true if the leaf was added, false if it already exists
   **/
  bool addLeaf(const LeafT &leaf, bool overwrite = true,
               Overwrites *overwrites = nullptr)
  {
    auto res = m_Leafs.insert(leaf);
    if (!res.second && overwrite) {
      if (overwrites != nullptr) {
        overwrites->push_back(
            std::make_pair(static_cast<int>(res.first->getIndex()),
                           static_cast<int>(leaf.getIndex())));
      }
      m_Leafs.erase(res.first);
      res = m_Leafs.insert(leaf);
    }
    return res.second;
  }

  /**
   * @brief add a new node to the tree
   *
   * @param node node to add. "this" takes custody of the pointer
   * @param merge if true the content of node will merged with an existing node
   * @param overwrites if not null, a list of overwritten nodes will be maintained
   * @param prio priority at which leaves are added. If a high priority leaf
   *             overwrites a low priority leaf, this doesn't get added to the
   *             overwrites list.
   * @return true if the node was added or merged. false if merge is false and a node with the
   *         specified node data exists already
   **/
  bool addNode(Node *node, bool merge, Overwrites *overwrites = nullptr);

  /**
   * @return the number of leafs in the current node
   **/
  std::size_t numLeafs() const { return m_Leafs.size(); }

  /**
   * @return the number of child-nodes in the current node
   **/
  std::size_t numNodes() const { return m_Nodes.size(); }

  /**
   * @return an iterator to the first leaf
   **/
  leaf_iterator leafsBegin() { return m_Leafs.begin(); }

  /**
   * @return a const iterator to the first leaf
   **/
  const_leaf_iterator leafsBegin() const { return m_Leafs.begin(); }

  /**
   * @return a const reverse iterator to the last leaf
   **/
  const_leaf_reverse_iterator leafsRBegin() const { return m_Leafs.rbegin(); }

  /**
   * @return an iterator one past the last leaf
   **/
  leaf_iterator leafsEnd() { return m_Leafs.end(); }

  /**
   * @return a const iterator one past the last leaf
   **/
  const_leaf_iterator leafsEnd() const { return m_Leafs.end(); }

  /**
   * @return a const reverse iterator one past the first leaf
   **/
  const_leaf_reverse_iterator leafsREnd() const { return m_Leafs.rend(); }

  /**
   * @return an iterator to the first sub-node
   **/
  node_iterator nodesBegin() { return m_Nodes.begin(); }

  /**
   * @return a const iterator one past the last sub-node
   **/
  const_node_iterator nodesBegin() const { return m_Nodes.begin(); }

  /**
   * @return a const reverse-iterator to the last sub-node
   **/
  const_node_reverse_iterator nodesRBegin() const { return m_Nodes.rbegin(); }

  /**
   * @return an iterator one past the last sub-node
   **/
  node_iterator nodesEnd() { return m_Nodes.end(); }

  /**
   * @return a const-iterator one past the last sub-node
   **/
  const_node_iterator nodesEnd() const { return m_Nodes.end(); }

  /**
   * @return a const reverse-iterator one past the first sub-node
   **/
  const_node_reverse_iterator nodesREnd() const { return m_Nodes.rend(); }

  /**
   * @return iterator to node with the specified data
   **/
  node_iterator nodeFind(const NodeData &data) { Node temp; temp.setData(data); return m_Nodes.find(&temp); }

  /**
   * @return iterator to node with the specified data
   **/
  const_node_iterator nodeFind(const NodeData &data) const { Node temp; temp.setData(data); return m_Nodes.find(&temp); }

  /**
   * @brief erase the leaf at the specfied iterator
   * @return an iterator to the following leaf
   **/
  leaf_iterator erase(leaf_iterator iter) { return m_Leafs.erase(iter); }

  /**
   * @brief erase the node at the specfied iterator. its content is deleted!
   * @return an iterator to the following node
   **/
  node_iterator erase(node_iterator iter) { delete *iter; return m_Nodes.erase(iter); }

  /**
   * @brief erase the node at the specfied iterator. its content is deleted!
   * @return an iterator to the following node
   **/
  const_node_reverse_iterator erase(const_node_reverse_iterator iter) {
    delete *iter;
    const_node_iterator next = m_Nodes.erase((++iter).base());
    return const_node_reverse_iterator(next); }

  /**
   * @brief remove the node at the specfied iterator but don't delete the content
   * @return an iterator to the following node
   **/
  node_iterator detach(node_iterator iter) { return m_Nodes.erase(iter); }

  /**
   * @return the parent of this node. may be nullptr
   **/
  const MyTree<LeafT, NodeData> *getParent() const { return m_Parent; }

  /**
   * @return Full pathname of a node or leaf of a node
   **/
  QDLLEXPORT QString getFullPath(LeafT const *leaf = nullptr) const;

private:

  const MyTree<LeafT, NodeData> *m_Parent;
  NodeData m_Data;

  std::set<LeafT> m_Leafs;
  std::set<Node*, ByNodeData> m_Nodes;

};


template <typename LeafT, typename NodeData>
MyTree<LeafT, NodeData>::~MyTree()
{
  for (typename std::set<Node*, ByNodeData>::iterator iter = m_Nodes.begin(); iter != m_Nodes.end(); ++iter) {
    delete *iter;
  }
  m_Nodes.clear();
}


template <typename LeafT, typename NodeData>
MyTree<LeafT, NodeData>::MyTree(const MyTree<LeafT, NodeData> &reference)
  : m_Data(reference.m_Data), m_Leafs(reference.m_Leafs)
{
  for (auto iter = reference.m_Nodes.begin(); iter != reference.m_Nodes.end(); ++iter) {
    auto temp = iter->copy();
    temp->m_Parent(this);
    addNode(temp, false);
  }
}


template <typename LeafT, typename NodeData>
MyTree<LeafT, NodeData> &MyTree<LeafT, NodeData>::operator=(const MyTree<LeafT, NodeData> &reference)
{
  if (this != &reference) {
    m_Data = reference.m_Data;
    m_Leafs = reference.m_Leafs;

    for (typename std::set<Node*, ByNodeData>::iterator iter = m_Nodes.begin(); iter != m_Nodes.end(); ++iter) {
      delete *iter;
    }
    m_Nodes.clear();

    for (auto iter = reference.m_Nodes.begin(); iter != reference.m_Nodes.end(); ++iter) {
      Node *temp = (*iter)->copy();
      temp->m_Parent = this;
      addNode(temp, false);
    }
  }
  return *this;
}


template <typename LeafT, typename NodeData>
MyTree<LeafT, NodeData> *MyTree<LeafT, NodeData>::copy() const
{
  MyTree<LeafT, NodeData> *result = new MyTree<LeafT, NodeData>();

  result->m_Data = this->m_Data;
  result->m_Leafs = this->m_Leafs;
  for (auto iter = this->m_Nodes.begin(); iter != this->m_Nodes.end(); ++iter) {
    Node *temp = (*iter)->copy();
    temp->m_Parent = this;
    result->addNode(temp, false);
  }

  return result;
}


template <typename LeafT, typename NodeData>
bool MyTree<LeafT, NodeData>::addNode(Node *node, bool merge, Overwrites *overwrites)
{
  std::pair<typename std::set<Node*, ByNodeData>::iterator, bool> res = m_Nodes.insert(node);
  if (res.second) {
    // no merge required
    node->m_Parent = this;
    return true;
  } else if (!res.second && merge) {
    // merge required
    for (node_iterator iter = node->nodesBegin(); iter != node->nodesEnd();) {
      Node *subNode = *iter;
      iter = node->detach(iter);
      (*res.first)->addNode(subNode, merge, overwrites);
    }
    for (leaf_iterator iter = node->leafsBegin(); iter != node->leafsEnd(); ++iter) {
      (*res.first)->addLeaf(*iter, true, overwrites);
    }
    return true;
  }
  // node exists and merge was disabled
  return false;
}

} // namespace MOBase

#endif // MYTREE_H
