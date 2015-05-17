#ifndef _IA_BINARY_TREE_H_F
#define _IA_BINARY_TREE_H_

#include <iosfwd>
#include <atomic>
#include "FError.h"
#include "TraversalIface.h"
#include "ThreadPolicy.h"

namespace dstruct
{
	// Use this binary tree structure and associated ops struct as a reference when defining trees

	namespace bin_tree_sample
	{
		enum ichild {CHILD_PRE = -1, 
			CHILD_LEFT = 0, 
			CHILD_RIGHT = 1, 
			CHILD_FINAL = 3  // a parent is not a child
		};

		enum ilabel {
			LABEL_INVALID = -2,
			LABEL_LEFT = 0,
			LABEL_RIGHT = 1,
			LABEL_PARENT = 2  // to avoid confusion
		};

		template<typename ThreadPolicy = foundation::tp_single_thread>
		struct node
		{
			using sequence_t = typename foundation::atomique<ThreadPolicy,unsigned long>::type;
			long m_key;
			node* m_edges[3];  // three -- left-child, right-child, parent
			sequence_t m_sequence;

			node()
				:m_key(0),
				m_sequence(0)
			{
				m_edges[LABEL_LEFT] = m_edges[LABEL_RIGHT] = m_edges[LABEL_PARENT] = nullptr;
			}
		};

		template<typename ThreadPolicy = foundation::tp_single_thread>
		struct ops
		{
			using mnode = node<ThreadPolicy>;
			using node_handle = mnode*;
			using node_index = ichild;
			using node_label = ilabel;
			using sequence = typename mnode::sequence_t;

			static const ilabel sm_parent_lbl = LABEL_PARENT;
			static const ilabel sm_invalid_lbl = LABEL_INVALID;

			static inline void check_label(ilabel lbl, const char* context)
			{
				if (lbl != LABEL_LEFT && lbl != LABEL_RIGHT && lbl != LABEL_PARENT)
				{
					std::string exc_c("bin_tree_ops::");
					exc_c.append(context);
					throw foundation::foundation_exception("label not valid", exc_c.c_str());
				}
			}

			static inline bool is_null(mnode* n) { return n == nullptr; }
			static inline bool is_index_pre(mnode*, ichild idx) { return idx == CHILD_PRE; }
			static inline sequence get_seq(mnode* n) { return n->m_sequence; }

			static inline bool is_index_first(mnode* n, ichild idx) {
				if (n->m_edges[CHILD_LEFT]) {
					return idx == CHILD_LEFT;
				}

				else if (n->m_edges[CHILD_RIGHT]) {
					return idx == CHILD_RIGHT;
				}
				// No first child in a childless tree
				return false;
			}

			static inline bool is_index_post(mnode* n, ichild idx) {
				if (n->m_edges[CHILD_RIGHT]) {
					return idx == CHILD_RIGHT;
				}
				else {
					if (n->m_edges[CHILD_LEFT]) {
						return idx == CHILD_LEFT;
					}
					else {
						return idx == CHILD_PRE;
					}
				}
			}

			static inline bool is_index_final(mnode*, ichild idx)
			{
				return idx == CHILD_FINAL;
			}

			static inline bool is_leaf(mnode* n)
			{
				return n->m_edges[CHILD_LEFT] == nullptr
					&& n->m_edges[CHILD_RIGHT] == nullptr;
			}
			
			static inline int tree_depth(mnode* n) { return 0; }

			static inline void init_child_index(mnode* n, ichild& idx)
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
				return CHILD_FINAL;
			}
			
			static inline void increment_index(mnode* n, ichild& idx)
			{
#ifdef _STRICT_CHECKS
				if (idx >= CHILD_FINAL) {
					throw foundation::foundation_exception("bintree ops increment index-- index too large.");
				}
#endif
				// (1) Get the next value
				idx = get_next_index(idx);

				while (idx != CHILD_FINAL && n->m_edges[idx] == nullptr)
				{
					idx = get_next_index(idx);
				}
			}

			// This function is used when we need to peek a mnode without getting it
			static inline EExists peek_node_labeled(mnode* n, ilabel label)
			{
#ifdef _STRICT_CHECKS
				check_label(label, "peek_node_labeled");
#endif
				return n->m_edges[label] != nullptr ? EXISTS : UNEXISTS;
			}

			// This function is used when we need to get the node
			static inline mnode* get_node_labeled(mnode* n, ilabel lbl)
			{
				// TODO: Use perfect forwarding to wrap these checks
#ifdef _STRICT_CHECKS
				check_label(lbl, "get_node_labeled");
#endif
				return n->m_edges[lbl];
			}

