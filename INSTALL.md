# DiffServ Implementation for ns-3 - Installation Guide

This guide provides instructions for installing and running the DiffServ implementation for ns-3.

## Prerequisites

- ns-3 (version 3.35 or later)
- GCC/G++ compiler (version 8.0 or later)
- GNU Make
- Gnuplot (for generating throughput plots)

## Installation Steps

1. **Install ns-3 (if not already installed)**

   Follow the [official ns-3 installation guide](https://www.nsnam.org/wiki/Installation) to install ns-3 on your system.

2. **Clone or extract this project**

   Clone or extract this project into your ns-3 directory, for example, in a folder named `diffserv`:

   ```bash
   cd ~/ns-3-dev
   git clone https://github.com/yourusername/diffserv-ns3.git diffserv
   ```

3. **Update the Makefile**

   Update the Makefile to point to your ns-3 installation directory by modifying the `NS3_DIR` variable:

   ```bash
   cd diffserv
   nano Makefile
   ```

   Change the `NS3_DIR` variable to point to your ns-3 installation directory:

   ```
   NS3_DIR = /path/to/your/ns-3-dev
   ```

4. **Build the project**

   ```bash
   make
   ```

## Running Simulations

The project includes three simulation scenarios:

### 1. SPQ Simulation with Standard Configuration

This simulation demonstrates Strict Priority Queueing with a standard configuration file.

```bash
make run-spq
```

### 2. SPQ Simulation with Cisco CLI Configuration

This simulation demonstrates Strict Priority Queueing with a Cisco CLI-style configuration file.

```bash
make run-spq-cisco
```

### 3. DRR Simulation

This simulation demonstrates Deficit Round Robin scheduling with three queues in a 3:2:1 ratio.

```bash
make run-drr
```

### 4. Run All Simulations

To run all three simulations in sequence:

```bash
make run-all
```

## Output

Each simulation produces a throughput vs. time plot in PNG format:

- SPQ: `spq-throughput.png`
- SPQ with Cisco config: `spq-cisco-throughput.png`
- DRR: `drr-throughput.png`

## Troubleshooting

- If you encounter compilation errors, ensure that your ns-3 installation is correct and that the `NS3_DIR` variable in the Makefile points to the correct location.
- If the simulations run but no plots are generated, ensure that Gnuplot is installed on your system.
- If you see errors about missing ns-3 libraries, you may need to update your `LD_LIBRARY_PATH` to include the ns-3 libraries:

  ```bash
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/ns-3-dev/build
  ```

## Customizing Configurations

You can modify the configuration files to adjust the behavior of the simulations:

- `spq.conf`: Modify the number of queues and their priority levels
- `cisco-spq.conf`: Modify the Cisco CLI commands for SPQ configuration
- `drr.conf`: Modify the number of queues and their quantum values

After modifying a configuration file, run the corresponding simulation to see the effects of your changes.