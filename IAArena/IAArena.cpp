// IAArena.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <stdlib.h>
#include <string>
#include "BinaryTree.h"
#include "Construction.h"
#include "IOUtils.h"

void bst_test()
{
	std::cout << "Input numbers, then 'stop'" << std::endl;

	std::vector<long> keys = { 4, 6, 2, 1, 3, 5, 7};
//	std::vector<long> keys = { 1, 2, 3, 4, 5, 6, 7};
#if 0
	std::string ink;

	std::cin >> ink;
	while (ink.compare("stop") != 0)
	{
		long key = atoi(ink.c_str());
		keys.push_back(key);
		std::cin >> ink;
	}
#endif

	using namespace dstruct::bin_tree_sample;
//	using namespace dstruct::tconstruction;
	using ops_t = ops<foundation::tp_single_thread>;
	using node = node<foundation::tp_single_thread>;

	node* root = nullptr;
	node* cur = nullptr;
	for (long k : keys) {
		auto condition = [&](node* bn, int depth) -> ilabel
		{
			if (k <= bn->m_key) {
				return LABEL_LEFT;
			}
			else {
				return LABEL_RIGHT;
			}
		};

		auto initializer = [&](node* n)
		{
			n->m_key = k;
		};

		// A linear traverser to find what we want using the little lambda above
		using l_tr = dstruct::ttraversal::linear_tr <decltype(condition), ops_t>;

		// Construct!
		dstruct::tconstruction::construct_at_end<l_tr>(root, initializer, condition);
			
	}

	// Now print
	std::cout << "Printing..." << std::endl;
	print::tree::print_tree_view<ops_t>(std::cout, root, "--");
	std::cout << "\n";
}

int main(int argc, char* argv[])
{
	// Test 1: build a bst
	bst_test();
	
	return 0;
}

