SRC = src

CXX = g++
CXX_FLAGS = -std=c++23 -Wshadow -Wall -Wfatal-errors
CXX_FLAGS_REL = $(CXX_FLAGS) -O2
CXX_FLAGS_DBG = $(CXX_FLAGS) -g -fsanitize=address,undefined -D_GLIBCXX_DEBUG

INCL = -I ../3rdparty

LNKS = -L ../3rdparty -ljson

HDR_FILES = $(shell find $(SRC) -name "*.h") $(shell find $(SRC) -name "*.hpp")
SRC_FILES = $(shell find $(SRC) -name "*.cpp")

OBJ_FILES_REL = ${SRC_FILES:%.cpp=%__release__.o}
OBJ_FILES_DBG = ${SRC_FILES:%.cpp=%__debug__.o}

%__release__.o: %.cpp ${HDR_FILES}
	$(CXX) -c $< -o $@ $(CXX_FLAGS_REL) $(INCL)

%__debug__.o: %.cpp ${HDR_FILES}
	$(CXX) -c $< -o $@ $(CXX_FLAGS_DBG) $(INCL)

analyze: ${OBJ_FILES_REL}
	$(CXX) $^ -o $@ $(CXX_FLAGS_REL) $(LNKS)

analyze_dbg: ${OBJ_FILES_DBG}
	$(CXX) $^ -o $@ $(CXX_FLAGS_DBG) $(LNKS)

.PHONY: clean
clean:
	find $(SRC) -name '*__release__.o' -delete
	find $(SRC) -name '*__debug__.o' -delete
