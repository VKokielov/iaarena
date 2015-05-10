#ifndef _IA_TRAVERSAL_H_
#define _IA_TRAVERSAL_H_

#include <vector>
#include <algorithm> // max

namespace dstruct
{
	/* Traversers (linearizers) for trees, generally implemented in terms of templates.
		Assumptions for this family:
		1) Nodes are identified by handles
		2) Each node has a list of children enumerable by index and instantly accessible.
	*/
	enum EExists {
		EXISTS,
		UNEXISTS,
		EUNKNOWN
	};

	namespace ttraversal {
		template<typename TO>
		struct node_state {
			typename TO::node_handle   m_node;
			typename TO::node_index    m_index;
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
		// doing an eliminating search

		template<typename TO, class DirPred>
		class linear_tr : private tree_tr_base<TO>
		{
			// At each node, the arrow is defined by DirPred

		private:

		};


		template<typename TO>
		class child_order_tr : private tree_tr_base<TO>
		{
		private:
			using node_state_t = node_state<TO>;
			using node_handle_t = typename TO::node_handle;
			using node_index_t = typename TO::node_index;

			inline void fail_fast()
			{
				if (m_failfast) 
				{
					node_state_t& cur = m_nstack[m_depth];
					if (cur.m_seq != TO::get_seq(cur.m_node))
					{
						throw foundation::foundation_exception("node changed", "child_order_tr");
					}
				}
			}
		public:
			using tree_ops = TO;

			child_order_tr(node_handle_t root, bool failfast = true)
				:m_nstack(TO::tree_depth (root)),  // 0 if unknown works
				m_depth(-1),
				m_failfast(failfast)
			{ 
				push_new_node(root);  // root is now the current node and depth is 0
			}

			int depth() const { fail_fast(); return m_depth; }  // -1 for "tree traversed"
			node_index_t location(int h = 0) const { fail_fast();  return m_nstack[std::max(m_depth - h, 0)].m_index; }  // *within* the node
			node_handle_t node(int h = 0) const { fail_fast(); return m_nstack[std::max(m_depth - h, 0)].m_node; }

			bool next()
			{
				fail_fast();  // have to; what if the children have changed?

				// Fundamentally a three way operation.
				// If we are coming *down*, we need to keep pushing and pushing till we 
				// can go no further, when we turn around and walk up.
				
				// If we are walking up, we increment the state until we bump up against
				// the limit, then walk up again.

				node_state_t& cur = m_nstack[m_depth];

				// Increment the child index
				TO::increment_index(cur.m_node, cur.m_index);  // from "one before start" to "first node" -- or "end"
				
				if (TO::is_index_final(cur.m_node, cur.m_index)) {
					pop_node();
					return m_depth != -1;
				} else {
					push_new_node(TO::get_node_at_index(cur.m_node, cur.m_index));
					return true; 
				}
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
		};

	}
}

#endif