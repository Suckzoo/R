/*
 * test_coroutine_emulate.cpp
 *
 *  Created on: 2016. 1. 31.
 *      Author: KHL
 */


#include <R/coroutine.hpp>
#define COROUTINE_NAME symmetric_coroutine
using R::COROUTINE_NAME;

#define __COROUTINE_TEST Coroutine_Emulated
#include "test_coroutine.hpp"
