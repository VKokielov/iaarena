#ifndef _IA_EFFICACY_UTIL
#define _IA_EFFICACY_UTIL

namespace algorithm
{
	// A wrapper for an efficient walk of a sequence with any kind of
	// iterator

	template<typename Iter>
	class ef_walker
	{
		operator[] (int x)
		{

		}
	};
}

namespace dstruct
{
	// An iterable set implemented using a vector and indices

	namespace ds_iterable_set {
		template<typename T>
		class iterable_set;

		template<typename T>
		struct iterable_entry
		{
			size_t m_index;
			iterable_set<T>* m_owner;
			T m_value;

			void deleteMe()
			{
				m_owner->deleteAt(m_index);
			}
		};

		template<typename T>
		class iterable_set
		{
		public:
			using iterator = std::vector<T>::iterator;
			using iset_entry_t = iterable_entry<T>;
			using set_handle_t = std::shared_ptr<iset_entry>;

			friend class iset_entry_t;

			iterable_set() {}

			set_handle insert(const T& v)
			{
				size_t new_index = m_set_.size();
				m_set_.emplace_back();
				// Initialize the entry
				iset_entry_t& new_entry = m_set_.back();

				new_entry.m_index = new_index;
				new_entry.m_owner = this;
				new_entry.m_value = v;

				// Create a pointer with a deleter that calls "remove" in this class, assuming
				// all indices are current and up-to-date
				return std::shared_ptr
					(&new_entry, 
					[](iset_entry_t* o_d)
						{o_d->deleteMe();}
			}

			iterator begin() { return m_set_.begin(); }
			iterator end() { return m_set_.end(); }

		private:
			void deleteAt(size_t index)
			{
				// Delete by swapping.  Update the index of the swapped element
				iset_entry_t& slot = m_set_[index];
				iset_entry_t& last = m_set_.back();

				if (&slot != &last) {
					std::swap(slot, last);
					last.m_index = index;
				}
				m_set_.pop_back();
			}

			std::vector<T> m_set_;
		};

	}
}

#endif