/*
 * shifted_int.hpp
 *
 *  Created on: 2016. 1. 29.
 *      Author: KHL
 */

#ifndef INCLUDE_R_SHIFTED_INT_HPP_
#define INCLUDE_R_SHIFTED_INT_HPP_

#include <cstdint>
#include <type_traits>
#include <limits>
#include <cassert>
#include <exception>

#ifdef FUNC_ATTR
#define __func__attr__ FUNC_ATTR
#else
#define __func__attr__
#endif

namespace R
{

#ifdef DEBUG
class PrecisionLossException : public std::exception
{
private:
	const char* msg;
public:
	PrecisionLossException(const char* msg)
	{
		this->msg = msg;
	}
	virtual const char* what() const throw()
	{
		return this->msg;
	}
};
#endif

template<typename Int, unsigned Shift>
class ShiftedInt
{
private:
	typedef uint64_t LARGE_INT;
	static_assert(std::is_integral<Int>::value, "Integer type required.");

	Int shifted_value;

public:
	template<typename InputInt>
	__func__attr__ ShiftedInt(const InputInt& initial)
	{
		static_assert(std::is_integral<InputInt>::value, "Integer type required.");
#ifdef DEBUG
		if(initial > ((LARGE_INT)std::numeric_limits<Int>::max() << Shift))
		throw PrecisionLossException("input type is too large");
		if((initial & (((LARGE_INT)1 << Shift) -1)) != 0)
		throw PrecisionLossException("lower digits are lost");
#endif
		shifted_value = (initial >> Shift);
	}

	//copy constructor
	__func__attr__
	ShiftedInt(const ShiftedInt& orig)
	{
		shifted_value = orig.shifted_value;
	}

	template<typename InputInt>
	__func__attr__ ShiftedInt& operator=(const InputInt& v)
	{
		static_assert(std::is_integral<InputInt>::value, "Integer type required.");
#ifdef DEBUG
		if(v > ((LARGE_INT)std::numeric_limits<Int>::max() << Shift))
		throw PrecisionLossException("input type is too large");
		if((v & (((LARGE_INT)1 << Shift) -1)) != 0)
		throw PrecisionLossException("lower digits are lost");
#endif
		shifted_value = (Int) (v >> Shift);
		return *this;
	}

	template<typename InputInt>
	__func__attr__ ShiftedInt& operator+=(const InputInt& v)
	{
		static_assert(std::is_integral<InputInt>::value, "Integer type required.");
#ifdef DEBUG
		if(v > ((LARGE_INT)std::numeric_limits<Int>::max() << Shift))
		throw PrecisionLossException("input type is too large");
		if((v & (((LARGE_INT)1 << Shift) -1)) != 0)
		throw PrecisionLossException("lower digits are lost");
#endif
		shifted_value += (Int) (v >> Shift);
		return *this;
	}

	template<typename InputInt>
	__func__attr__ const ShiftedInt operator+(const InputInt& v) const
	{
		static_assert(std::is_integral<InputInt>::value, "Integer type required.");
#ifdef DEBUG
		if(v > ((LARGE_INT)std::numeric_limits<Int>::max() << Shift))
		throw PrecisionLossException("input type is too large");
		if((v & (((LARGE_INT)1 << Shift) -1)) != 0)
		throw PrecisionLossException("lower digits are lost");
#endif
		ShiftedInt ret(*this);
		ret.shifted_value += (Int) (v >> Shift);
		return ret;
	}

	template<typename InputInt>
	__func__attr__ ShiftedInt& operator*=(const InputInt& v)
	{
		static_assert(std::is_integral<InputInt>::value, "Integer type required.");
#ifdef DEBUG
		if(v > ((LARGE_INT)std::numeric_limits<Int>::max() << Shift))
		throw PrecisionLossException("input type is too large");
#endif
		shifted_value *= (Int) (v);
		return *this;
	}

	template<typename InputInt>
	__func__attr__ const ShiftedInt operator*(const InputInt& v) const
	{
		static_assert(std::is_integral<InputInt>::value, "Integer type required.");
#ifdef DEBUG
		if(v > ((LARGE_INT)std::numeric_limits<Int>::max() << Shift))
		throw PrecisionLossException("input type is too large");
#endif
		ShiftedInt ret(*this);
		ret.shifted_value *= (Int) (v);
		return ret;
	}

	template<typename InputInt>
	__func__attr__ bool operator==(const InputInt& v) const
	{
		static_assert(std::is_integral<InputInt>::value, "Integer type required.");
#ifdef DEBUG
		if(v > ((LARGE_INT)std::numeric_limits<Int>::max() << Shift))
		throw PrecisionLossException("input type is too large");
		if((v & (((LARGE_INT)1 << Shift) -1)) != 0)
		throw PrecisionLossException("lower digits are lost");
#endif
		return shifted_value == (Int) (v >> Shift);
	}

	template<typename InputInt>
	__func__attr__ bool operator!=(const InputInt& v) const
	{
		static_assert(std::is_integral<InputInt>::value, "Integer type required.");
#ifdef DEBUG
		if(v > ((LARGE_INT)std::numeric_limits<Int>::max() << Shift))
		throw PrecisionLossException("input type is too large");
		if((v & (((LARGE_INT)1 << Shift) -1)) != 0)
		throw PrecisionLossException("lower digits are lost");
#endif
		return shifted_value != (Int) (v >> Shift);
	}

	__func__attr__
	ShiftedInt& operator=(const ShiftedInt& v)
	{
		shifted_value = v.shifted_value;
		return *this;
	}

	__func__attr__
	ShiftedInt& operator+=(const ShiftedInt& v)
	{
		shifted_value += v.shifted_value;
		return *this;
	}

	__func__attr__
	const ShiftedInt operator+(const ShiftedInt& v) const
	{
		ShiftedInt ret(*this);
		ret.shifted_value += v.shifted_value;
		return ret;
	}

	__func__attr__
	ShiftedInt& operator*=(const ShiftedInt& v)
	{
		shifted_value *= v.shifted_value;
		shifted_value <<= Shift;
		return *this;
	}

	__func__attr__
	const ShiftedInt operator*(const ShiftedInt& v) const
	{
		ShiftedInt ret(*this);
		ret.shifted_value *= v.shifted_value;
		ret.shifted_value <<= Shift;
		return ret;
	}

	__func__attr__
	bool operator==(const ShiftedInt& v) const
	{
		return shifted_value == v.shifted_value;
	}

	__func__attr__
	bool operator!=(const ShiftedInt& v) const
	{
		return shifted_value != v.shifted_value;
	}

	template<typename ReturnInt>
	__func__attr__ ReturnInt as_value() const
	{
		static_assert(std::numeric_limits<ReturnInt>::max()
				>= ((LARGE_INT)std::numeric_limits<Int>::max() << Shift),
				"return type is not large enough.");
		return (ReturnInt) (shifted_value << Shift);
	}
};

}

#ifdef __func__attr__
#undef __func__attr__
#endif

#endif /* INCLUDE_R_SHIFTED_INT_HPP_ */
