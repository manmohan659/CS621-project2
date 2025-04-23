# DiffServ Implementation for ns-3

This project implements a base class for Differentiated Services (DiffServ) in the ns-3 network simulator, along with two Quality-of-Service (QoS) mechanisms: Strict Priority Queueing (SPQ) and Deficit Round Robin (DRR).

## Project Structure

The project consists of the following components:

1. **DiffServ Base Class**: A base class for differentiated services that inherits from ns-3::Queue.
2. **TrafficClass**: Defines the characteristics of a queue.
3. **Filter**: A collection of FilterElement conditions.
4. **FilterElement**: A base class for packet matching conditions with seven subclasses.
5. **SPQ**: Implementation of Strict Priority Queueing.
6. **DRR**: Implementation of Deficit Round Robin.
7. **Simulation Scripts**: Scripts to validate and verify the implementations.

## Installation

### Prerequisites

- ns-3 (version 3.35 or later)
- GCC/G++ compiler (version 8.0 or later)
- GNU Make

### Setup

1. Clone or extract this project into your ns-3 directory, for example, in a folder named `diffserv`.
2. Update the Makefile to point to your ns-3 installation directory by modifying the `NS3_DIR` variable.
3. Build the project:

```bash
make
```

## Running Simulations

### SPQ Simulation

Run the SPQ validation scenario:

```bash
make run-spq
```

This will simulate a 3-node topology with two bulk data transfer applications. Application A has higher priority than Application B, and starts after Application B has already begun transmitting.

### DRR Simulation

Run the DRR validation scenario:

```bash
make run-drr
```

This will simulate a 3-node topology with three bulk data transfer applications, each classified into a different queue with quantum values in a 3:2:1 ratio.

### Run Both Simulations

To run both simulations in sequence:

```bash
make run-all
```

## Configuration Files

### SPQ Configuration

The SPQ configuration file specifies the number of queues and their associated priority levels. Lower priority values indicate higher priority.

Example `spq.conf`:
```
2
0  // High priority queue
1  // Low priority queue
```

### DRR Configuration

The DRR configuration file specifies the number of queues and the quantum value assigned to each queue.

Example `drr.conf`:
```
3
300  // Queue with weight 3
200  // Queue with weight 2
100  // Queue with weight 1
```

## Output

Each simulation produces a throughput vs. time plot in PNG format:
- SPQ: `spq-throughput.png`
- DRR: `drr-throughput.png`

## Project Files

- `diffserv.h/cc`: DiffServ base class implementation
- `traffic-class.h/cc`: TrafficClass implementation
- `filter.h/cc`: Filter implementation
- `filter-element.h/cc`: FilterElement base class
- `source-ip-address.h/cc`: Source IP address filter element
- `dest-ip-address.h/cc`: Destination IP address filter element
- `source-port.h/cc`: Source port filter element
- `dest-port.h/cc`: Destination port filter element
- `protocol-number.h/cc`: Protocol number filter element
- `tos-field.h/cc`: ToS field filter element
- `packet-number.h/cc`: Packet number filter element
- `spq.h/cc`: SPQ implementation
- `drr.h/cc`: DRR implementation
- `diffserv-simulation.cc`: Simulation scenarios
- `Makefile`: Build script
- `spq.conf`: SPQ configuration file
- `drr.conf`: DRR configuration file

## API Usage

### Creating a DiffServ Queue

```cpp
// Create SPQ queue
Ptr<SPQ> spq = CreateObject<SPQ> ();
spq->SetConfigFile ("spq.conf");

// Create DRR queue
Ptr<DRR> drr = CreateObject<DRR> ();
drr->SetConfigFile ("drr.conf");
```

### Adding Traffic Classes Manually

```cpp
// Create a traffic class
Ptr<TrafficClass> tClass = CreateObject<TrafficClass> ();
tClass->SetPriorityLevel (1);  // For SPQ
tClass->SetWeight (100);      // For DRR

// Add to DiffServ queue
diffServ->AddTrafficClass (tClass);
```

### Adding Filters to Traffic Classes

```cpp
// Create a filter
Ptr<Filter> filter = CreateObject<Filter> ();

// Create filter elements
Ptr<SourceIpAddress> srcIp = CreateObject<SourceIpAddress> (Ipv4Address ("10.1.1.1"));
Ptr<DestPort> destPort = CreateObject<DestPort> (80);

// Add elements to filter
filter->AddFilterElement (srcIp);
filter->AddFilterElement (destPort);

// Add filter to traffic class
tClass->AddFilter (filter);
```

### Applying DiffServ to a Device

```cpp
// Get the device
Ptr<NetDevice> dev = node->GetDevice (0);

// Set the queue disc
dev->SetAttribute ("TxQueue", PointerValue (diffServ));
```

## License

This project is licensed under the terms of the GNU General Public License v3.0.