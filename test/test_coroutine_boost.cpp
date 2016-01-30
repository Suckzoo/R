/*
 * test_coroutine_boost.cpp
 *
 *  Created on: 2016. 1. 31.
 *      Author: KHL
 */


#ifdef BOOST_ENABLED

#include <boost/coroutine/all.hpp>
using boost::coroutines::symmetric_coroutine;

#define __COROUTINE_TEST Coroutine_Boost
#include "test_coroutine.hpp"

#endif

