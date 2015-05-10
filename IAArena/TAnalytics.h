#ifndef _IA_TANALYTICS_H_
#define _IA_TANALYTICS_H_

#include <vector>
#include <type_traits>
#include "FError.h"
#include "Inputs.h"

namespace foundation
{
	// Theoretical analytics
	enum af_type {
		F_INT,
		F_FLOAT
	};

	struct af_result
	{
		af_type m_type;
		union {
			double m_float;
			unsigned long m_int;
		};
	};

	template<typename Measure, dim_t Dims>
	class abase {
	public:
		static const dim_t s_dimension = Dims;  // OK; this is an integral type
	};

	template<typename Measure, dim_t Dims>
	class afunction : public abase
	{
	public:
		virtual bool eval(const mvector<Measure>* vecm, af_result& result) = 0;
	};

	/* Each algorithm gets an analytics counter for its inputs.
	   The trick is to provide an interface to some subset of its dimensions, with a mapping.
	   We can do this relatively easily, using the same principles as std::bind.
	  
	   But the mapping should be constructed beforehand, rather than during execution. */

	template<dim_t Dims>
	class acounter_map
	{
	public:
		acounter_map()
		{
			// Construct an identity mapping
			for (dim_t d = 0; d < Dims; d++)
			{
				m_mapping[d] = d;
			}
		}

		acounter_map(const acounter_template& rhs)
		{
			memcpy(m_mapping, rhs.m_mapping, sizeof(m_mapping));  // that's all
		}

		template<dim_t Dimz>
		acounter_map(const acounter_map<Dimz>& bmap, const dim_t* map)
		{
			if (Dims > Dimz)
			{
				throw foundation_exception("Creating acounter_map -- cannot create partial map.")
			}

			for (dim_t d = 0; d < Dims; d++)
			{
				dim_t loc = map[2 * d];
				dim_t target = map[2 * d + 1];

				if (loc >= Dims) {
					throw foundation_exception("Creating acounter_map -- my index is out of bounds.")
				}
				
				if (target >= Dimz) {
					throw foundation_exception("Creating acounter_map -- source map index is out of bounds.")
				}
				
				/* Now, compose the mapping.  We do this by looking up the final value of target.
			      Note: we need to retrieve it, since we have no special access! */
				m_mapping[loc] = bmap.lookup(target); 
			}
		}

		dim_t lookup(dim_t s) const
		{
#ifdef _STRICT_CHECKS
			if (s >= Dims) {
				throw new foundation_exception("acounter_map::lookup -- index out of bounds.");
			}
#endif
			return m_mapping[s];
		}

		const dim_t* get_mapping() const { return m_mapping; }
	private:
		dim_t m_mapping[Dims];
	};

	/* We have two kinds of counters: a real, full one used for the entire algorithm, 
	and a pair formed by a wrapper with a mapping. */
	template<dim_t Dims>
	class acounter_base
	{
	public:
		static const acounter_map<Dims>& get_id_map()
		{
			return sm_map_identity;
		}
	private:
		static acounter_map<Dims> sm_map_identity;
	};

	template<dim_t Dims>
	acounter_map<Dims> acounter_base::sm_map_identity;

	template<dim_t Dims>
	class acounter : public acounter_base<Dims> {
	public:

		using acounter_base::get_id_map;

		acounter()
		{
			clear_counter();
		}

		void clear_counter() {
			for (unsigned int& m : m_counters) { m = 0; }
		}

		void clear_counter_dim(dim_t d)
		{
#ifdef _STRICT_CHECKS
			if (s >= Dims) {
				throw new foundation_exception("acounter::clear_counter_dim -- index out of bounds.");
			}
#endif
			m_counters[d] = 0;
		}

		void add_to_counter(unsigned int delta, dim_t d = 0)
		{
#ifdef _STRICT_CHECKS
			if (s >= Dims) {
				throw new foundation_exception("acounter::add_to_counter -- index out of bounds.");
			}
#endif
			m_counters[d] += delta;
		}
	private:
		unsigned int  m_counters[Dims];
	};

	template<dim_t Dimz, dim_t Dims>
	class acounter_view 
	{
	public:
		acounter_view(const acounter_map<Dimz>* map, acounter<Dims>* base)
			:m_map(map),
			m_base(base)
		{ }

		void clear_counter()
		{
			const dim_t* cmapping = m_map.get_mapping();
			for (dim_t d = 0; d < Dimz; d++)
			{
				m_base->clear_counter_dim(cmapping[d]);
			}
		}
		
		void clear_counter_dim(dim_t d)
		{
#ifdef _STRICT_CHECKS
			if (s >= Dimz) {
				throw new foundation_exception("acounter_view::clear_counter_dim -- index out of bounds.");
			}
#endif
			return m_base->clear_counter_dim(m_map->lookup(d));
		}

		void add_to_counter(unsigned int delta, dim_t d = 0)
		{
#ifdef _STRICT_CHECKS
			if (s >= Dimz) {
				throw new foundation_exception("acounter_view::add_to_counter -- index out of bounds.");
			}
#endif
			m_base->add_to_counter(delta, m_map->lookup(d));
		}
	private:
		const acounter_map<Dimz>*  m_map;
		acounter<Dims>*  m_base;
	};

