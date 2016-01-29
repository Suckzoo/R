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
private:
	typedef std::size_t Size;
	typedef BitArray<BaseInt, TotalBits> SelfType;

	constexpr static Size BUCKETS()
	{
		return ((Size) (TotalBits / (sizeof(BaseInt) * 8)));
	}
	constexpr static Size BIT_COUNT()
	{
		return ((Size) (sizeof(BaseInt) * 8));
	}
	constexpr static BaseInt LOW_FLAG()
	{
		return ((BaseInt) 1);
	}
	constexpr static BaseInt HIGH_FLAG()
	{
		return ((LOW_FLAG()) << (BIT_COUNT() - 1));
	}
	constexpr static BaseInt ZERO()
	{
		return ((BaseInt) 0);
	}
	constexpr static BaseInt MASK()
	{
		return (~ZERO());
	}

	std::array<BaseInt, BUCKETS()> array;

private:
	constexpr static BaseInt mark_bit(Size index)
	{
		return ((BaseInt) (HIGH_FLAG()) >> (BaseInt) (index));
	}

	constexpr static inline BaseInt fill_left(Size count)
	{
		return ~(MASK() >> count);
	}

	constexpr static inline BaseInt fill_right(Size count)
	{
		return (MASK() >> (BIT_COUNT() - count));
	}

	constexpr static inline Size bucket_index(Size count)
	{
		return count / BIT_COUNT();
	}

	constexpr static inline Size sub_index(Size count)
	{
		return count % BIT_COUNT();
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

	__func__attr__
	BitArray(const SelfType& source)
	{
		this->array = source.array;
	}

	__func__attr__
	~BitArray()
	{
	}

	template<typename OtherInt>
	__func__attr__ bool is_collide(const SelfType& other) const
	{
		for (Size index = 0; index < BUCKETS(); ++index)
			if (this->array[index] & other.array[index])
				return true;
		return false;
	}

	__func__attr__
	void merge(const SelfType& other)
	{
		for (int index = 0; index < BUCKETS(); ++index)
			this->array[index] |= other.array[index];
	}

	__func__attr__
	void intersect(const SelfType& other)
	{
		for (int index = 0; index < BUCKETS(); ++index)
			this->array[index] &= other.array[index];
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
			Size current_fill = std::min(length + start_offset, BIT_COUNT())
					- start_offset;
			BaseInt current_mask = fill_left(current_fill) >> start_offset;
			this->array[current_bucket++] |= current_mask;
			length -= current_fill;
			start_offset = 0;
		}

		while (length > 0)
		{
			Size current_fill = std::min(length, BIT_COUNT());
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
			Size current_fill = std::min(length + start_offset, BIT_COUNT())
					- start_offset;
			BaseInt current_mask = fill_left(current_fill) >> start_offset;
			this->array[current_bucket++] &= (~current_mask);
			length -= current_fill;
			start_offset = 0;
		}

		while (length > 0)
		{
			Size current_fill = std::min(length, BIT_COUNT());
			BaseInt current_mask = fill_left(current_fill);
			this->array[current_bucket++] &= (~current_mask);
			length -= current_fill;
		}
	}

	__func__attr__
	void clear()
	{
		array.fill(ZERO());
	}

	__func__attr__
	void fill()
	{
		array.fill(MASK());
	}

	bool operator==(const SelfType& other) const
	{
		return this->array == other.array;
	}

	bool operator!=(const SelfType& other) const
	{
		return this->array != other.array;
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
