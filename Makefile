.PHONY: all test clean

CXX+= -std=gnu++11

GTEST_PATH= 3rdparty/googletest
GTEST_LIBS= $(GTEST_PATH)/libgtest.a $(GTEST_PATH)/libgtest_main.a
TEST_BIN= bin/test
DEPS= .make.dep

INCLUDES= -Iinclude -I$(GTEST_PATH)/include
LDFLAGS= -L$(GTEST_PATH)
LIBS= -lgtest_main -lgtest -lpthread

HEADERS = $(wildcard include/*.hh) $(wildcard include/*.hpp)
TEST_SRCS = $(wildcard test/*.cpp)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

all: $(TEST_BIN)

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_OBJS) $(GTEST_LIBS)
	$(CXX) $(CXXFLAGS) $(TEST_OBJS) -o $@ $(LDFLAGS) $(LIBS)

$(GTEST_PATH)/Makefile:
	cd $(GTEST_PATH) && cmake .

$(GTEST_LIBS): $(GTEST_PATH)/Makefile
	$(MAKE) -C $(GTEST_PATH)

$(DEPS): $(TEST_SRCS) $(HEADERS)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -MM $(TEST_SRCS) > $(DEPS)
	
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

-include $(DEPS)

clean: 
	rm -f $(DEPS) $(TEST_BIN) $(TEST_OBJS)