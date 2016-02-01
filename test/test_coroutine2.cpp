/*
 * test_coroutine2.cpp
 *
 *  Created on: 2016. 2. 1.
 *      Author: KHL
 */


#include <cstdio>
#include <gtest/gtest.h>
#include <iostream>
#include <queue>
#include <mutex>
#include <R/coroutine2.hpp>

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

using namespace R;
TEST(Coro2, BasicVoid)
{
	SafeQueue<int> result;

	Context ctx;

	std::cout<<"main 1"<<std::endl;
	ctx.run();
	std::cout<<"main 2"<<std::endl;
	ctx.run();
	std::cout<<"main 3"<<std::endl;
	ctx.run();
	std::cout<<"main 4"<<std::endl;
	ctx.run();
	ctx.run();
	ctx.run();
	ctx.run();
}
