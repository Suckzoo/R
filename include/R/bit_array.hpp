/*
 * bit_array.hpp
 *
 *  Created on: 2016. 1. 29.
 *      Author: KHL
 */

#ifndef INCLUDE_R_BIT_ARRAY_HPP_
#define INCLUDE_R_BIT_ARRAY_HPP_

#include <type_traits>
#include <algorithm>
#include <ostream>

#ifdef FUNC_ATTR
#define __func__attr__ FUNC_ATTR
#else
#define __func__attr__
#endif

namespace R
{

template<typename BaseInt, std::size_t TotalBits>
class BitArray
{
	static_assert(std::is_integral<BaseInt>::value, "Integer type required.");
	static_assert(std::is_unsigned<BaseInt>::value, "Unsigned type required.");
	static_assert(TotalBits % (sizeof(BaseInt) * 8) == 0,
			"TotalBits must be multiple of the bits in BaseInt");
public:
	typedef std::size_t Size;
	const static Size BUCKETS = (TotalBits / (sizeof(BaseInt) * 8));

private:
	std::array<BaseInt, BUCKETS> array;
	typedef BitArray<BaseInt, TotalBits> SelfType;
	const static Size BIT_COUNT = ((Size) (sizeof(BaseInt) * 8));
	const static Size LOW_FLAG = ((BaseInt) 1);
	const static Size HIGH_FLAG = ((LOW_FLAG) << (BIT_COUNT - 1));
	const static Size ZERO = ((BaseInt) 0);
	const static Size MASK = (~ZERO);

private:
	constexpr static BaseInt mark_bit(Size index)
	{
		return ((BaseInt) (HIGH_FLAG) >> (BaseInt) (index));
	}

	constexpr static inline BaseInt fill_left(Size count)
	{
		return ~(MASK >> count);
	}

	constexpr static inline BaseInt fill_right(Size count)
	{
		return (MASK >> (BIT_COUNT - count));
	}

	constexpr static inline Size bucket_index(Size count)
	{
		return count / BIT_COUNT;
	}

	constexpr static inline Size sub_index(Size count)
	{
		return count % BIT_COUNT;
	}
public:
	__func__attr__
	BitArray() :
			BitArray(false)
	{
	}

	__func__attr__
	BitArray(bool initial)
	{
		if (initial)
			this->fill();
		else
			this->clear();
	}

	template<typename OtherInt>
	__func__attr__ BitArray(const BitArray<OtherInt, TotalBits>& source)
	{
		typedef BitArray<OtherInt, TotalBits> Other;
		if (Other::BIT_COUNT > BIT_COUNT)
		{
			const Size base_in_other = Other::BIT_COUNT / BIT_COUNT;
			Size base_index = 0;
			for (Size other_index = 0; other_index < Other::BUCKETS;
					++other_index)
			{
				OtherInt val = source.array[other_index];
				for (Size base_iter = 0; base_iter < base_in_other; ++base_iter)
				{
					this->array[base_index++] = (BaseInt) (val
							>> (BIT_COUNT * (base_in_other - 1 - base_iter)));
				}
			}
		}
		else if (Other::BIT_COUNT < BIT_COUNT)
		{
			const Size other_in_base = BIT_COUNT / Other::BIT_COUNT;
			Size other_index = 0;
			for (Size base_index = 0; base_index < BUCKETS; ++base_index)
			{
				BaseInt val = ZERO;
				for (Size other_iter = 0; other_iter < other_in_base;
						++other_iter)
				{
					val |= ((BaseInt) source.array[other_index++])
							<< (Other::BIT_COUNT
									* (other_in_base - 1 - other_iter));
				}
				this->array[base_index] = val;
			}
		}
		else
			this->array = source.array;
	}

	__func__attr__
	~BitArray()
	{
	}

