# Mini Hypervisor Project 🚀  

## Overview  
This project is a **lightweight hypervisor** 🖥️ designed to manage **virtual machines (VMs)**. It includes capabilities for executing VM instructions, handling snapshots, and supporting **live migration** of VM states.  

## Demo
[Watch the demo video](/Demonstration.mp4)


## Features 🌟  
- **Virtual Machine Execution**: Supports instruction execution with CPU register management.  
- **Snapshot Management**: Save and load VM states to enable consistent restoration.  
- **Live Migration**: Transfer VM state to remote hosts 🌍 for seamless execution.  
- **Execution Slices**: Configure instruction execution intervals ⏱️ for improved traceability.  

## Motivation 🎯  
This project demonstrates core **hypervisor functionalities** and serves as a foundation for understanding **virtualization concepts**. It addresses practical challenges like VM state persistence and migration.  

## Objectives 🎯  
1. Simulate VM execution with minimal overhead 🏃‍♂️.  
2. Provide robust mechanisms for state saving and restoration 🔄.  
3. Enable seamless live migration for VM state transfer 🌐.  

## System Architecture 🏗️  
1. **Input**:  
   - VM configuration files 📄.  
   - VM binaries for instruction execution 🖥️.  

2. **Components**:  
   - **Initialization**: Parses configuration files and sets up VM state.  
   - **Execution**: Executes VM instructions with support for basic MIPS operations 💻.  
   - **Snapshot Management**: Saves and restores VM state ⏳.  
   - **Migration**: Transfers VM state between hosts over TCP/IP 🌍.  

3. **Output**:  
   - Execution logs and migration status 📝.  
   - Snapshots for state restoration 💾.  

## Implementation Details 🛠️  
### Technologies Used 🧑‍💻  
- **Programming Language**: C 💻  
- **Libraries**:  
  - `stdio.h` and `stdlib.h` for file operations and memory management 💾.  
  - `string.h` for string manipulation 🔤.  
  - `stdint.h` for register state management 🧮.  
  - `arpa/inet.h` and `sys/socket.h` for socket programming 🌐.  

### Key Functionalities ⚙️  
1. **VM Initialization**:  
   - Parses configuration files to load VM binaries and set execution parameters 🛠️.  

2. **Instruction Execution**:  
   - Executes instructions such as `add`, `sub`, `li`, and `migrate` 🔄.  
   - Supports basic MIPS-like operations for simulation 💻.  

3. **Snapshot Management**:  
   - Save: Writes the program counter and registers to a binary file 💾.  
   - Load: Restores state from a snapshot file 🏞️.  

4. **Live Migration**:  
   - Transfers VM binary, program counter, and registers to a remote host 🖥️➡️🌍.  

## Usage 📋  
### Build and Run 🛠️  
1. **Compile**:  
   ```bash
   gcc -o hypervisor hypervisor.c
   ```  

2. **Run as Sender (Normal Execution)**:  
   ```bash
   ./hypervisor -v <config_file>
   ```  

3. **Run as Receiver (Migration Listener)**:  
   ```bash
   ./hypervisor -p <IP:PORT>
   ```  

### Configuration File Format 📄  
- **vm_binary**: Path to the VM binary file 📝.  
- **vm_exec_slice_in_instructions**: Number of instructions per execution slice ⏱️.  

Example:  
```plaintext
vm_binary=sample_vm.bin  
vm_exec_slice_in_instructions=10  
```  

## Example Output 🖥️  
- **Normal Execution**:  
   ```plaintext
   Starting execution with binary: sample_vm.bin  
   Executing instruction 1: li $1, 10  
   Executing instruction 2: add $2, $1, $3  
   Completed execution at PC=10  
   ```  

- **Snapshot Management**:  
   ```plaintext
   Snapshot saved to vm_state.bin with PC=10  
   Snapshot loaded from vm_state.bin with PC=10  
   ```  

- **Live Migration**:  
   ```plaintext
   Migration completed to 192.168.1.10:8080 at PC=10.  
   ```  

## Contribution 💡  
Contributions are welcome! Feel free to open issues or submit pull requests 🤝.  

## License 📜  
This project is licensed under the MIT License. See the `LICENSE` file for more details.  

