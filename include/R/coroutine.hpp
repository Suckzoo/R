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

template<typename DataType>
class Context
{
	friend class CallType<DataType> ;
	friend class YieldType<DataType> ;
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
	bool _is_started;

private:
	Context() noexcept
	{
		_has_throw = false;
		_interrupted = false;
		__inner_finished = false;

		_mutex = new std::mutex;
		__inner_barrier = new std::condition_variable;
		_barrier = new std::condition_variable;

		__inner_lock = new std::unique_lock<std::mutex>(*_mutex,
				std::defer_lock);
		_is_started = false;
	}
	void __start(std::function<void(void)> && body_function)
	{
		auto lock = std::unique_lock<std::mutex>(*_mutex, std::defer_lock);
		lock.lock();
		if (_is_started)
			assert(0);
		_is_started = true;
		_runner = new std::thread([this, body_function]()
		{
			__inner_lock->lock();
			_barrier->notify_one();
			__inner_barrier->wait(*__inner_lock); //to start

				try
				{
					body_function();
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
				catch(...)
				{
					last_exception = std::exception();
					_has_throw = true;
				}

				__inner_finished = true;
				_barrier->notify_one();
				__inner_lock->unlock();
			});
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

	void __yield(Context* other)
	{
		other->_barrier->notify_one();
		__inner_barrier->wait(*__inner_lock);
		if (_interrupted)
			throw InterruptedException();
	}

	void run()
	{
		auto lock = std::unique_lock<std::mutex>(*_mutex, std::defer_lock);
		lock.lock();
		if (!_is_started)
			assert(0);
		__inner_barrier->notify_one();
		if (!__inner_finished)
			_barrier->wait(lock);
		if (_has_throw)
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
		return !this->_interrupted && !this->__inner_finished
				&& this->_is_started;
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

template<typename CallData>
class CallType
{
public:
	typedef std::function<void(YieldType<CallData>&)> Body;
private:
	bool _not_a_coro;
	YieldType<CallData>* _child;
	__data_holder <CallData> _data;
	Context<CallData>* _ctx;

public:
	//not a coro
	CallType() noexcept
	{
		_not_a_coro = true;
		_child = nullptr;
		_ctx = nullptr;
	}

	//Creates a coroutine which will execute fn
	template<typename Function>
	CallType(Function && fn) :
			CallType()
	{
		_ctx = new Context<CallData>();
		_not_a_coro = false;
		_child = new YieldType<CallData>(this);

		_ctx->__start([this,fn]()
		{
			fn(*_child);
		});
	}

	virtual ~CallType()
	{
		if (_ctx != nullptr)
			delete _ctx;
		if (_child != nullptr)
			delete _child;
	}

	explicit operator bool() noexcept
	{
		if (_not_a_coro)
			return false;
		{
			return this->_ctx->is_running();
		}
	}

	//invert bool()
	bool operator!() noexcept
	{
		if (_not_a_coro)
			return true;
		{
			return !this->_ctx->is_running();
		}
	}

	//Precond: bool() is true
	//Execution control is transferred to coroutine-function
	//and the argument arg is passed to the coroutine-function.
	template<typename CalledData>
	CallType& operator()(CalledData arg)
	{
		this->_data.value = arg;
		this->_ctx->run();
		return *this;
	}

	CallType& operator()(void)
	{
		this->_ctx->run();
		return *this;
	}

	void swap(CallType & other) noexcept
	{
		std::swap(*this, other);

		if (this->_child != nullptr)
			this->_child->_parent = this;
		if (other._child != nullptr)
			other._child->_parent = &other;
	}

private:

	friend class YieldType<CallData> ;
};

template<typename YieldData>
class YieldType
{
private:
	bool _not_a_coro;
	CallType<YieldData>* _parent;

	YieldType(CallType<YieldData>* parent) :
			YieldType()
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
		_parent->_ctx->__yield(_parent->_ctx);
		return *this;
	}

#ifdef __NOT_VERIFIED
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
#endif

	//Pre: *this is not a not-a-coroutine.
	//Returns data transferred from coroutine-function via
	//PushType::operator().
	YieldData get()
	{
		return _parent->_data.value;
	}

	void swap(YieldType & other) noexcept
	{
		std::swap(*this, other);

		if (this->_parent != nullptr)
			this->_parent->_child = this;
		if (other._parent != nullptr)
			other._parent->_child = &other;
	}

	friend class CallType<YieldData> ;
};

template<typename T>
struct symmetric_coroutine
{
	typedef CallType<T> call_type;
	typedef YieldType<T> yield_type;
};

}

#endif /* INCLUDE_R_COROUTINE_HPP_ */
