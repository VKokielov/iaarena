#ifndef _IA_IO_UTILS_H_
#define _IA_IO_UTILS_H_

#include <sstream>
#include "Traversal.h"

namespace print 
{
	// Use a traverser and a printer object to print out a tree line by line

	namespace tree {
		
		// Use a traverser to print out a tree in "TreeView" format
		template<typename TO>
		void print_tree_view(std::ostream& os, typename TO::node_handle root,
			const char* depth_marker = "\t", const char* node_break = "\n")
		{
			dstruct::ttraversal::child_order_tr<TO> trav(root);

			bool proceed = trav.depth () >= 0;
			while (proceed)
			{
				// Just print out the node text pre-order
				auto cur_node_index = trav.location(0);
				auto cur_node = trav.node(0);

				bool pre = TO::is_index_pre(cur_node, cur_node_index);

				if (pre) {
					for (int d = 0; d < trav.depth(); d++) {
						os << depth_marker;
					}
					TO::print_node(os, trav.node());
					os << node_break;
				}

				proceed = trav.next();
			}
		}

		// Use a traverser to print out a tree in LISP format
		template<typename TO>
		void print_lisp_tree(std::ostream& os, typename TO::node_handle root)
		{
			dstruct::ttraversal::child_order_tr<TO> trav(root);

			while (trav.depth() >= 0)
			{
				// The separator we print depends on our position
				auto cur_node_index = trav.location(0);
				auto cur_node = trav.node(0);

				decltype(cur_node_index) pindex;
				auto pnode = trav.depth() > 0 ? trav.node(1) : nullptr;

				if (pnode) {
					pindex = trav.location(1);
				}

				bool leaf = TO::is_leaf(cur_node);
				bool pre = TO::is_index_pre(cur_node, cur_node_index);
				bool post = TO::is_index_post(cur_node, cur_node_index);  // possibly both
				bool left_paren = !leaf || (pnode && TO::is_index_first(pnode, pindex));
				bool right_paren = !leaf || (pnode && TO::is_index_post(pnode, pindex));

				//os << "leaf " << leaf << " lparen " << left_paren << " rparen " << right_paren << std::endl;
				if (pre) {
					if (left_paren) {
						os << " (";
					}
					else {
						os << " ";
					}
				}
				// Print only once, on pre
				if (pre) {
					TO::print_node(os, trav.node());
				}

				if (post && right_paren) {
					os << ")";
				}

				trav.next();
			}
		}
	}
}
#endif