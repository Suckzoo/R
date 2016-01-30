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

class DefaultScheduler
{

};

enum __data_type
{
	DATA,
	EXCEPTION,
	DONE,
};

template<typename YieldData>
class YieldType;

template<typename DataType>
struct __passed_data
{
	enum __data_type type;
	struct
	{
		std::exception exception;
		DataType value;
	}payload;
};

template< typename CallData >
class CallType
{
public:
	typedef void (*Body)(YieldType<CallData>&);
private:
	typedef struct __passed_data<CallData> MessageType;

	std::queue<MessageType*> _incoming_messages;
	bool _not_a_coro;
	YieldType<CallData>* _child;
	std::mutex* _mutex;
	std::condition_variable *_cv;
	std::function<void(void)> _safe_body;
	bool _is_started;
	std::thread* _runner;
	bool _is_done;

public:
	//not a coro
	CallType() noexcept
	{
		_not_a_coro = true;
		_child = nullptr;
		_mutex = nullptr;
		_cv = nullptr;
		_is_started = false;
		_runner = nullptr;
		_is_done = false;
	}

	//Creates a coroutine which will execute fn
	template<typename Function>
	CallType(Function && fn) : CallType()
    {
		_not_a_coro = false;

    	_mutex = new std::mutex;
    	_child = new YieldType<CallData>(this);
    	_cv = new std::condition_variable;

    	_safe_body = [&]()->void
    	{
    		_child->_lock->lock();
    		try
    		{
    			fn(*_child);
    		}
    		catch(const std::exception& err)
    		{
    			auto msg = new MessageType;
    			msg->type = EXCEPTION;
    			msg->payload.exception = err;
    			_incoming_messages.push(msg);
    			_cv->notify_one();
    		}
    		_child->_is_running = false;
    		_cv->notify_one();
    		_child->_lock->unlock();
    	};
    }

    ~CallType()
    {
    	if(!_not_a_coro)
    	{
    		assert(_mutex != nullptr);
    		assert(_child != nullptr);
    		assert(_child->_not_a_coro == false);
    		assert(_child->_parent == this);
    		if(_is_started)
    		{
    			assert(_runner != nullptr);
    			assert(_is_done == false);
    			{
    				std::lock_guard<std::mutex> guard(*_mutex);
    				_is_done = true;
    				_child->_cv->notify_one();
    			}
    			_runner->join();
    			delete _runner;
    		}
    		assert(_child->_is_running == false);
    		delete _child;
    		_child = nullptr;

    		delete _cv;
    		_cv = nullptr;
    		delete _mutex;
    		_mutex = nullptr;
    	}
    	while(!_incoming_messages.empty())
    	{
    		auto x = _incoming_messages.front();
    		_incoming_messages.pop();
    		delete x;
    	}
    }

	explicit operator bool() const noexcept
	{
		if(_not_a_coro)
			return false;
		{
			std::lock_guard<std::mutex> guard(*_mutex);
			return _child->_is_running;
		}
	}

	//invert bool()
	bool operator!() const noexcept
	{
		if(_not_a_coro)
			return true;
		{
			std::lock_guard<std::mutex> guard(*_mutex);
			return !_child->_is_running;
		}
	}

    //Precond: bool() is true
    //Execution control is transferred to coroutine-function
    //and the argument arg is passed to the coroutine-function.
    CallType& operator()(CallData arg)
    {
    	{
    		std::lock_guard<std::mutex> guard(*_mutex);
    		if (!_is_started)
    		{
    			_child->_is_running = true;
    			_runner = new std::thread(
    					_safe_body
    			);
    			_is_started = true;
    		}
    	}
		__call_from_others(arg);
		{
			std::unique_lock<std::mutex> lock(*_mutex, std::defer_lock);
			lock.lock();

			while(_incoming_messages.empty() && _child->_is_running)
				_cv->wait(lock);
			if(_incoming_messages.empty())
			{
				lock.unlock();
				return *this;
			}

			auto message = _incoming_messages.front();

			if(message->type == EXCEPTION)
			{
				std::exception exception = message->payload.exception;
				lock.unlock();
				throw exception;
			}
			if(message->type == DONE)
			{
				_incoming_messages.pop();
				delete message;
				lock.unlock();
				return *this;
			}
			assert(0);
		}
		return *this;
    }

public:
    void __call_from_others(CallData data)
    {
    	std::lock_guard<std::mutex> guard(*_mutex);
    	auto my_message = new MessageType;
    	my_message->type = DATA;
    	my_message->payload.value = data;
    	_child->_incoming_messages.push(my_message);
    	_child->_cv->notify_one();
    }

    friend class YieldType<CallData>;
};

template< typename YieldData >
class YieldType
{
private:
	typedef struct __passed_data<YieldData> MessageType;
	std::queue<MessageType*> _incoming_messages;
	bool _is_running;
	bool _not_a_coro;
	std::condition_variable* _cv;
	CallType<YieldData>* _parent;
	std::unique_lock<std::mutex>* _lock;

	YieldType(CallType<YieldData>* parent) : YieldType()
	{
		_cv = new std::condition_variable;
		_parent = parent;
		_not_a_coro = false;
		_lock = new std::unique_lock<std::mutex>(*_parent->_mutex, std::defer_lock);
	}
public:
	//not-a-coroutine.
	YieldType() noexcept
	{
		_not_a_coro = true;
		_is_running = false;
		_cv = nullptr;
		_lock = nullptr;
	}

    ~YieldType()
    {
    	if(!_not_a_coro)
    	{
    		assert(_parent != nullptr);
    		assert(_cv != nullptr);
    		assert(_lock != nullptr);
    		delete _lock;
    		_lock = nullptr;
    		delete _cv;
    		_cv = nullptr;
    	}
    	assert(_is_running == false);
    	_parent = nullptr;
    }

    //not a coro or completed: true, other false
	explicit operator bool() const noexcept
	{
		return !_not_a_coro && _is_running;
	}

    //!bool()
	bool operator!() const noexcept
	{
		return _not_a_coro || !_is_running;
	}

    //Pre: *this is not a not-a-coroutine.
    //Execution control is transferred to coroutine-function
    //(no parameter is passed to the coroutine-function).
    YieldType& operator()()
    {
    	auto my_message = new MessageType;
    	my_message->type = DONE;
    	_parent->_incoming_messages.push(my_message);
    	_parent->_cv->notify_one();

    	if(!_parent->_is_done)
    		_cv->wait(*_lock);
    	return *this;
    }

    template< typename X >
    YieldType& operator()( CallType<X> & other, X & x)
    {
    	other.__call_from_others(x);
    	if(!_parent->_is_done)
    		_cv->wait(*_lock);
    	return *this;
    }

    //Pre: *this is not a not-a-coroutine.
    //Returns data transferred from coroutine-function via
    //PushType::operator().
    YieldData get()
    {
    	while(_incoming_messages.empty())
    		_cv->wait(*_lock);
    	auto message = _incoming_messages.front();
    	if(message->type == DATA)
    	{
    		YieldData ret = message->payload.value;
    		delete message;
    		_incoming_messages.pop();
    		return ret;
    	}
    	assert(0);
    }

    friend class CallType<YieldData>;
};

}


#endif /* INCLUDE_R_COROUTINE_HPP_ */
