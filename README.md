# MiniHypervisor Project

## Overview  
This project is a lightweight hypervisor that provides basic functionalities for managing virtual machines (VMs). It includes capabilities for executing VM instructions, handling snapshots, and supporting live migration of VM states.  

## Features  
- **Virtual Machine Execution**: Supports instruction execution with CPU register management.  
- **Snapshot Management**: Save and load VM states to enable consistent restoration.  
- **Live Migration**: Transfer VM state to remote hosts over TCP/IP for seamless execution.  
- **Execution Slices**: Configure instruction execution intervals for improved traceability.  

## Motivation  
This project demonstrates core hypervisor functionalities and serves as a foundation for understanding virtualization concepts. It addresses practical challenges like VM state persistence and migration.  

## Objectives  
1. Simulate VM execution with minimal overhead.  
2. Provide robust mechanisms for state saving and restoration.  
3. Enable seamless live migration for VM state transfer.  

## System Architecture  
1. **Input**:  
   - VM configuration files.  
   - VM binaries for instruction execution.  

2. **Components**:  
   - **Initialization**: Parses configuration files and sets up VM state.  
   - **Execution**: Executes VM instructions with support for basic MIPS operations.  
   - **Snapshot Management**: Saves and restores VM state.  
   - **Migration**: Transfers VM state between hosts over TCP/IP.  

3. **Output**:  
   - Execution logs and migration status.  
   - Snapshots for state restoration.  

## Implementation Details  
### Technologies Used  
- **Programming Language**: C  
- **Libraries**:  
  - `stdio.h` and `stdlib.h` for file operations and memory management.  
  - `string.h` for string manipulation.  
  - `stdint.h` for register state management.  
  - `arpa/inet.h` and `sys/socket.h` for socket programming.  

### Key Functionalities  
1. **VM Initialization**:  
   - Parses configuration files to load VM binaries and set execution parameters.  

2. **Instruction Execution**:  
   - Executes instructions such as `add`, `sub`, `li`, and `migrate`.  
   - Supports basic MIPS-like operations for simulation.  

3. **Snapshot Management**:  
   - Save: Writes the program counter and registers to a binary file.  
   - Load: Restores state from a snapshot file.  

4. **Live Migration**:  
   - Transfers VM binary, program counter, and registers to a remote host.  

## Usage  
### Build and Run  
1. **Compile**:  
   ```bash
   gcc -o hypervisor hypervisor.c
