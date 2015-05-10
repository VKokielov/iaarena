#ifndef _IA_BINARY_TREE_H_
#define _IA_BINARY_TREE_H_

#include <iosfwd>
#include <atomic>
#include "FError.h"

namespace dstruct
{
	// Use this binary tree structure and associated ops struct as a reference when defining trees

	namespace bin_tree_sample
	{
		enum ichild {CHILD_PRE = -1, 
			CHILD_LEFT = 0, 
			CHILD_RIGHT = 1, 
			CHILD_FINAL = 2};

		enum ilabel {
			LABEL_PARENT = -2,  // to avoid confusion
			LABEL_LEFT = 0,
			LABEL_RIGHT = 1
		};

		struct node
		{
			long m_key;
			node* m_children[2];  // binary, hence two
			std::atomic<unsigned long> m_sequence;

			node()
				:m_key(0),
				m_children{ nullptr, nullptr },
				m_sequence(0)
			{}
		};

		struct ops
		{
			using node_handle = node*;
			using node_index = ichild;
			using node_label = ilabel;

			static const ilabel sm_lbl_parent = LABEL_PARENT;

			using node_sequence = std::atomic<unsigned long>;

			static inline bool is_index_pre(node*, ichild idx) { return idx == CHILD_PRE; }

			static inline bool is_index_first(node* n, ichild idx) {
				if (n->m_children[CHILD_LEFT]) {
					return idx == CHILD_LEFT;
				}

				else if (n->m_children[CHILD_RIGHT]) {
					return idx == CHILD_RIGHT;
				}
				// No first child in a childless tree
				return false;
			}

			static inline bool is_index_post(node* n, ichild idx) {
				if (n->m_children[CHILD_RIGHT]) {
					return idx == CHILD_RIGHT;
				}
				else {
					if (n->m_children[CHILD_LEFT]) {
						return idx == CHILD_LEFT;
					}
					else {
						return idx == CHILD_PRE;
					}
				}
			}

			static inline bool is_index_final(node*, ichild idx)
			{
				return idx == CHILD_FINAL;
			}

			static inline bool is_leaf(node* n)
			{
				return n->m_children[CHILD_LEFT] == nullptr
					&& n->m_children[CHILD_RIGHT] == nullptr;
			}
			
			static inline int tree_depth(node* n) { return 0; }

			static inline void init_child_index(node* n, ichild& idx)
			{
				idx = CHILD_PRE;
			}

			static inline ichild get_next_index(ichild c)
			{
				switch (c) {
				case CHILD_PRE: return CHILD_LEFT;
				case CHILD_LEFT: return CHILD_RIGHT;
				case CHILD_RIGHT: return CHILD_FINAL;
				}
			}
			
			static inline void increment_index(node* n, ichild& idx)
			{
#ifdef _STRICT_CHECKS
				if (idx >= CHILD_FINAL) {
					throw foundation::foundation_exception("bintree ops increment index-- index too large.");
				}
#endif
				// (1) Get the next value
				idx = get_next_index(idx);

				while (idx != CHILD_FINAL && n->m_children[idx] == nullptr)
				{
					idx = get_next_index(idx);
				}
			}

			static inline EExists peek_node_labeled(node* n, ichild label)
			{
				if (label == LABEL_PARENT) {
					return EUNKNOWN;  
				} else {
					return n->m_children[label] != nullptr ? EXISTS : UNEXISTS;
				}
			}


			static inline node* get_node_labeled(node* n, ilabel lbl)
			{
#ifdef _STRICT_CHECKS
				try {
					return get_node_at_index(n, label);
				}
				catch (foundation::foundation_exception ob)
				{
					throw foundation::foundation_exception(ob, "bintree_ops::get_node_at_label");
				}
#else
				return get_node_at_index(n, label);
#endif
			}

			static inline node* get_node_at_index(node* n, ichild idx)
			{
#ifdef _STRICT_CHECKS
				if (idx != CHILD_LEFT && idx != CHILD_RIGHT) {
					throw foundation::foundation_exception("index invalid.", "bintree_ops::get_node_at_index");
				}
#endif
				return n->m_children[idx];
			}

			static inline node* create_free_node()
			{
				node* ret = new node();
			}

			static inline node* detach_node(node* p, ilabel lbl)
			{
#ifdef _STRICT_CHECKS
				if (lbl != CHILD_LEFT && lbl != CHILD_RIGHT) {
					throw foundation::foundation_exception("index invalid.", "bintree_ops::detach_node");
				}
#endif
				if (p->m_children[lbl] == nullptr)
				{
					return nullptr;
				}

				/* We have a problem.  Detaching a node affects two nodes.
				 How can the sequence numbers be kept up to date at once?  They can't.
				 So we cheat and increment them before we do anything.
				 There is still of course a possibility, though eternally slim,
				 that the wrong value will be registered if anyone tries to read the node.
				 Fail-fast ought to be the rule.
				*/
				node* r = p->m_children[lbl];

				p->m_children[lbl]->m_sequence++;
				p->m_sequence++;

				p->m_children[lbl] = nullptr;
				return r;
			}

			static inline void attach_node(node* to, ilabel lbl, node* n)
			{
#ifdef _STRICT_CHECKS
				if (to->m_children[lbl] != nullptr)
				{
					throw foundation::foundation_exception("node with label exists", "bintree_ops::attach_node");
				}
#endif
				// Sequence numbers: as above
				to->m_sequence++;
				n->m_sequence++;  // as is proper
				to->m_children[lbl] = n;
			}

			// Must be defined
			static inline ilabel get_index_label(node* n, ichild idx) 
			{
				return idx;
			}
			static inline ilabel get_parent_label(node* n)
			{

			}

			static void print_node(std::ostream& os, node* n);
		};

		// Build this up as a BST
		void add_to_bst(long key, node*& in_out_root, node*& out_node);

	}


}
#endif