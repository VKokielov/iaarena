#ifndef _IA_THREAD_POLICY
#define _IA_THREAD_POLICY

namespace foundation
{
	struct tp_single_thread
	{
		template<typename I>
		struct atomique {
			using type = I;
		};
	};

	struct tp_multi_thread
	{

	};

	template<typename P, typename I>
	struct atomique {
	};

	template<typename I>
	struct atomique<tp_single_thread, I>
	{
		using type = I;
	};
	
	template<typename I>
	struct atomique<tp_multi_thread, I>
	{
		using type = std::atomic<I>;
	};
}
#endif
