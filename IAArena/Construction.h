#ifndef _IA_CONSTRUCTION_H_
#define _IA_CONSTRUCTION_H_

#include <vector>
#include "FError.h"
#include "TreeUtil.h"

namespace dstruct
{
	namespace tconstruct 
	{
		/*  Tree construction
			As with traversal, we seek to provide a generic utility for constructing trees.
			Traversal and construction are closely intertwined and often complementary.

			As such, it is advisable for these operations to be able to work with state maintained
			by traversal.

			More generally, we may say that construction takes over where traversal leaves off,
			adding nodes and the edges to them in 
		*/

		enum class NodeSplice
		{
			NEVER,
			ONE_LEVEL,
			RECURSIVE // does this work?
		};

		// Tr is a label-based traverser
		template<typename Tr>
		class tree_construct {
		private:
			using TO = typename Tr::tree_ops;
			using node_handle_t = typename TO::node_handle;
			using node_label_t = typename TO::node_label;
		public:
			Tr& get_traverser() { return m_traverse; }

			// Pop a tree off the free stack and attach it to the main tree.
			// If a node with the given label exists, we can complain or remedy
			// the situation.  
			void pop_insert(NodeSplice spl)
			{
#ifdef _STRICT_CHECKS
				if (m_free_stack.empty())
				{
					throw foundation::foundation_exception("no free nodes to insert", "tree_construct::pop_insert");
				}
#endif
				node_label_t lbl_next = m_traverser->get_arrow();  // the arrow is the label to be taken next
				node_handle_t n = m_traverser->node(0);
				node_handle_t nf = m_free_stack.back();

				bool needs_splice = has_node_labeled<TO>(n, lbl); // utility function for different implementations

				if (needs_splice) {
					if (spl == NodeSplice::NEVER)
					{
						throw foundation::foundation_exception("trying to splice node in", "tree_construct::pop_insert");
					}

					// Is it possible to "splice in"?  The free node must have space for the child
					// under the same label
					bool can_splice = !has_node_labeled<TO>(nf, lbl);

					if (!can_splice) 
					{
						throw foundation::foundation_exception("cannot splice node -- child is occupied", "tree_construct::pop_insert");
					}

					swap_node<TO>(nf, n, lbl);
				} else {
					// Just insert
					TO::attach_node(nf, n, lbl);
				}

				// Pop off the free stack
				m_free_stack.pop_back();
			}

			node_handle_t push_free_node()
			{
				// Create a free node, push it on the stack, and return a handle to it
				node_handle_t nf = TO::create_free_node();  // initialized properly, of course

				m_free_stack.push_back(nf);
			}

		private:
			Tr* m_traverser;
			std::vector<node_handle_t> m_free_stack;
		};
	}
}
#endif