#!/bin/bash
# Script to test the DRR implementation

# Define colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Running DRR test for CS621-project2...${NC}"

# Check if the executable exists
if [ ! -f "diffserv-simulation" ]; then
  echo -e "${RED}ERROR: diffserv-simulation executable not found${NC}"
  echo -e "${YELLOW}Please compile the project first with: make${NC}"
  exit 1
fi

# Run the DRR test
echo -e "${YELLOW}Running DRR simulation...${NC}"
./diffserv-simulation --mode=drr --config=drr.config

# Check if the DRR throughput plot was generated
if [ -f "drr-throughput.png" ]; then
  echo -e "${GREEN}DRR test completed successfully!${NC}"
  echo -e "${YELLOW}Throughput plot generated: drr-throughput.png${NC}"
else
  echo -e "${RED}DRR test failed: No throughput plot was generated${NC}"
fi