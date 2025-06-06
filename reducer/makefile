SRC = src

CXX = g++
CXX_MIS = g++

PACE_MIS_DIR = ../3rdparty/pace-2019/lib
KAHIP_IO_DIR = ../3rdparty/pace-2019/extern/KaHIP/lib

MIS_SRC_FILES = $(shell find $(PACE_MIS_DIR) $(KAHIP_IO_DIR) -name '*.cpp')

MIS_OBJ_FILES_REL = ${MIS_SRC_FILES:%.cpp=%__release__mis__.o}
MIS_OBJ_FILES_DBG = ${MIS_SRC_FILES:%.cpp=%__debug__mis__.o}

MIS_INCLUDE_DIRS = $(shell find ../3rdparty/pace-2019 -type d)
INCLUDES = $(addprefix -I,$(MIS_INCLUDE_DIRS))

# LP_SOLVER = -DLP_HIGHS
LP_SOLVER = -DLP_CBC
# LP_SOLVER = -DLP_SCIP

CXX_FLAGS = -std=c++23 -Wshadow -Wall -Wfatal-errors $(LP_SOLVER) -I /usr/local/include/highs/
CXX_FLAGS_MIS = -std=c++23 -Wshadow -Wall -Wfatal-errors -fopenmp $(INCLUDES)

CXX_FLAGS_REL = $(CXX_FLAGS) -O3 -static
CXX_FLAGS_REL_MIS = $(CXX_FLAGS_MIS) -O3 -static

CXX_FLAGS_DBG = $(CXX_FLAGS) -g -fsanitize=address,undefined -D_GLIBCXX_DEBUG
CXX_FLAGS_DBG_MIS = $(CXX_FLAGS_MIS) -g -fsanitize=address,undefined -D_GLIBCXX_DEBUG

# LNKS = -lscip -lsoplex -lmpfr -lgmp -lglpk -L/usr/local/lib/ -lhighs -L/usr/lib/x86-64-linux-gnu/ -Wl,--start-group  -lCbcSolver -lCbc -lClp -lCgl -lOsiClp -lOsi -lCoinUtils  -Wl,--end-group -llapack -lblas -lgfortran -lz -lbz2 -fopenmp
# does not work for SCIP or HIGHS:
LNKS = -L/usr/lib/x86-64-linux-gnu/ -Wl,--start-group  -lCbcSolver -lCbc -lClp -lCgl -lOsiClp -lOsi -lCoinUtils  -Wl,--end-group -llapack -lblas -lgfortran -lz -lbz2 -fopenmp

HDR_FILES = $(shell find $(SRC) -name "*.h") $(shell find $(SRC) -name "*.hpp")
SRC_FILES = $(shell find $(SRC) -name "*.cpp")

OBJ_FILES_REL = ${SRC_FILES:%.cpp=%__release__.o}
OBJ_FILES_DBG = ${SRC_FILES:%.cpp=%__debug__.o}

all: rel

.PHONY: vcdep
vcdep:
	cd ../3rdparty && rm -rf pace-2019 && git clone https://github.com/KarlsruheMIS/pace-2019.git
	rm ../3rdparty/pace-2019/extern/KaHIP/lib/tools/mpi_tools.cpp
	rm ../3rdparty/pace-2019/extern/KaHIP/lib/tools/graph_communication.cpp
	rm ../3rdparty/pace-2019/extern/KaHIP/lib/parallel_mh/exchange/exchanger.cpp
	rm ../3rdparty/pace-2019/extern/KaHIP/lib/parallel_mh/population.cpp
	rm ../3rdparty/pace-2019/extern/KaHIP/lib/parallel_mh/parallel_mh_async.cpp
	rm ../3rdparty/pace-2019/extern/KaHIP/interface/kaHIP_interface.cpp

%__release__.o: %.cpp ${HDR_FILES}
	$(CXX) -c $< -o $@ $(CXX_FLAGS_REL) $(INCLUDES)

%__debug__.o: %.cpp ${HDR_FILES}
	$(CXX) -c $< -o $@ $(CXX_FLAGS_DBG) $(INCLUDES)

%__release__mis__.o: %.cpp ${HDR_FILES}
	$(CXX_MIS) -fPIC -c $< -o $@ $(CXX_FLAGS_REL_MIS)

%__debug__mis__.o: %.cpp ${HDR_FILES}
	$(CXX_MIS) -fPIC -c $< -o $@ $(CXX_FLAGS_DBG_MIS)

librel_mis.a: ${MIS_OBJ_FILES_REL}
	ar rcs $@ $^

libdbg_mis.a: ${MIS_OBJ_FILES_DBG}
	ar rcs $@ $^

rel: ${OBJ_FILES_REL} librel_mis.a
	$(CXX) $^ -o $@ $(CXX_FLAGS_REL) $(LNKS) -L. -lrel_mis

dbg: ${OBJ_FILES_DBG} libdbg_mis.a
	$(CXX) $^ -o $@ $(CXX_FLAGS_DBG) $(LNKS) -L. -ldbg_mis

.PHONY: clean
clean:
	find $(SRC) -name '*__release__.o' -delete
	find $(SRC) -name '*__debug__.o' -delete
	find $(PACE_MIS_DIR) -name '*__release__mis__.o' -delete
	find $(KAHIP_IO_DIR) -name '*__release__mis__.o' -delete
	find $(PACE_MIS_DIR) -name '*__debug__mis__.o' -delete
	find $(KAHIP_IO_DIR) -name '*__deubg__mis__.o' -delete
