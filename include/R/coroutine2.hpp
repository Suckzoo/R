/*
 * coroutine2.hpp
 *
 *  Created on: 2016. 2. 1.
 *      Author: KHL
 */

#ifndef INCLUDE_R_COROUTINE2_HPP_
#define INCLUDE_R_COROUTINE2_HPP_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <exception>
#include <cassert>
#include <functional>

namespace R
{

#include <iostream>

class InterruptedException : public std::exception
{
	virtual const char* what() const noexcept {return "InterruptedException";}
};

class Context
{
private:
	std::thread* _runner;
	bool _interrupted;
	bool __inner_finished;
	std::mutex *_mutex;
	std::condition_variable *__inner_barrier;
	std::condition_variable *_barrier;
	std::unique_lock<std::mutex> *_lock;

	std::unique_lock<std::mutex> *__inner_lock;
public:
	Context()
	{
		_interrupted = false;
		__inner_finished = false;

		_mutex = new std::mutex;
		__inner_barrier = new std::condition_variable;
		_barrier = new std::condition_variable;
		_lock = new std::unique_lock<std::mutex>(*_mutex, std::defer_lock);
		__inner_lock = new std::unique_lock<std::mutex>(*_mutex, std::defer_lock);

		_lock->lock();
		_runner = new std::thread(__wrapper, this);
		_barrier->wait(*_lock);
		_lock->unlock();
	}
	~Context()
	{
		_lock->lock();
		_interrupted = true;
		__inner_barrier->notify_one();
		_lock->unlock();

		_runner->join();
		delete _runner;
		delete __inner_barrier;
		delete _barrier;
		delete _lock;
		delete __inner_lock;
		delete _mutex;
	}

	void __wrapper()
	{
		__inner_lock->lock();
		_barrier->notify_one();
		__inner_barrier->wait(*__inner_lock); //to start

		try
		{
			__body();
		}
		catch(const InterruptedException &e)
		{
			std::cout<<"interrupted!"<<std::endl;
		}

		__inner_finished = true;
		_barrier->notify_one();
		__inner_lock->unlock();
	}

	void __yield(Context* other)
	{
		other->_barrier->notify_one();
		__inner_barrier->wait(*__inner_lock);
		if(_interrupted)
			throw InterruptedException();
	}

	void __body() //thread body
	{
		std::cout<<"1 hello"<<std::endl;
		__yield(this);
		std::cout<<"2 hello"<<std::endl;
		__yield(this);
		std::cout<<"3 hello"<<std::endl;
		__yield(this);
		std::cout<<"4 hello"<<std::endl;
		__yield(this);
	}

	void run()
	{
		_lock->lock();

		__inner_barrier->notify_one();
		if(!__inner_finished)
			_barrier->wait(*_lock);

		_lock->unlock();
	}
};

}

#endif /* INCLUDE_R_COROUTINE2_HPP_ */
