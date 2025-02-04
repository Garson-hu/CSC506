# Cache Coherence Simulator with MESI and MOESI Protocols

## Author info

Student name: Guangxing Hu
UnityID: ghu4

## Overview
ECE 406, ECE/CSC 506, Fall 2024, Architecture Of Parallel Computers. 

This project implements a cache coherence simulator for a multiprocessor system using the MESI and MOESI protocols. The simulator models cache behavior across multiple processors, supporting realistic evaluation of:

- Read and write operations with cache hits and misses.
- Cache invalidations during coherence actions.
- Memory transactions, including flushes and write-backs.
- Execution time estimation b◊ased on latency for various operations.

## Processor Access
### 1. MESI_Processor_Access:
- Handles processor read and write requests.
- Simulates cache coherence actions based on the MESI protocol.
- Updates execution time, cache hits/misses, and transitions cache states.

### 2. MOESI_Processor_Access:
- Similar to MESI_Processor_Access but includes the Owner state.
- Simulates cache-to-cache transfers for dirty blocks.


## Bus Snooping
### 1.MESI_Bus_Snoop:
- Handles bus transactions (BusRd, BusRdX, BusUpgr) in the MESI protocol.
- Invalidates or transitions cache states in response to coherence requests from other processors.
### 2. MOESI_Bus_Snoop:
- Extends MESI_Bus_Snoop with support for the Owner state.
- Includes logic for handling dirty blocks and reducing memory bandwidth usage.

## Usage
### Compilation
To compile the simulator, use the provided <span style="background-color: #f0f0f0; font-family: monospace; padding: 2px 4px; border-radius: 4px; box-shadow: 2px 2px 5px rgba(0, 0, 0, 0.15);">Makefile</span> 

    cd src
    make all

### Running the simulator
    make test_mesi
or 
    
    make test_moesi

