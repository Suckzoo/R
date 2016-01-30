/*
 * test_coroutine_boost.cpp
 *
 *  Created on: 2016. 1. 31.
 *      Author: KHL
 */


#ifdef BOOST_ENABLED

#include <boost/coroutine/all.hpp>

#define COROUTINE_NAME BOOST_ENABLED
using boost::coroutines::COROUTINE_NAME;

#define __COROUTINE_TEST Coroutine_Boost
#include "test_coroutine.hpp"

#endif

