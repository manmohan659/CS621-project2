# DiffServ Implementation for ns-3
A modular implementation of Differentiated Services (DiffServ) for the ns-3 network simulator with Strict Priority Queueing (SPQ) and Deficit Round Robin (DRR) scheduling disciplines.

# Developers
- Deniz Gould
- Nannan Venkatesan
- Manmohan Sharma

# Project Overview
This project implements a base class for Differentiated Services (DiffServ) in the ns-3 network simulator, along with two Quality-of-Service (QoS) scheduling mechanisms:

Strict Priority Queueing (SPQ): Implements strict priority scheduling where higher priority queues are always served before lower priority queues.
Deficit Round Robin (DRR): Implements fair queuing based on assigned weights, ensuring proportional bandwidth allocation.

The implementation includes a flexible architecture with:

DiffServ: Base class that provides the framework for packet classification and scheduling
TrafficClass: Defines queue characteristics (priority, weight, max packets)
Filter: Enables packet classification based on multiple criteria
FilterElement: Base class for specific matching conditions (IP address, port, etc.)


# Project Structure

- diffserv.h/cc: DiffServ base class implementation
- traffic-class.h/cc: TrafficClass implementation
- filter.h/cc: Filter implementation
- filter-element.h/cc: FilterElement base class
- source-ip-address.h/cc: Source IP address filter element
- dest-ip-address.h/cc: Destination IP address filter element
- source-port.h/cc: Source port filter element
- dest-port.h/cc: Destination port filter element
- spq.h/cc: SPQ implementation
- drr.h/cc: DRR implementation
- diffserv-simulation.cc: Simulation scenarios
- cisco-parser.h/cc: Parser for Cisco-style configuration
- Makefile: Build script
- spq.config: SPQ configuration file
- drr.config: DRR configuration file
- cisco-spq.config: Cisco-style SPQ configuration

# Installation Instructions
## Prerequisites

- ns-3 (version 3.35 or later)
- C++ compiler with C++17 support
- GNU Make


## Setup

Update the Makefile to point to your ns-3 installation directory by modifying the NS3_DIR variable if necessary:
Eg.
`NS3_DIR := $(HOME)/Documents/ns-allinone-3.35/ns-3.35`

## Build the project:

```bash
cd ~/Documents/project/
make
```


## Usage
### Running SPQ Simulation
- To run the SPQ validation scenario:
`make run-spq`
This simulates a 3-node topology with two bulk data transfer applications. Application A has higher priority than Application B and starts after Application B has already begun transmitting.

### Running DRR Simulation
To run the DRR validation scenario:
`make run-drr`
This simulates a 3-node topology with three bulk data transfer applications, each classified into a different queue with quantum values in a 3:2:1 ratio.

### Running with Cisco-style Configuration (Extra Credit)
To run SPQ with Cisco-style configuration:
`make run-spq-cisco`

#### Output
Each simulation produces a throughput vs. time plot in PNG format:
- SPQ: spq-throughput.png
- SPQ (Cisco): spq-cisco-throughput.png
- DRR: drr-throughput.png

## Cleaning Up
To clean the build files:
make clean

# Known Limitations

The current implementation uses TCP for traffic generation, which introduces congestion control dynamics that can affect the observed QoS behavior. For clearer demonstration of QoS mechanisms, UDP might provide more predictable results.
Packet capture is implemented using FlowMonitor rather than direct pcap capture at specific NetDevices.