	template<typename OtherInt>
	__func__attr__ bool is_collide(
			const BitArray<OtherInt, TotalBits>& other) const
	{
		typedef BitArray<OtherInt, TotalBits> Other;
		if (Other::BIT_COUNT > BIT_COUNT)
		{
			const Size base_in_other = Other::BIT_COUNT / BIT_COUNT;
			Size base_index = 0;
			for (Size other_index = 0; other_index < Other::BUCKETS;
					++other_index)
			{
				OtherInt val = Other::ZERO;
				for (Size base_iter = 0; base_iter < base_in_other; ++base_iter)
				{
					val |= ((OtherInt) this->array[base_index++])
							<< (BIT_COUNT * (base_in_other - 1 - base_iter));
				}
				if (val & other.array[other_index])
					return true;
			}
			return false;
		}
		else if (Other::BIT_COUNT < BIT_COUNT)
		{
			const Size other_in_base = BIT_COUNT / Other::BIT_COUNT;
			Size other_index = 0;
			for (Size base_index = 0; base_index < BUCKETS; ++base_index)
			{
				BaseInt val = ZERO;
				for (Size other_iter = 0; other_iter < other_in_base;
						++other_iter)
				{
					val |= ((BaseInt) other.array[other_index++])
							<< (Other::BIT_COUNT
									* (other_in_base - 1 - other_iter));
				}
				if (val & this->array[base_index])
					return true;
			}
			return false;
		}
		else
		{
			for (Size index = 0; index < BUCKETS; ++index)
				if (this->array[index] & other.array[index])
					return true;
			return false;
		}
	}

	template<typename OtherInt>
	__func__attr__ void merge(const BitArray<OtherInt, TotalBits>& other)
	{
		typedef BitArray<OtherInt, TotalBits> Other;
		if (Other::BIT_COUNT > BIT_COUNT)
		{
			const Size base_in_other = Other::BIT_COUNT / BIT_COUNT;
			Size base_index = 0;
			for (Size other_index = 0; other_index < Other::BUCKETS;
					++other_index)
			{
				OtherInt val = other.array[other_index];
				for (Size base_iter = 0; base_iter < base_in_other; ++base_iter)
				{
					this->array[base_index++] |= (BaseInt) (val
							>> (BIT_COUNT * (base_in_other - 1 - base_iter)));
				}
			}
		}
		else if (Other::BIT_COUNT < BIT_COUNT)
		{
			const Size other_in_base = BIT_COUNT / Other::BIT_COUNT;
			Size other_index = 0;
			for (Size base_index = 0; base_index < BUCKETS; ++base_index)
			{
				BaseInt val = ZERO;
				for (Size other_iter = 0; other_iter < other_in_base;
						++other_iter)
				{
					val |= ((BaseInt) other.array[other_index++])
							<< (Other::BIT_COUNT
									* (other_in_base - 1 - other_iter));
				}
				this->array[base_index] |= val;
			}
		}
		else
		{
			for (int index = 0; index < BUCKETS; ++index)
				this->array[index] |= other.array[index];
		}
	}

	template<typename OtherInt>
	__func__attr__ void intersect(const BitArray<OtherInt, TotalBits>& other)
	{
		typedef BitArray<OtherInt, TotalBits> Other;
		if (Other::BIT_COUNT > BIT_COUNT)
		{
			const Size base_in_other = Other::BIT_COUNT / BIT_COUNT;
			Size base_index = 0;
			for (Size other_index = 0; other_index < Other::BUCKETS;
					++other_index)
			{
				OtherInt val = other.array[other_index];
				for (Size base_iter = 0; base_iter < base_in_other; ++base_iter)
				{
					this->array[base_index++] &= (BaseInt) (val
							>> (BIT_COUNT * (base_in_other - 1 - base_iter)));
				}
			}
		}
		else if (Other::BIT_COUNT < BIT_COUNT)
		{
			const Size other_in_base = BIT_COUNT / Other::BIT_COUNT;
			Size other_index = 0;
			for (Size base_index = 0; base_index < BUCKETS; ++base_index)
			{
				BaseInt val = ZERO;
				for (Size other_iter = 0; other_iter < other_in_base;
						++other_iter)
				{
					val |= ((BaseInt) other.array[other_index++])
							<< (Other::BIT_COUNT
									* (other_in_base - 1 - other_iter));
				}
				this->array[base_index] &= val;
			}
		}
		else
		{
			for (int index = 0; index < BUCKETS; ++index)
				this->array[index] &= other.array[index];
		}
	}