	// The class below helps build and visualize recursion trees.  It incorporates a counter.
	// NOTE: No shared pointers here.
	struct arecursion_tree_node 
	{
		arecursion_tree_node* m_parent;
		std::vector<arecursion_tree_node*> m_children;
		unsigned int m_step_count;

		arecursion_tree_node()
			:m_parent(nullptr),
			m_step_count(0)
		{ }

		~arecursion_tree_node()
		{
			for (arecursion_tree_node* c : m_children) {
				delete c;
			}

			// Invalidate myself in the parent's vector.  It's worth it
			if (m_parent)
			{
				m_parent->invalidate_child(this);
			}
		}

		arecursion_tree_node(arecursion_tree_node* p)
			:m_parent(p)
		{ }

	private:
		void invalidate_child(arecursion_tree_node* child)
		{
			// Note: we preserve order here, so we can't shuffle nodes around
			for (arecursion_tree_node*& c : m_children) {
				if (c == child) {
					c = nullptr;
				}
			}
		}

	};

	// This data structure is built up as we pop out
	struct arecursion_tree_level
	{
		arecursion_tree_level* m_deeper;
		arecursion_tree_level* m_shallower;

		arecursion_tree_node* m_last_visited;  // used for the top node 
		unsigned int m_level_sum;		
		bool m_owner;

		arecursion_tree_level(bool owner)
			:m_shallower(nullptr),
			m_deeper(nullptr),
			m_last_visited(nullptr),
			m_level_sum(0),
			m_owner(owner)  // maintain this faithfully
		{ }

		~arecursion_tree_level()
		{
			if (m_owner) {
				// Go to the end and delete one by one
				arecursion_tree_level* plast = this;
				while (plast->m_deeper) {
					plast = plast->m_deeper;
				}

				// Now destroy
				while (plast != nullptr) {
					plast = plast->m_shallower;
					delete plast->m_deeper;
				}

				// Don't want to destroy myself -- I am being destroyed

				// Destroy the tree also
				delete m_last_visited;  // this is the root of the tree
			}
		}
	};

	class arecursion_null_counter
	{
	public:
		arecursion_null_counter()
		{ }

		inline void add_to_counter(unsigned int m, dim_t d)
		{ }  // do nothing
	};

	template<typename Counter = arecursion_null_counter>
	class arecursion_tree_builder
	{
	private:
//		using arecursion_tree_level_t = arecursion_tree_level<Measure>;
//		using arecursion_tree_node_t = arecursion_tree_node<Measure>;
	public:
		arecursion_tree_builder(const std::shared_ptr<Counter>& counter)
			:m_counter(counter),  // possibly nullptr
			m_level(new arecursion_tree_level_t(true))
		{ }

		// Call when entering a recursive function
		void push()
		{
			// I am somewhere.  I need to create the next child -- perhaps the first child
			arecursion_tree_node_t* nn = new arecursion_tree_node_t();

			if (m_node_stack.size() > 0)
			{
				arecursion_tree_node_t* cur = m_node_stack.back();
				cur->m_children.push_back(nn);
				nn->m_parent = cur;
			}

			m_node_stack.push_back(nn);

			// Create or get the next level
			arecursion_tree_level_t* nl = m_level->m_deeper;
			if (nl == nullptr)
			{
				nl = new arecursion_tree_level_t(false);
				m_level->m_deeper = nl;
				nl->m_shallower = m_level;
			}
		}

		void pop()
		{
#ifdef _STRICT_CHECKS
			if (m_node_stack.size () == 0) {
				throw new foundation_exception("arecursion_tree_builder::pop -- cannot pop without nodes.");
			}
#endif

			// Add the total to the level count
			m_level->m_level_sum += m_node_stack.back()->m_step_count;
			m_level->m_last_visited = m_node_stack.back();

			// Return
			m_node_stack.pop_back();
		}

		void add_to_counter(unsigned int delta, dim_t d = 0)
		{
#ifdef _STRICT_CHECKS
			if (d != 0) {
				throw new foundation_exception("arecursion_tree_builder::add_to_counter -- index must be 0.");
			}

			if (m_node_stack.size() == 0) {
				throw new foundation_exception("arecursion_tree_builder::add_to_counter -- must first call push.");
			}
#endif

			m_counter->add_to_counter(delta, 1);
			m_node_stack.back()->m_step_count += delta;
		}

		std::shared_ptr<arecursion_tree_level_t> get_tree() { 
			return m_level;
		}  
	private:
		bool m_tree_disowned;
		std::shared_ptr<Counter> m_counter;  // a one-dimensional counter.  Its measure should be the same as Measure
		std::vector<arecursion_tree_node_t*>  m_node_stack;
		std::shared_ptr<arecursion_level_t> m_level;
	};

	// Print a recursion tree in various ways
	// The simplest way is to go down the levels and print the sums

	void print_arecursion_tree_levels(const arecursion_tree_level* base);

	// Next, we can print a tree using a stack 

	// The most fundamental type of analytic function is an actual 

}
#endif