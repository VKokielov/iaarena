
#include <iostream>
#include "BinaryTree.h"

void dstruct::bin_tree_sample::ops::print_node(std::ostream& os, node* n)
{
	os << n->m_key;
}

void dstruct::bin_tree_sample::add_to_bst(long key, dstruct::bin_tree_sample::node*& in_out_root,
	dstruct::bin_tree_sample::node*& out_node)
{
	// Need to add a node and find its parent.
	out_node = new node();
	out_node->m_children[0] = out_node->m_children[1] = nullptr;
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

		node* next_parent = parent->m_children[dir];
		findpl = next_parent != nullptr;

		if (findpl) {
			parent = next_parent;
		} else {
			break;
		}
	}

	parent->m_children[dir] = out_node;
	// done
}