#ifndef _IA_CONSTRUCTION_H_
#define _IA_CONSTRUCTION_H_

#include <vector>
#include "FError.h"
#include "TreeUtils.h"

namespace dstruct
{
	namespace tconstruction
	{
		/*  Tree construction
			As with traversal, we seek to provide a generic utility for constructing trees.
			Traversal and construction are closely intertwined and often complementary.

			As such, it is advisable for these operations to be able to work with state maintained
			by traversal.

			More generally, we may say that construction takes over where traversal leaves off,
			adding nodes and the edges to them in 
		*/

		template<typename N>
		struct default_end_cons
		{
		public:
			void operator () (N n) { }
		};

		template<typename Tr>
		using default_end_cons_for = default_end_cons<typename Tr::node_handle>;

		template<typename Tr, typename Cons>
		void construct_at_end(typename Tr::tree_ops_t::node_handle& n,
			Cons& constructor,
			typename std::reference_wrapper<typename Tr::initializer> tr_init = typename Tr::initializer())
		{
			using TO = typename Tr::tree_ops_t;
			using node_handle = typename TO::node_handle;

			node_handle new_node = TO::create_free_node();
			if (TO::is_null(n)) {
				n = new_node;
			}
			else {
				Tr traverser(n, tr_init);

				// Go to the end and attach at the arrow
				while (traverser.next());

				TO::attach_node(traverser.node(0), traverser.get_arrow(), new_node);
				// No need to call refresh_arrow, as we are done
			}

			constructor(new_node);
		}
	}
}

#endif