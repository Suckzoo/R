/*
 * test_bitarray.cpp
 *
 *  Created on: 2016. 1. 29.
 *      Author: KHL
 */

#ifndef DEBUG
#define DEBUG
#endif

#include <cstdio>
#include <gtest/gtest.h>
#include <R/bit_array.hpp>


using namespace R;

static const size_t NUM_BITS = 256;
typedef BitArray<uint64_t, NUM_BITS> Type1;
typedef BitArray<uint32_t, NUM_BITS> Type2;
typedef BitArray<uint16_t, NUM_BITS> Type3;
typedef BitArray<uint8_t, NUM_BITS> Type4;

TEST(BitArrayTest, Equal_Empty)
{
	Type1 array1(false);
	Type2 array2(false);
	Type3 array3(false);
	Type4 array4(false);

	EXPECT_TRUE(array1 == array1);
	EXPECT_TRUE(array2 == array2);
	EXPECT_TRUE(array3 == array3);
	EXPECT_TRUE(array4 == array4);
}

TEST(BitArrayTest, Equal_Full)
{
	Type1 array1(true);
	Type2 array2(true);
	Type3 array3(true);
	Type4 array4(true);

	EXPECT_TRUE(array1 == array1);
	EXPECT_TRUE(array2 == array2);
	EXPECT_TRUE(array3 == array3);
	EXPECT_TRUE(array4 == array4);
}

TEST(BitArrayTest, Different)
{
	Type1 array1(true);
	Type2 array2(true);
	Type3 array3(true);
	Type4 array4(true);

	Type1 array1_(false);
	Type2 array2_(false);
	Type3 array3_(false);
	Type4 array4_(false);

	EXPECT_TRUE(array1 != array1_);
	EXPECT_TRUE(array2 != array2_);
	EXPECT_TRUE(array3 != array3_);
	EXPECT_TRUE(array4 != array4_);

	EXPECT_FALSE(array1 == array1_);
	EXPECT_FALSE(array2 == array2_);
	EXPECT_FALSE(array3 == array3_);
	EXPECT_FALSE(array4 == array4_);
}
