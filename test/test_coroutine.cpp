/*
 * test_coroutine.cpp
 *
 *  Created on: 2016. 1. 30.
 *      Author: KHL
 */


#include <cstdio>
#include <gtest/gtest.h>
#include <R/coroutine.hpp>
#include <iostream>

using namespace R;

void cooperative(YieldType<int> &yield)
{
	std::cout << "Hello";
	yield();
	std::cout << "world" << yield.get();
}

TEST(Coroutine_Test, Empty)
{
	CallType<int> source(cooperative);
	std::cout << ", ";
	source(3);
	std::cout << "!\n";
}
