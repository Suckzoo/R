.PHONY: all test clean

CXXFLAGS+= -std=gnu++11 -O0 -g

ifndef GTEST_REPEAT
GTEST_REPEAT=100
endif

ifdef BOOST_ENABLED
CXXFLAGS+= -DBOOST_ENABLED=$(BOOST_ENABLED)
LIBS= -lboost_coroutine -lboost_system -lboost_thread -lboost_context
VALGRIND=./
GTEST_REPEAT=1
else
LIBS=
VALGRIND=valgrind --leak-check=full --trace-children=yes --error-exitcode=1 
endif

GTEST_PATH= 3rdparty/googletest
GTEST_LIBS= $(GTEST_PATH)/libgtest.a $(GTEST_PATH)/libgtest_main.a
TEST_BIN= bin/test

INCLUDES= -Iinclude -I$(GTEST_PATH)/include
LDFLAGS= -L$(GTEST_PATH)
LIBS+= -lgtest_main -lgtest -lpthread

HEADERS = $(wildcard include/*.hh) $(wildcard include/*.hpp) $(wildcard test/*.hpp)
TEST_SRCS = $(wildcard test/*.cpp)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
DEPS= $(TEST_SRCS:.cpp=.d)

CXXFLAGS+= $(INCLUDES)

all: $(DEPS) $(TEST_BIN)

test: $(DEPS) $(TEST_BIN)
	$(VALGRIND)$(TEST_BIN) --gtest_death_test_style=threadsafe --gtest_repeat=$(GTEST_REPEAT)

$(TEST_BIN): $(TEST_OBJS) $(GTEST_LIBS)
	$(CXX) $(CXXFLAGS) $(TEST_OBJS) -o $@ $(LDFLAGS) $(LIBS)

$(GTEST_PATH)/Makefile:
	cd $(GTEST_PATH) && cmake .

$(GTEST_LIBS): $(GTEST_PATH)/Makefile
	$(MAKE) -C $(GTEST_PATH)
	
$(TEST_OBJS): %.o: %.cpp Makefile $(HEADERS)

$(DEPS): %.d: %.cpp Makefile
	@$(CXX) $(CXXFLAGS) -MM $< > $@

-include $(DEPS)

clean: 
	rm -f $(DEPS) $(TEST_BIN) $(TEST_OBJS)