/*
 * test_coroutine_boost.cpp
 *
 *  Created on: 2016. 1. 31.
 *      Author: KHL
 */


#ifdef BOOST_ENABLED

#include <boost/coroutine/all.hpp>

using boost::coroutines::coroutine;
#define symmetric_coroutine coroutine
#define yield_type pull_type
#define call_type push_type

#define __COROUTINE_TEST Coroutine_Boost
#include "test_coroutine.hpp"

#endif

