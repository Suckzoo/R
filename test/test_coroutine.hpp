/*
 * test_coroutine.cpp
 *
 *  Created on: 2016. 1. 30.
 *      Author: KHL
 */

#include <cstdio>
#include <gtest/gtest.h>
#include <iostream>
#include <queue>
#include <mutex>

template<typename T>
class SafeQueue
{
private:
	std::mutex mutex;
	std::queue<T> queue;

public:
	void pop(void)
	{
		std::lock_guard<std::mutex> guard(mutex);
		queue.pop();
	}

	void push(const T& x)
	{
		std::lock_guard<std::mutex> guard(mutex);
		queue.push(x);
	}

	T front(void)
	{
		std::lock_guard<std::mutex> guard(mutex);
		T ret = queue.front();
		return ret;
	}

	T take(void)
	{
		std::lock_guard<std::mutex> guard(mutex);
		T ret = queue.front();
		queue.pop();
		return ret;
	}

	std::size_t size(void)
	{
		std::lock_guard<std::mutex> guard(mutex);
		auto size = queue.size();
		return size;
	}

	bool empty(void)
	{
		std::lock_guard<std::mutex> guard(mutex);
		auto ret = queue.empty();
		return ret;
	}
};

TEST(Coroutine_Test, BasicInt)
{
	SafeQueue<int> result;

	auto cooperative = [&result](symmetric_coroutine<int>::yield_type &yield)
	{
		result.push(2);
		result.push(yield.get());
		result.push(4);
		yield();
		result.push(6);
		result.push(yield.get());
		result.push(8);
		yield();
		yield();
	};

	symmetric_coroutine<int>::call_type source(cooperative);
	result.push(1);
	source(3);
	result.push(5);
	source(7);

	ASSERT_EQ(8, result.size());
	EXPECT_EQ(1, result.take());
	EXPECT_EQ(2, result.take());
	EXPECT_EQ(3, result.take());
	EXPECT_EQ(4, result.take());
	EXPECT_EQ(5, result.take());
	EXPECT_EQ(6, result.take());
	EXPECT_EQ(7, result.take());
	EXPECT_EQ(8, result.take());
}


TEST(Coroutine_Test, BasicVoid)
{
	SafeQueue<int> result;

	auto cooperative = [&result](symmetric_coroutine<void>::yield_type &yield)
	{
		result.push(2);
		//result.push(yield.get());
		result.push(4);
		yield();
		result.push(6);
		//result.push(yield.get());
		result.push(8);
		yield();
		yield();
	};

	symmetric_coroutine<void>::call_type source(cooperative);
	result.push(1);
	//source(3);
	source();
	result.push(5);
	//source(7);
	source();

	ASSERT_EQ(6, result.size());
	EXPECT_EQ(1, result.take());
	EXPECT_EQ(2, result.take());
	//EXPECT_EQ(3, result.take());
	EXPECT_EQ(4, result.take());
	EXPECT_EQ(5, result.take());
	EXPECT_EQ(6, result.take());
	//EXPECT_EQ(7, result.take());
	EXPECT_EQ(8, result.take());
}
