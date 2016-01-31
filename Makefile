.PHONY: all test clean

CXXFLAGS+= -std=gnu++11 -O0 -g

ifdef BOOST_ENABLED
CXXFLAGS+= -DBOOST_ENABLED=$(BOOST_ENABLED)
LIBS= -lboost_coroutine -lboost_system -lboost_thread -lboost_context
else
LIBS=
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
	valgrind \
	--leak-check=full --trace-children=yes --error-exitcode=1 \
	$(TEST_BIN)

$(TEST_BIN): $(TEST_OBJS) $(GTEST_LIBS)
	$(CXX) $(CXXFLAGS) $(TEST_OBJS) -o $@ $(LDFLAGS) $(LIBS)

$(GTEST_PATH)/Makefile:
	cd $(GTEST_PATH) && cmake .

$(GTEST_LIBS): $(GTEST_PATH)/Makefile
	$(MAKE) -C $(GTEST_PATH)
	
$(TEST_OBJS): %.o: %.cpp Makefile

$(DEPS): %.d: %.cpp Makefile
	@$(CXX) $(CXXFLAGS) -MM $< > $@

-include $(DEPS)

clean: 
	rm -f $(DEPS) $(TEST_BIN) $(TEST_OBJS)