	__func__attr__
	bool get_bit(Size index) const
	{
		auto bucket = bucket_index(index);
		auto sub = sub_index(index);
		auto checker = mark_bit(sub);

		return !!(checker & this->array[bucket]);
	}

	__func__attr__
	void set_bit(Size index)
	{
		auto bucket = bucket_index(index);
		auto sub = sub_index(index);
		auto marker = mark_bit(sub);
		this->array[bucket] |= marker;
	}

	__func__attr__
	void clear_bit(Size index)
	{
		auto bucket = bucket_index(index);
		auto sub = sub_index(index);
		auto marker = mark_bit(sub);
		this->array[bucket] &= (~marker);
	}

	__func__attr__
	void set_range(Size start, Size length)
	{
		Size start_offset = sub_index(start);
		Size current_bucket = bucket_index(start);

		if (start_offset > 0)
		{
			Size current_fill = std::min(length + start_offset, BIT_COUNT)
					- start_offset;
			BaseInt current_mask = fill_left(current_fill) >> start_offset;
			this->array[current_bucket++] |= current_mask;
			length -= current_fill;
			start_offset = 0;
		}

		while (length > 0)
		{
			Size current_fill = std::min(length, BIT_COUNT);
			BaseInt current_mask = fill_left(current_fill);
			this->array[current_bucket++] |= current_mask;
			length -= current_fill;
		}
	}

	__func__attr__
	void clear_range(Size start, Size length)
	{
		Size start_offset = sub_index(start);
		Size current_bucket = bucket_index(start);

		if (start_offset > 0)
		{
			Size current_fill = std::min(length + start_offset, BIT_COUNT)
					- start_offset;
			BaseInt current_mask = fill_left(current_fill) >> start_offset;
			this->array[current_bucket++] &= (~current_mask);
			length -= current_fill;
			start_offset = 0;
		}

		while (length > 0)
		{
			Size current_fill = std::min(length, BIT_COUNT);
			BaseInt current_mask = fill_left(current_fill);
			this->array[current_bucket++] &= (~current_mask);
			length -= current_fill;
		}
	}

	__func__attr__
	void clear()
	{
		array.fill(ZERO);
	}

	__func__attr__
	void fill()
	{
		array.fill(MASK);
	}

	template<typename OtherInt>
	bool operator==(const BitArray<OtherInt, TotalBits>& other) const
	{
		typedef BitArray<OtherInt, TotalBits> Other;
		if (Other::BIT_COUNT > BIT_COUNT)
		{
			const Size base_in_other = Other::BIT_COUNT / BIT_COUNT;
			Size base_index = 0;
			for (Size other_index = 0; other_index < Other::BUCKETS;
					++other_index)
			{
				OtherInt val = other.array[other_index];
				for (Size base_iter = 0; base_iter < base_in_other; ++base_iter)
				{
					if (this->array[base_index++]
							!= (BaseInt) (val
									>> (BIT_COUNT
											* (base_in_other - 1 - base_iter))))
						return false;
				}
			}
			return true;
		}
		else if (Other::BIT_COUNT < BIT_COUNT)
		{
			const Size other_in_base = BIT_COUNT / Other::BIT_COUNT;
			Size other_index = 0;
			for (Size base_index = 0; base_index < BUCKETS; ++base_index)
			{
				BaseInt val = ZERO;
				for (Size other_iter = 0; other_iter < other_in_base;
						++other_iter)
				{
					val |= ((BaseInt) other.array[other_index++])
							<< (Other::BIT_COUNT
									* (other_in_base - 1 - other_iter));
				}
				if (this->array[base_index] != val)
					return false;
			}
			return true;
		}
		else
		{
			return this->array == other.array;
		}
	}

	friend std::ostream& operator<<(std::ostream& os, const SelfType& me)
	{
		for (Size k = 0; k < TotalBits; ++k)
		{
			bool ret = me->get_bit(k);
			os << ret;
		}
		return os;
	}
};

}

#ifdef __func__attr__
#undef __func__attr__
#endif

#endif /* INCLUDE_R_BIT_ARRAY_HPP_ */
