/*
 * memory_allocator.hpp
 *
 *  Created on: 2016. 1. 29.
 *      Author: KHL
 */

#ifndef INCLUDE_R_MEMORY_ALLOCATOR_HPP_
#define INCLUDE_R_MEMORY_ALLOCATOR_HPP_


#include <cstdlib>


#ifdef FUNC_ATTR
#define __func__attr__ FUNC_ATTR
#else
#define __func__attr__
#endif


namespace R
{

class Allocator
{
public:
	typedef void* Aux;
	typedef void* Ptr;
	typedef std::size_t Size;

	const static Ptr NullPtr = nullptr;

	__func__attr__ Allocator() { };
	__func__attr__ virtual ~Allocator() { };
	__func__attr__ virtual Aux allocate(Size size, Ptr &addr) = 0;
	__func__attr__ virtual void deallocate(Aux aux) = 0;
};

class DefaultAllocator : public Allocator
{
	__func__attr__ DefaultAllocator() { };
	__func__attr__ virtual ~DefaultAllocator() { };
	__func__attr__ virtual Aux allocate(Size size, Ptr &addr)
	{
		addr = (Ptr)malloc(size);
		Aux allocated = (Aux)addr;
		return allocated;
	}
	__func__attr__ virtual void deallocate(Aux aux)
	{
		Ptr addr = (Aux)aux;
		free(addr);
	}
};

}


#ifdef __func__attr__
#undef __func__attr__
#endif


#endif /* INCLUDE_R_MEMORY_ALLOCATOR_HPP_ */
