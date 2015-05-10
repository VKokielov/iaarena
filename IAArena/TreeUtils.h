#ifndef _IA_TREE_UTILS
#define _IA_TREE_UTILS

#include "FError.h"
#include "Traversal.h"

namespace dstruct
{
	namespace tree_utils
	{
		template<class TO>
		bool has_node_labeled(typename TO::node_handle n, typename TO::node_label lbl)
		{
			// Try the quick way
			EExists node_exists = TO::peek_node_labeled(n, lbl);
			if (node_exists == EUNKNOWN) 
			{
				// Have to get the node to find out
				return TO::get_node_labeled(n, lbl) != nullptr;
			}
			else 
			{
				return node_exists == EXISTS;
			}
		}

		template<class TO>
		void swap_node(typename TO::node_handle_t to, typename TO::node_handle_t from,
			typename TO::node_label_t label)
		{
			// Move the child of a node, preserving the order
#ifdef _STRICT_CHECKS
			if (has_node_labeled<TO>(to, label))
			{
				throw foundation::foundation_exception("swapping into existing node!", "swap_node');
			}
#endif
			// Detach and reattach
			node_handle_t nf = TO::detach_node(from, label);
			TO::attach_node(to, label, nf);
		}
	}
}

#endif