			static inline mnode* get_node_at_index(mnode* n, ichild idx)
			{
#ifdef _STRICT_CHECKS
				if (idx != CHILD_LEFT && idx != CHILD_RIGHT) {
					throw foundation::foundation_exception("index invalid.", "bintree_ops::get_node_at_index");
				}
#endif
				return n->m_edges[idx];
			}

			static inline mnode* create_free_node()
			{
				return new mnode();
			}

			static inline mnode* detach_node(mnode* n, ilabel lbl)
			{
#ifdef _STRICT_CHECKS
				check_label(lbl, "detach_node");
#endif

				// Determine who is the child and who is the parent
				mnode* p = nullptr;
				mnode* c = nullptr;
				ilabel lbl_detach = LABEL_INVALID;
				mnode* r_node = nullptr;  // the node to be returned
				if (lbl == LABEL_PARENT)
				{
					r_node = p = n->m_edges[LABEL_PARENT];
					c = n;

					// Determine the label to detach
					if (p->m_edges[LABEL_LEFT] == n) 
					{
						lbl_detach = LABEL_LEFT;
					} 
					else 
					{
						lbl_detach = LABEL_RIGHT;  // assuming that integrity has not been compromised
					}
				}
				else 
				{
					r_node = c = n->m_edges[lbl];
					p = n;
					lbl_detach = lbl;
				}

#ifdef _STRICT_CHECKS
				// Neither p nor c can be null
				if (p == nullptr) {
					throw foundation::foundation_exception("detaching nonexistent parent node", "bintree_ops::detach_node");
				}

				if (c == nullptr) {
					throw foundation::foundation_exception("detaching nonexistent child node", "bintree_ops::detach_node");
				}
#endif

				/* We have a problem.  Detaching a node affects two nodes.
				How can the sequence numbers be kept up to date at once?  They can't.
				So we cheat and increment them before we do anything.
				There is still of course a possibility, though eternally slim,
				that the wrong value will be registered if anyone tries to read the node.
				Fail-fast ought to be the rule.
				*/
				p->m_sequence++;
				c->m_sequence++;

				// Detach
				p->m_edges[lbl_detach] = nullptr;
				c->m_edges[LABEL_PARENT] = nullptr;
	
				return r_node;
			}

			static inline void attach_node(mnode* to, ilabel lbl, mnode* n)
			{

#ifdef _STRICT_CHECKS
				check_label(lbl, "attach_node");
#endif
				// Assign relationships
				mnode* p = nullptr;
				mnode* c = nullptr;
				ilabel lbl_insert = LABEL_INVALID;

				if (lbl == LABEL_PARENT) {
					p = n;
					c = to;
				}
				else 
				{
					p = to;
					c = n;
					lbl_insert = lbl;
				}

				// Child cannot have any parents
#ifdef _STRICT_CHECKS
				if (c->m_edges[LABEL_PARENT]) 
				{
					throw foundation::foundation_exception("attaching an attached node",
						"binary_tree_ops::attach_node");
				}
#endif
				// Try to deduce lbl_insert if it is indeterminate
				if (p->m_edges[LABEL_LEFT]) {
					if (p->m_edges[LABEL_RIGHT]) {
#ifdef _STRICT_CHECKS
						throw foundation::foundation_exception("nowhere to attach node",
							"binary_tree_ops::attach_node");
#endif
					}
					else {
						if (lbl_insert == LABEL_INVALID)
							lbl_insert = LABEL_RIGHT;
					}
				}
				else if (lbl_insert == LABEL_INVALID) {
					if (!p->m_edges[LABEL_RIGHT]) {
						throw foundation::foundation_exception("cannot deduce child label",
							"binary_tree_ops::attach_node");
					}
					else {
						lbl_insert = LABEL_LEFT;
					}
				}
				
				// Sequence numbers: as above
				p->m_sequence++;
				c->m_sequence++;  // as is proper

				// Attach
				p->m_edges[lbl_insert] = c;
				c->m_edges[LABEL_PARENT] = p;
			}

			// Must be defined
			static inline ilabel get_index_label(mnode* n, ichild idx) 
			{
				if (idx == CHILD_LEFT) {
					return LABEL_LEFT;
				}
				else if (idx == CHILD_RIGHT) {
					return LABEL_RIGHT;
				}
				else {
					return LABEL_INVALID;
				}
			}

			// Trivial copy/move
			static void copy_index(ichild& to, ichild& from)
			{
				to = from;
			}
			
			static void move_index(ichild& to, ichild& from)
			{
				to = from;
			}
			static void print_node(std::ostream& os, mnode* n);
		};

		// Build this up as a BST
		template<class ThreadPolicy>
		void add_to_bst(long key, node<ThreadPolicy>*& in_out_root, node<ThreadPolicy>*& out_node);

	}


}
#endif