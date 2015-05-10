// IAArena.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <stdlib.h>
#include <string>
#include "BinaryTree.h"
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

	node* root = nullptr;
	node* cur = nullptr;
	for (long k : keys) {
		add_to_bst(k, root, cur);
	}

	// Now print
	std::cout << "Printing..." << std::endl;
	print::tree::print_tree_view<ops>(std::cout, root, "--");
	std::cout << "\n";
}

int _tmain(int argc, _TCHAR* argv[])
{
	// Test 1: build a bst
	bst_test();
	
	return 0;
}

