/*
 * coroutine.hpp
 *
 *  Created on: 2016. 1. 30.
 *      Author: KHL
 */

#ifndef INCLUDE_R_COROUTINE_HPP_
#define INCLUDE_R_COROUTINE_HPP_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <exception>
#include <cassert>
#include <functional>

namespace R
{

template<typename CallData>
class CallType;

template<typename YieldData>
class YieldType;

class InterruptedException
{

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
	std::unique_lock<std::mutex> *__inner_lock;

	std::exception last_exception;
	bool _has_throw;

protected:
	Context() noexcept
	{
		_has_throw = false;
		_interrupted = false;
		__inner_finished = false;

		_mutex = new std::mutex;
		__inner_barrier = new std::condition_variable;
		_barrier = new std::condition_variable;
		auto lock = std::unique_lock<std::mutex>(*_mutex, std::defer_lock);
		__inner_lock = new std::unique_lock<std::mutex>(*_mutex, std::defer_lock);

		lock.lock();
		_runner = new std::thread(__wrapper, this);
		_barrier->wait(lock);
		lock.unlock();
	}
	virtual ~Context() noexcept
	{
		auto lock = std::unique_lock<std::mutex>(*_mutex, std::defer_lock);
		lock.lock();
		_interrupted = true;
		__inner_barrier->notify_one();
		lock.unlock();

		_runner->join();
		delete _runner;
		delete __inner_barrier;
		delete _barrier;
		delete __inner_lock;
		delete _mutex;
	}

	void __wrapper() noexcept
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
			//std::cout<<"interrupted!"<<std::endl;
		}
		catch(const std::exception& err)
		{
			last_exception = err;
			_has_throw = true;
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

	virtual void __body() = 0;

	void run()
	{
		auto lock = std::unique_lock<std::mutex>(*_mutex, std::defer_lock);
		lock.lock();
		__inner_barrier->notify_one();
		if(!__inner_finished)
			_barrier->wait(lock);
		if(_has_throw)
		{
			lock.unlock();
			throw last_exception;
			return;
		}
		lock.unlock();
	}

	bool is_running() noexcept
	{
		std::lock_guard<std::mutex> guard(*_mutex);

		return is_running_unsafe();
	}

	bool is_running_unsafe() noexcept
	{
		return !this->_interrupted && !this->__inner_finished;
	}

	void safe_run(std::function<void(void)> fn) noexcept
	{
		std::lock_guard<std::mutex> guard(*_mutex);
		fn();
	}
};

template<typename Data>
struct __data_holder
{
	Data value;
};

template<>
struct __data_holder<void>
{

};

template< typename CallData >
class CallType : public Context
{
public:
	typedef std::function<void(YieldType<CallData>&)> Body;
private:
	bool _not_a_coro;
	YieldType<CallData>* _child;
	Body _body;

	__data_holder<CallData> _data;

protected:
	virtual void __body()
	{
		_body(*_child);
	}

public:
	//not a coro
	CallType() noexcept
	{
		_not_a_coro = true;
		_child = nullptr;
	}

	//Creates a coroutine which will execute fn
	template<typename Function>
	CallType(Function && fn) : CallType()
    {
		_not_a_coro = false;
    	_child = new YieldType<CallData>(this);
    	_body = fn;
    }

    virtual ~CallType()
    {

    }

	explicit operator bool() noexcept
	{
		if(_not_a_coro)
			return false;
		{
			return this->is_running();
		}
	}

	//invert bool()
	bool operator!() noexcept
	{
		if(_not_a_coro)
			return true;
		{
			return !this->is_running();
		}
	}

    //Precond: bool() is true
    //Execution control is transferred to coroutine-function
    //and the argument arg is passed to the coroutine-function.
	template<typename CalledData>
    CallType& operator()(CalledData arg)
    {
		this->_data.value = arg;
    	this->run();
		return *this;
    }

    CallType& operator()(void)
    {
    	this->run();
    	return *this;
    }

private:

    friend class YieldType<CallData>;
};

template< typename YieldData >
class YieldType
{
private:
	bool _not_a_coro;
	CallType<YieldData>* _parent;

	YieldType(CallType<YieldData>* parent) : YieldType()
	{
		_parent = parent;
		_not_a_coro = false;
	}
public:
	//not-a-coroutine.
	YieldType() noexcept
	{
		_not_a_coro = true;
		_parent = nullptr;
	}

    ~YieldType()
    {
    	_parent = nullptr;
    }

	explicit operator bool() noexcept
	{
		if (_not_a_coro)
			return false;
		{
			return _parent->is_running_unsafe();
		}
	}

	//invert bool()
	bool operator!() noexcept
	{
		if (_not_a_coro)
			return true;
		{
			return !_parent->is_running_unsafe();
		}
	}

    //Pre: *this is not a not-a-coroutine.
    //Execution control is transferred to coroutine-function
    //(no parameter is passed to the coroutine-function).
    YieldType& operator()()
    {
    	_parent->__yield(_parent);
    	return *this;
    }

    template< typename X >
    YieldType& operator()( CallType<X> & other, X & x)
    {
    	other._data.value = x;
    	_parent->__yield(&other);
    	return *this;
    }

    YieldType& operator()( CallType<void> & other)
    {
    	_parent->__yield(&other);
    	return *this;
    }

    //Pre: *this is not a not-a-coroutine.
    //Returns data transferred from coroutine-function via
    //PushType::operator().
    YieldData get()
    {
    	return _parent->_data.value;
    }

    friend class CallType<YieldData>;
};

template<typename T>
struct symmetric_coroutine
{
	typedef CallType<T> call_type;
	typedef YieldType<T> yield_type;
};

}


#endif /* INCLUDE_R_COROUTINE_HPP_ */
