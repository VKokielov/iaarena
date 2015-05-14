
#include <iostream>
#include "BinaryTree.h"

template<typename ThreadPolicy>
void dstruct::bin_tree_sample::ops<ThreadPolicy>::print_node(std::ostream& os, node<ThreadPolicy>* n)
{
	os << n->m_key;
}

template<typename ThreadPolicy>
void dstruct::bin_tree_sample::add_to_bst(long key, dstruct::bin_tree_sample::node<ThreadPolicy>*& in_out_root,
	dstruct::bin_tree_sample::node<ThreadPolicy>*& out_node)
{
	// Need to add a node and find its parent.
	out_node = new node();
	out_node->m_edges[0] = out_node->m_edges[1] = nullptr;
	out_node->m_key = key;

	if (in_out_root == nullptr) {
		in_out_root = out_node;
		return;
	}

	// Otherwise find the parent
	node* parent = in_out_root;
	bool findpl = true;

	ichild dir = CHILD_FINAL;
	while (true) {
		dir = key <= parent->m_key ? CHILD_LEFT : CHILD_RIGHT;

		node* next_parent = parent->m_edges[dir];
		findpl = next_parent != nullptr;

		if (findpl) {
			parent = next_parent;
		} else {
			break;
		}
	}

	parent->m_edges[dir] = out_node;
	out_node->m_edges[LABEL_PARENT] = parent;
	// done
}

template struct dstruct::bin_tree_sample::ops<foundation::tp_single_thread>;