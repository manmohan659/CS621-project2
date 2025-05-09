# ─────────  CS621 Project-2  (DiffServ)  ─────────
# Works with ns-3.35 built in *debug* profile
#   If you rebuild ns-3 with --build-profile=release
#   just change NS3_SUFFIX = -optimized and re-make.

# ─── ns-3 locations ──────────────────────────────
NS3_DIR      := $(HOME)/Documents/ns-allinone-3.35/ns-3.35
NS3_LIB_DIR  := $(NS3_DIR)/build/lib
NS3_INC_DIR  := $(NS3_DIR)/build/include
NS3_VERSION  := 35
NS3_SUFFIX   := -debug          # ← use -optimized for release libs

# ─── toolchain opts ──────────────────────────────
CXX          := g++
CXXFLAGS     := -std=c++17 -g -Wall -I$(NS3_INC_DIR) -I$(NS3_DIR)/build
LDFLAGS      := -L$(NS3_LIB_DIR) -Wl,-rpath,$(NS3_LIB_DIR)

# ─── ns-3 libs to link ───────────────────────────
NS3_LIBS :=  -lns3.$(NS3_VERSION)-core$(NS3_SUFFIX) \
             -lns3.$(NS3_VERSION)-network$(NS3_SUFFIX) \
             -lns3.$(NS3_VERSION)-internet$(NS3_SUFFIX) \
             -lns3.$(NS3_VERSION)-point-to-point$(NS3_SUFFIX) \
             -lns3.$(NS3_VERSION)-applications$(NS3_SUFFIX) \
             -lns3.$(NS3_VERSION)-traffic-control$(NS3_SUFFIX) \
             -lns3.$(NS3_VERSION)-flow-monitor$(NS3_SUFFIX) \
             -lns3.$(NS3_VERSION)-stats$(NS3_SUFFIX)          # ← NEW

# ─── project sources ─────────────────────────────
SRCS  := diffserv.cc traffic-class.cc filter.cc filter-element.cc \
         source-ip-address.cc dest-ip-address.cc spq.cc drr.cc \
         cisco-parser.cc diffserv-simulation.cc
OBJS  := $(SRCS:.cc=.o)
EXEC  := diffserv-simulation

# ─── rules ───────────────────────────────────────
.PHONY: all clean run-spq run-spq-cisco run-drr run-all

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(NS3_LIBS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)

# convenience runners
run-spq:         $(EXEC) ; ./$(EXEC) --mode=spq        --config=spq.config
run-spq-cisco:   $(EXEC) ; ./$(EXEC) --mode=spq        --config=cisco-spq.config --cisco=true
run-drr:         $(EXEC) ; ./$(EXEC) --mode=drr        --config=drr.config
run-all: run-spq run-spq-cisco run-drr
