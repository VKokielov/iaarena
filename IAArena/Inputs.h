#ifndef _IA_INPUTS_H_
#define _IA_INPUTS_H_

namespace foundation
{
	using dim_t = unsigned int;

	template<class Measure>
	class mvector
	{
	public:
		virtual Measure operator [] (dim_t dim) = 0;
		~mvector();
	};
}
#endif