#ifndef _IA_TRAVERSAL_H_
#define _IA_TRAVERSAL_H_

#include <type_traits>
#include <vector>
#include <algorithm> // max
#include "TraversalIface.h"

namespace dstruct
{
	/* Traversers (linearizers) for trees, generally implemented in terms of templates.
		Assumptions for this family:
		1) Nodes are identified by handles
		2) Each node has a list of children enumerable by index and instantly accessible.
	*/


	namespace ttraversal {
		template<typename TO>
		struct node_state {
			typename TO::node_handle   m_node;
			typename TO::node_index    m_index;
			typename TO::node_index    m_next_index;
			typename TO::sequence      m_seq; // if a node is modified, this 
		};

		template<typename TO>
		class tree_tr_base
		{
		protected:
			tree_tr_base() 
			{ }

			std::vector<node_state<TO> > m_nstack;
			int m_depth;
		};

		// A linear condition-based traversal, as when
		// doing an eliminating search.

		// The arrow depends on the result of the direction predicate, which 
		// produces a label

		template<class DirPred, class TO>
		class linear_tr 
		{
		private:
			using node_handle_t = typename TO::node_handle;
			using node_label_t = typename TO::node_label;

		public:
			using tree_ops_t = TO;
			using initializer = DirPred;  // If I needed to pack this, I could do it easily

			// At each node, the arrow is defined by DirPred
			linear_tr(node_handle_t root, DirPred& pred)
				:m_predicate(pred),
				m_depth(-1)
			{
				if (root) {
					go_to(root);
				}
				compute_arrow();
			}

			int depth() const { return m_depth; }  // -1 for "tree traversed"
			node_handle_t node(int h = 0) const { return m_depth != -1 ? m_stack[std::max(m_depth - h, 0)] : nullptr; }
			node_label_t get_arrow() const { return m_next; }
			bool is_trivial() const { return m_depth == -1; }
			
			void refresh_arrow() { 
				if (m_next != TO::sm_invalid_lbl) {
					m_next_node = TO::get_node_labeled(m_stack[m_depth], m_next);  // maybe nullptr
				}
				else {
					m_next_node = nullptr;
				}
			}

			bool next()
			{
				bool has_next = follow_arrow() != nullptr;
				compute_arrow();
				return has_next;  // this is for now
			}
		private:
			void compute_arrow()
			{
				if (m_depth == -1) {  // tree with no root
					m_next = TO::sm_invalid_lbl;
				}
				else {
					m_next = m_predicate(m_stack[m_depth], m_depth);
				}
				refresh_arrow();
			}

			node_handle_t follow_arrow()
			{
				node_handle_t nn = m_next_node;
				go_to(m_next_node);
				return nn;
			}
			
			void go_to(node_handle_t nh)
			{
				m_depth++;
				if ((int)m_stack.size() <= m_depth)
				{
					m_stack.emplace_back();
				}
				m_stack[m_depth] = nh;
			}

			DirPred m_predicate;
			int m_depth;
			node_label_t m_next;
			node_handle_t m_next_node;
			std::vector<node_handle_t> m_stack;
		};

		template<typename TO>
		class child_order_tr : private tree_tr_base<TO>
		{
		private:
			using node_state_t = node_state<TO>;
			using node_handle_t = typename TO::node_handle;
			using node_index_t = typename TO::node_index;
			using node_label_t = typename TO::node_label;

			inline void fail_fast() const
			{
				if (m_failfast)
				{
					const node_state_t& cur = m_nstack[m_depth];
					if (cur.m_seq != TO::get_seq(cur.m_node))
					{
						throw foundation::foundation_exception("node changed", "child_order_tr::fail_fast");
					}
				}
			}
		public:
			using tree_ops = TO;

			child_order_tr(node_handle_t root, bool failfast = true)
				:m_nstack(TO::tree_depth(root)),  // 0 if unknown works
				m_depth(-1),
				m_failfast(failfast)
			{
				if (root) {
					push_new_node(root);  // root is now the current node and depth is 0
				}
				else {
					m_depth = -2;  // trivial
				}
			}

			int depth() const { fail_fast(); return m_depth; }  // -1 for "tree traversed"
			node_index_t location(int h = 0) const { fail_fast();  return m_nstack[std::max(m_depth - h, 0)].m_index; }  // *within* the node
			node_handle_t node(int h = 0) const { fail_fast(); return m_depth != -1 ? m_nstack[std::max(m_depth - h, 0)].m_node : nullptr; }
			bool is_trivial() const { return m_depth == -2;  }
			node_label_t get_arrow() const 
			{
				node_state_t& cur = m_nstack[m_depth];
				if (TO::is_index_final(cur.m_node, cur.m_next_index)) {
					return TO::sm_parent_lbl;
				}
				else {
					return TO::get_index_label(cur.m_node, cur.m_next_index);
				}
			}

			void refresh_arrow()
			{
				// Refresh the node pointed to by the arrow
			}

			bool next()
			{
				fail_fast();  // have to; what if the children have changed?

				// Fundamentally a three way operation.
				// If we are coming *down*, we need to keep pushing and pushing till we 
				// can go no further, when we turn around and walk up.
				
				// If we are walking up, we increment the state until we bump up against
				// the limit, then walk up again.

				// We store the "next index" in the state and convert it to an arrow on demand.
				// Here we use it to move

				node_state_t& cur = m_nstack[m_depth];

				cur.m_index = cur.m_next_index;  // this was the arrow
				if (TO::is_index_final(cur.m_node, cur.m_index)) {
					// The final index represents the parent arrow.  Move up
					pop_node();
				}
				else {
					push_new_node(TO::get_node_at_index(cur.m_node, cur.m_index));
				}

				// Compute the next index
				compute_arrow();

				return m_depth != -1;
			}
		private:
			void push_new_node(node_handle_t nh)
			{
				m_depth++;
				if (m_depth >= (int)m_nstack.size()) // incremental; difference cannot be more than 1
				{
					m_nstack.emplace_back();
				}

				reset_obj(m_nstack[m_depth], nh);
			}

			void pop_node()
			{
				m_depth--;  // that's really all
			}

			void reset_obj(node_state_t& o, node_handle_t nh)
			{
				TO::init_child_index (nh, o.m_index);
				// Capture the sequence number of the node
				o.m_seq = TO::get_seq(nh);
				o.m_node = nh;
			}

			void compute_arrow()
			{
				node_state_t& cur = m_nstack[m_depth];
				node_index_t ni = cur.m_index;
				// Remember, an arrow points along an edge.  
				// For general graphs, this will be harder
				TO::increment_index(cur.m_node, ni);  // from "one before start" to "first node" -- or "end"
				cur.m_next_index = ni;
			}

			std::vector<node_state_t> m_nstack;
			int m_depth;
			bool m_failfast;
		};

	}
}

#endif