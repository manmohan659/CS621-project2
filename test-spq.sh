#!/bin/bash
# Script to test the SPQ implementation

# Define colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Running SPQ test for CS621-project2...${NC}"

# Check if the executable exists
if [ ! -f "diffserv-simulation" ]; then
  echo -e "${RED}ERROR: diffserv-simulation executable not found${NC}"
  echo -e "${YELLOW}Please compile the project first with: make${NC}"
  exit 1
fi

# Run the SPQ test
echo -e "${YELLOW}Running SPQ simulation...${NC}"
./diffserv-simulation --mode=spq --config=spq.conf

# Check if the SPQ throughput plot was generated
if [ -f "spq-throughput.png" ]; then
  echo -e "${GREEN}SPQ test completed successfully!${NC}"
  echo -e "${YELLOW}Throughput plot generated: spq-throughput.png${NC}"
else
  echo -e "${RED}SPQ test failed: No throughput plot was generated${NC}"
fi