## Acknowledgments 🙏  
- Inspired by foundational concepts of **virtualization** and **hypervisors** 🧑‍💻.  
- Special thanks to contributors and reviewers for improving the project 👏.  

---

## Key Functions and Code Walkthrough 🧑‍💻  

### 1. **`hypervisor_init`**  
Initializes the hypervisor, processes configuration files, and sets up VM state 🛠️.  

```c
void hypervisor_init(Hypervisor* hypervisor, int argc, char** argv) {
    hypervisor->vmFileCount = 0;
    hypervisor->configCount = 0;
    hypervisor->programCounter = 0;
    hypervisor->isMigrated = 0;
    hypervisor->vmBinary = NULL;

    for (int i = 0; i < 10; i++) {
        hypervisor->vmFiles[i] = NULL;
        hypervisor->snapshotFiles[i] = NULL;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 && i + 1 < argc) {
            hypervisor->vmFiles[hypervisor->vmFileCount] = strdup(argv[++i]);
            parseConfigFile(hypervisor, hypervisor->vmFiles[hypervisor->vmFileCount]);
            hypervisor->vmFileCount++;
        }
    }

    for (int i = 0; i < 32; i++) {
        hypervisor->registers[i] = 0;
    }

    hypervisor->execSlice = 0;
}
```

### 2. **`executeInstructions`**  
Executes VM instructions like `add`, `sub`, and `migrate`, and processes migrations 🚀.  

```c
void executeInstructions(Hypervisor* hypervisor, int vmIndex) {
    if (hypervisor->vmBinary == NULL) {
        printf("Error: No VM binary specified\n");
        return;
    }

    FILE* file = fopen(hypervisor->vmBinary, "r");
    if (file == NULL) {
        printf("Error: Unable to open VM binary file %s\n", hypervisor->vmBinary);
        return;
    }

    char line[256];
    int instructionCount = 0;

    while (fgets(line, sizeof(line), file)) {
        instructionCount++;
        char* token = strtok(line, " ,\t\n");
        if (token == NULL) continue;

        if (strncmp(token, "MIGRATE", 7) == 0) {
            char* remoteHost = strtok(NULL, " \t\n");
            if (remoteHost != NULL) {
                migrateVM(hypervisor, remoteHost);
            }
            continue;
        }

        if (strcmp(token, "li") == 0) {
            int reg = atoi(strtok(NULL, " ,$\t\n"));
            int value = atoi(strtok(NULL, " ,\t\n"));
            hypervisor->registers[reg] = value;
        } else if (strcmp(token, "add") == 0) {
            int reg1 = atoi(strtok(NULL, " ,$\t\n"));
            int reg2 = atoi(strtok(NULL, " ,$\t\n"));
            int reg3 = atoi(strtok(NULL, " ,$\t\n"));
            hypervisor->registers[reg1] = hypervisor->registers[reg2] + hypervisor->registers[reg3];
        }

        hypervisor->programCounter = instructionCount;
    }

    fclose(file);
}
```

### 3. **`saveSnapshot` and `loadSnapshot`**  
Handles saving and loading of VM state to/from snapshot files 💾.  

```c
void saveSnapshot(Hypervisor* hypervisor, const char* filename) {
    FILE* snapshot = fopen(filename, "wb");
    if (snapshot == NULL) {
        printf("Error: Unable to create snapshot file %s\n", filename);
        return;
    }

    fwrite(&hypervisor->programCounter, sizeof(int), 1, snapshot);
    fwrite(hypervisor->registers, sizeof(int32_t), 32, snapshot);
    fclose(snapshot);
}

void loadSnapshot(Hypervisor* hypervisor, const char* filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening snapshot file");
        return;
    }

    fread(&hypervisor->programCounter, sizeof(int), 1, file);
    fread(hypervisor->registers, sizeof(int32_t), 32, file);
    fclose(file);
}
```

### 4. **`migrateVM`**  
Handles live migration of VM state to a remote host 🌍.  

```c
void migrateVM(Hypervisor* hypervisor, const char* ipPort) {
    // Logic to transfer VM state over TCP/IP to the destination
    printf("Migrating VM to %s\n", ipPort);
    // Implementation of the migration process here...
}
```
