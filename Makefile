# Makefile
# Variables
CXX = g++
CXXFLAGS = -g -Wall
NS3_DIR = $(HOME)/ns-3-dev

# NS-3 modules
NS3_LIBS = -lns3-core -lns3-network -lns3-internet -lns3-point-to-point -lns3-applications -lns3-traffic-control -lns3-flow-monitor

# Source files
SRCS = diffserv.cc \
       traffic-class.cc \
       filter.cc \
       filter-element.cc \
       source-ip-address.cc \
       dest-ip-address.cc \
       source-port.cc \
       dest-port.cc \
       protocol-number.cc \
       tos-field.cc \
       packet-number.cc \
       spq.cc \
       drr.cc \
       diffserv-simulation.cc

# Object files
OBJS = $(SRCS:.cc=.o)

# Executables
EXEC = diffserv-simulation

# Default target
all: $(EXEC)

# Linking the executable
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L$(NS3_DIR)/build $(NS3_LIBS)

# Compiling source files
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $< -I$(NS3_DIR)/build

# Clean target
clean:
	rm -f $(OBJS) $(EXEC)

# Run SPQ simulation
run-spq: $(EXEC)
	./$(EXEC) --mode=spq --config=spq.conf

# Run DRR simulation
run-drr: $(EXEC)
	./$(EXEC) --mode=drr --config=drr.conf

# Run both simulations
run-all: run-spq run-drr

.PHONY: all clean run-spq run-drr run-all