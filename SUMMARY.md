# Project Summary: DiffServ Implementation for ns-3

## Overview

This project implements Differentiated Services (DiffServ) in the ns-3 network simulator, including two Quality-of-Service (QoS) mechanisms: Strict Priority Queueing (SPQ) and Deficit Round Robin (DRR).

## Components Implemented

### 1. DiffServ Base Class
- Inherits from ns-3::Queue and provides the foundation for packet classification and scheduling
- Includes Classify() method to sort incoming packets into appropriate traffic queues
- Includes Schedule() method to select which traffic queue to serve next

### 2. Traffic Class
- Defines characteristics of a queue (priority, weight, filters, etc.)
- Maintains a collection of filters for matching packets
- Provides methods for enqueueing, dequeueing, and checking packets

### 3. Filter System
- Filters are collections of FilterElement conditions combined with AND logic
- A packet matches a filter only if all filter elements are satisfied
- Traffic classes can have multiple filters (combined with OR logic)

### 4. Filter Elements
Seven filter element types were implemented for matching different packet properties:
- SourceIpAddress/DestIpAddress: Match based on source or destination IP
- SourcePort/DestPort: Match based on TCP or UDP port numbers
- ProtocolNumber: Match based on the IP protocol field (e.g., TCP=6, UDP=17)
- TosField: Match based on the Type of Service field in the IP header
- PacketNumber: Match based on packet sequence numbers (useful for testing)

### 5. QoS Mechanisms
Two QoS scheduling disciplines were implemented:

#### SPQ (Strict Priority Queueing)
- Queues have priority levels (lower value = higher priority)
- Always serves the highest priority non-empty queue
- Higher priority traffic preempts lower priority traffic

#### DRR (Deficit Round Robin)
- Implements fair queuing with quantum values
- Each queue gets bandwidth proportional to its quantum value
- Maintains deficit counters to track how many bytes each queue can transmit

### 6. Cisco CLI Parser (Extra Credit)
- Parses Cisco IOS commands for configuring SPQ
- Supports commands like mls qos, priority-queue out, mls qos trust dscp
- Maps DSCP values to priority levels for QoS

### 7. Validation Scenarios
- SPQ Validation: Shows preemption of low priority traffic by high priority traffic
- SPQ Cisco Validation: Same as above but with Cisco CLI configuration
- DRR Validation: Shows bandwidth sharing in a 3:2:1 ratio

## Key Features

1. **Modular Design**: The implementation follows a modular design, allowing for easy extension and customization.
2. **Two QoS Mechanisms**: SPQ for strict priority handling and DRR for fair bandwidth allocation.
3. **Flexible Packet Classification**: The filter system allows for complex classification based on various packet properties.
4. **Cisco CLI Support**: Extra credit component providing compatibility with Cisco CLI commands.
5. **Validation Scenarios**: Scenarios to demonstrate and verify the correct operation of the implemented QoS mechanisms.

## Installation and Usage

See the INSTALL.md file for detailed instructions on installing and running the simulations.

## Future Work

Some possible areas for future improvement include:
1. Implementing additional QoS mechanisms (e.g., WFQ, CBWFQ)
2. Enhancing the Cisco parser to support more configuration options
3. Adding support for IPv6 headers in filter elements
4. Improving the visualization of throughput plots
5. Implementing more complex network topologies for testing