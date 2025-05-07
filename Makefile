# Makefile for CS621 Project2 DiffServ in ns-3 on macOS

NS3_DIR     := $(HOME)/ns-3-dev
NS3_VERSION := 3.44

SDK_PATH    := $(shell xcrun --show-sdk-path)
CXX         := clang++
STD_LIB     := -stdlib=libc++

CXXFLAGS    := -std=c++17 -g -Wall \
               -isysroot $(SDK_PATH) $(STD_LIB) \
               -I$(SDK_PATH)/usr/include \
               -isystem $(NS3_DIR)/build/include \
               -isystem $(NS3_DIR)/build/include/ns3

LDFLAGS     := -isysroot $(SDK_PATH) $(STD_LIB) \
               -L$(NS3_DIR)/build/lib

NS3_LIBS    := -lns3.$(NS3_VERSION)-core \
               -lns3.$(NS3_VERSION)-network \
               -lns3.$(NS3_VERSION)-internet \
               -lns3.$(NS3_VERSION)-point-to-point \
               -lns3.$(NS3_VERSION)-applications \
               -lns3.$(NS3_VERSION)-traffic-control \
               -lns3.$(NS3_VERSION)-flow-monitor

SRCS        := diffserv.cc traffic-class.cc filter.cc filter-element.cc \
               source-ip-address.cc dest-ip-address.cc source-port.cc \
               dest-port.cc protocol-number.cc tos-field.cc \
               packet-number.cc spq.cc drr.cc cisco-parser.cc \
               diffserv-simulation.cc

OBJS        := $(SRCS:.cc=.o)
EXEC        := diffserv-simulation

.PHONY: all clean run-spq run-spq-cisco run-drr run-all

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(NS3_LIBS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)

run-spq: $(EXEC)
	./$(EXEC) --mode=spq --config=spq.conf

run-spq-cisco: $(EXEC)
	./$(EXEC) --mode=spq --config=cisco-spq.conf --cisco=true

run-drr: $(EXEC)
	./$(EXEC) --mode=drr --config=drr.conf

run-all: run-spq run-spq-cisco run-drr