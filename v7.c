// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <stdint.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <errno.h>
// 
// typedef struct {
//     char* vmFiles[10];
//     char* snapshotFiles[10];
//     int vmFileCount;
//     char config[100][100];
//     int configCount;
//     int32_t registers[32];
//     int execSlice;
//     char* vmBinary;
//     int programCounter;
//     int isMigrated;  // New flag to track migration status
// } Hypervisor;
// 
// // Function declarations (unchanged)
// void hypervisor_init(Hypervisor* hypervisor, int argc, char** argv);
// void hypervisor_run(Hypervisor* hypervisor);
// void parseConfigFile(Hypervisor* hypervisor, const char* filename);
// void executeInstructions(Hypervisor* hypervisor, int vmIndex);
// void dumpProcessorState(Hypervisor* hypervisor);
// void saveSnapshot(Hypervisor* hypervisor, const char* filename);
// void loadSnapshot(Hypervisor* hypervisor, const char* filename);
// void migrateVM(Hypervisor* hypervisor, const char* remoteHost);
// // void startReceiver(Hypervisor* hypervisor);
// void startReceiver(Hypervisor* hypervisor, const char* ipPort);  // Updated declaration
// 
// 
// 
// void saveSnapshot(Hypervisor* hypervisor, const char* filename) {
//     FILE* snapshot = fopen(filename, "wb");
//     if (snapshot == NULL) {
//         printf("Error: Unable to create snapshot file %s\n", filename);
//         return;
//     }
// 
//     // Save program counter and registers
//     fwrite(&hypervisor->programCounter, sizeof(int), 1, snapshot);
//     fwrite(hypervisor->registers, sizeof(int32_t), 32, snapshot);
//     fclose(snapshot);
//     printf("Snapshot saved to %s with PC=%d\n", filename, hypervisor->programCounter);
// }
// 
// void loadSnapshot(Hypervisor* hypervisor, const char* filename) {
//     FILE *file = fopen(filename, "rb");
//     if (!file) {
//         perror("Error opening snapshot file");
//         return;
//     }
//     
//     // Load program counter and registers
//     fread(&hypervisor->programCounter, sizeof(int), 1, file);
//     fread(hypervisor->registers, sizeof(int32_t), 32, file);
//     fclose(file);
//     printf("Snapshot loaded from %s with PC=%d\n", filename, hypervisor->programCounter);
// }
// 
// // Modified hypervisor_init function to properly parse config files
// void hypervisor_init(Hypervisor* hypervisor, int argc, char** argv) {
//     hypervisor->vmFileCount = 0;
//     hypervisor->configCount = 0;
//     hypervisor->programCounter = 0;
//     hypervisor->isMigrated = 0;
//     hypervisor->vmBinary = NULL;  // Important to initialize to NULL
//     
//     // Initialize arrays
//     for (int i = 0; i < 10; i++) {
//         hypervisor->vmFiles[i] = NULL;
//         hypervisor->snapshotFiles[i] = NULL;
//     }
// 
//     // Parse command line arguments and process config files
//     for (int i = 1; i < argc; i++) {
//         if (strcmp(argv[i], "-v") == 0 && i + 1 < argc) {
//             // Store the config file path
//             hypervisor->vmFiles[hypervisor->vmFileCount] = strdup(argv[++i]);
//             // Parse the config file immediately
//             parseConfigFile(hypervisor, hypervisor->vmFiles[hypervisor->vmFileCount]);
//             hypervisor->vmFileCount++;
//         }
//     }
//     
//     // Initialize registers
//     for (int i = 0; i < 32; i++) {
//         hypervisor->registers[i] = 0;
//     }
//     
//     hypervisor->execSlice = 0;
// }
// 
// 
// 
// 
// void executeInstructions(Hypervisor* hypervisor, int vmIndex) {
//     if (hypervisor->vmBinary == NULL) {
//         printf("Error: No VM binary specified\n");
//         return;
//     }
// 
//     FILE* file = fopen(hypervisor->vmBinary, "r");
//     if (file == NULL) {
//         printf("Error: Unable to open VM binary file %s\n", hypervisor->vmBinary);
//         return;
//     }
// 
//     char line[256];
//     int instructionCount = 0;
// 
//     // Skip instructions up to the program counter if we're resuming after migration
//     while (instructionCount < hypervisor->programCounter && fgets(line, sizeof(line), file)) {
//         instructionCount++;
//         printf("Skipping instruction %d: %s", instructionCount, line);
//     }
// 
//     printf("\nStarting execution from instruction %d\n", instructionCount + 1);
// 
//     // Continue execution from current position
//     while (fgets(line, sizeof(line), file)) {
//         instructionCount++;
//         printf("Executing instruction %d: %s", instructionCount, line);
//         
//         char lineCopy[256];
//         strcpy(lineCopy, line);
//         char* token = strtok(line, " ,\t\n");
// 
//         if (token == NULL) continue;
// 
//         // Handle MIGRATE command
//         if (strncmp(token, "MIGRATE", 7) == 0) {
//             char* remoteHost = strtok(NULL, " \t\n");
//             if (remoteHost != NULL) {
//                 printf("Migration command encountered at PC=%d\n", instructionCount);
//                 migrateVM(hypervisor, remoteHost);
//             }
//             continue;
//         }
// 
//         // Process MIPS instructions
//         if (strcmp(token, "li") == 0) {
//             int reg = atoi(strtok(NULL, " ,$\t\n"));
//             int value = atoi(strtok(NULL, " ,\t\n"));
//             hypervisor->registers[reg] = value;
//             printf("Executed li: R%d = %d\n", reg, value);
//         }
//         else if (strcmp(token, "add") == 0) {
//             int reg1 = atoi(strtok(NULL, " ,$\t\n"));
//             int reg2 = atoi(strtok(NULL, " ,$\t\n"));
//             int reg3 = atoi(strtok(NULL, " ,$\t\n"));
//             hypervisor->registers[reg1] = hypervisor->registers[reg2] + hypervisor->registers[reg3];
//             printf("Executed add: R%d = R%d + R%d = %d\n", 
//                    reg1, reg2, reg3, hypervisor->registers[reg1]);
//         }
//         else if (strcmp(token, "sub") == 0) {
//             int reg1 = atoi(strtok(NULL, " ,$\t\n"));
//             int reg2 = atoi(strtok(NULL, " ,$\t\n"));
//             int reg3 = atoi(strtok(NULL, " ,$\t\n"));
//             hypervisor->registers[reg1] = hypervisor->registers[reg2] - hypervisor->registers[reg3];
//             printf("Executed sub: R%d = R%d - R%d = %d\n", 
//                    reg1, reg2, reg3, hypervisor->registers[reg1]);
//         }
//         else if (strcmp(token, "DUMP_PROCESSOR_STATE") == 0) {
//             dumpProcessorState(hypervisor);
//         }
// 
//         else if (strcmp(token, "addi") == 0) {
//             int reg1 = atoi(strtok(NULL, " ,$\t\n"));
//             int reg2 = atoi(strtok(NULL, " ,$\t\n"));
//             int32_t value = atoi(strtok(NULL, " ,\t\n"));
//             hypervisor->registers[reg1] = hypervisor->registers[reg2] + value;
//         } else if (strcmp(token, "mul") == 0) {
//             int reg1 = atoi(strtok(NULL, " ,$\t\n"));
//             int reg2 = atoi(strtok(NULL, " ,$\t\n"));
//             int reg3 = atoi(strtok(NULL, " ,$\t\n"));
//             hypervisor->registers[reg1] = hypervisor->registers[reg2] * hypervisor->registers[reg3];
//         } else if (strcmp(token, "and") == 0) {
//             int reg1 = atoi(strtok(NULL, " ,$\t\n"));
//             int reg2 = atoi(strtok(NULL, " ,$\t\n"));
//             int reg3 = atoi(strtok(NULL, " ,$\t\n"));
//             hypervisor->registers[reg1] = hypervisor->registers[reg2] & hypervisor->registers[reg3];
//         } else if (strcmp(token, "or") == 0) {
//             int reg1 = atoi(strtok(NULL, " ,$\t\n"));
//             int reg2 = atoi(strtok(NULL, " ,$\t\n"));
//             char* third = strtok(NULL, " ,\t\n");
//             if (third[0] == '$') {
//                 int reg3 = atoi(third + 1);
//                 hypervisor->registers[reg1] = hypervisor->registers[reg2] | hypervisor->registers[reg3];
//             } else {
//                 int32_t value = atoi(third);
//                 hypervisor->registers[reg1] = hypervisor->registers[reg2] | value;
//             }
//         } else if (strcmp(token, "xor") == 0) {
//             int reg1 = atoi(strtok(NULL, " ,$\t\n"));
//             int reg2 = atoi(strtok(NULL, " ,$\t\n"));
//             int reg3 = atoi(strtok(NULL, " ,$\t\n"));
//             hypervisor->registers[reg1] = hypervisor->registers[reg2] ^ hypervisor->registers[reg3];
//         } else if (strcmp(token, "sll") == 0) {
//             int reg1 = atoi(strtok(NULL, " ,$\t\n"));
//             int reg2 = atoi(strtok(NULL, " ,$\t\n"));
//             int shift = atoi(strtok(NULL, " ,\t\n"));
//             hypervisor->registers[reg1] = hypervisor->registers[reg2] << shift;
//         } else if (strcmp(token, "srl") == 0) {
//             int reg1 = atoi(strtok(NULL, " ,$\t\n"));
//             int reg2 = atoi(strtok(NULL, " ,$\t\n"));
//             int shift = atoi(strtok(NULL, " ,\t\n"));
//             hypervisor->registers[reg1] = (uint32_t)hypervisor->registers[reg2] >> shift;
//         }
// 		  else if (strcmp(token, "DUMP_PROCESSOR_STATE") == 0) {
//             dumpProcessorState(hypervisor);
//         }
//         
//         // Update program counter
//         hypervisor->programCounter = instructionCount;
// 
//         // Handle execution slice if specified
//         if (hypervisor->execSlice > 0 && (instructionCount % hypervisor->execSlice) == 0) {
//             printf("\nReached execution slice boundary at instruction %d\n", instructionCount);
//             dumpProcessorState(hypervisor);
//         }
//     }
// 
//     printf("\nCompleted execution at PC=%d\n", hypervisor->programCounter);
//     dumpProcessorState(hypervisor);
//     fclose(file);
// }
// 
// // Modified hypervisor_run to handle both initial run and post-migration
// void hypervisor_run(Hypervisor* hypervisor) {
//     if (hypervisor->vmBinary == NULL) {
//         printf("Error: No VM binary specified\n");
//         return;
//     }
// 
//     printf("Starting execution with binary: %s\n", hypervisor->vmBinary);
//     printf("Initial program counter: %d\n", hypervisor->programCounter);
//     
//     // Execute instructions
//     executeInstructions(hypervisor, 0);
// }
// 
// // Add this implementation for dumpProcessorState
// void dumpProcessorState(Hypervisor* hypervisor) {
//     printf("\nProcessor State:\n");
//     printf("Program Counter: %d\n", hypervisor->programCounter);
//     for (int i = 0; i < 32; i++) {
//         if (hypervisor->registers[i] != 0) {
//             printf("R%d = %d\n", i, hypervisor->registers[i]);
//         }
//     }
//     printf("\n");
// }
// 
// // Add this implementation for parseConfigFile
// // Modified parseConfigFile function to properly handle VM binary path
// void parseConfigFile(Hypervisor* hypervisor, const char* filename) {
//     FILE* file = fopen(filename, "r");
//     if (file == NULL) {
//         printf("Error: Unable to open config file %s\n", filename);
//         return;
//     }
// 
//     char line[256];
//     char* directory = NULL;
//     
//     // Get the directory of the config file
//     const char* last_slash = strrchr(filename, '/');
//     if (last_slash != NULL) {
//         size_t dir_len = last_slash - filename + 1;
//         directory = (char*)malloc(dir_len + 1);
//         strncpy(directory, filename, dir_len);
//         directory[dir_len] = '\0';
//     }
// 
//     while (fgets(line, sizeof(line), file)) {
//         // Remove newline
//         line[strcspn(line, "\n")] = 0;
//         
//         char* key = strtok(line, "=");
//         char* value = strtok(NULL, "=");
//         
//         if (key && value) {
//             // Remove leading/trailing whitespace
//             while (*key == ' ') key++;
//             while (*value == ' ') value++;
//             
//             if (strcmp(key, "vm_binary") == 0) {
//                 if (directory != NULL) {
//                     // Construct full path by combining directory and binary name
//                     char* full_path = (char*)malloc(strlen(directory) + strlen(value) + 1);
//                     strcpy(full_path, directory);
//                     strcat(full_path, value);
//                     hypervisor->vmBinary = full_path;
//                 } else {
//                     hypervisor->vmBinary = strdup(value);
//                 }
//                 printf("VM Binary path set to: %s\n", hypervisor->vmBinary);
//             }
//             else if (strcmp(key, "vm_exec_slice_in_instructions") == 0) {
//                 hypervisor->execSlice = atoi(value);
//             }
//             
//             // Store in config array for future reference
//             if (hypervisor->configCount < 100) {
//                 strncpy(hypervisor->config[hypervisor->configCount], line, 99);
//                 hypervisor->config[hypervisor->configCount][99] = '\0';
//                 hypervisor->configCount++;
//             }
//         }
//     }
//     
//     free(directory);
//     fclose(file);
// }
// 
// 
// // Add new helper function for parsing IP:PORT
// void parseIpPort(const char* ipPort, char* ip, int* port) {
//     char* colonPos = strchr(ipPort, ':');
//     if (colonPos != NULL) {
//         int ipLen = colonPos - ipPort;
//         strncpy(ip, ipPort, ipLen);
//         ip[ipLen] = '\0';
//         *port = atoi(colonPos + 1);
//     } else {
//         strcpy(ip, ipPort);
//         *port = 12345;  // Default port if none specified
//     }
// }
// 
// // Modified startReceiver function
// void startReceiver(Hypervisor* hypervisor, const char* ipPort) {
//     char ip[256];
//     int port;
//     parseIpPort(ipPort, ip, &port);
// 
//     int serverSock = socket(AF_INET, SOCK_STREAM, 0);
//     if (serverSock < 0) {
//         perror("Socket creation failed");
//         return;
//     }
// 
//     struct sockaddr_in serverAddr;
//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_port = htons(port);
//     
//     if (strcmp(ip, "") == 0 || strcmp(ip, "0.0.0.0") == 0) {
//         serverAddr.sin_addr.s_addr = INADDR_ANY;
//     } else {
//         if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
//             printf("Invalid address/ Address not supported\n");
//             close(serverSock);
//             return;
//         }
//     }
// 
//     int opt = 1;
//     setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
// 
//     if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
//         perror("Bind failed");
//         close(serverSock);
//         return;
//     }
// 
//     if (listen(serverSock, 1) < 0) {
//         perror("Listen failed");
//         close(serverSock);
//         return;
//     }
// 
//     printf("Waiting for migration on %s:%d...\n", ip, port);
// 
//     struct sockaddr_in clientAddr;
//     socklen_t clientLen = sizeof(clientAddr);
// 
//     int clientSock = accept(serverSock, (struct sockaddr*)&clientAddr, &clientLen);
//     if (clientSock < 0) {
//         perror("Accept failed");
//         close(serverSock);
//         return;
//     }
// 
//     // Rest of the receiver code remains the same
//     // ... [previous implementation]
// 
//     close(clientSock);
//     close(serverSock);
// }
// 
// // Modified migrateVM function
// void migrateVM(Hypervisor* hypervisor, const char* ipPort) {
//     char ip[256];
//     int port;
//     parseIpPort(ipPort, ip, &port);
// 
//     int sock = socket(AF_INET, SOCK_STREAM, 0);
//     if (sock < 0) {
//         perror("Socket creation failed");
//         return;
//     }
// 
//     struct sockaddr_in serverAddr;
//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_port = htons(port);
//     if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
//         printf("Invalid address/ Address not supported\n");
//         close(sock);
//         return;
//     }
// 
//     if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
//         printf("Connection Failed to %s:%d\n", ip, port);
//         close(sock);
//         return;
//     }
// 
//     // Rest of the migration code remains the same
//     // ... [previous implementation]
// 
//     close(sock);
//     printf("Migration completed to %s:%d at PC=%d. Terminating local execution.\n", 
//            ip, port, hypervisor->programCounter);
//     exit(0);
// }
// 
// // // Modified main function
// // int main(int argc, char** argv) {
// //     Hypervisor hypervisor;
// //     hypervisor_init(&hypervisor, argc, argv);
// // 
// //     // Parse command line arguments
// //     if (argc >= 3 && strcmp(argv[1], "-p") == 0) {
// //         // Receiver mode with specified IP:PORT
// //         startReceiver(&hypervisor, argv[2]);
// //     } else {
// //         // Normal execution mode
// //         hypervisor_run(&hypervisor);
// //     }
// // 
// //     return 0;
// // }
// 
// void print_usage(const char* program_name) {
//     printf("Usage:\n");
//     printf("  Receiver mode: %s -p <IP:PORT>\n", program_name);
//     printf("  Sender mode:   %s -v <config_file>\n", program_name);
//     printf("\nExamples:\n");
//     printf("  %s -p 0.0.0.0:8080     # Listen on all interfaces, port 8080\n", program_name);
//     printf("  %s -p 192.168.1.100:9090 # Listen on specific IP and port\n", program_name);
//     printf("  %s -v vm_config.txt     # Run as sender with config file\n", program_name);
// }
// 
// int main(int argc, char** argv) {
//     if (argc < 2) {
//         print_usage(argv[0]);
//         return 1;
//     }
// 
//     Hypervisor hypervisor;
//     memset(&hypervisor, 0, sizeof(Hypervisor)); // Initialize all fields to 0
// 
//     // Check the mode flag
//     if (strcmp(argv[1], "-p") == 0) {
//         // Receiver mode
//         if (argc != 3) {
//             printf("Error: Receiver mode requires IP:PORT argument\n");
//             print_usage(argv[0]);
//             return 1;
//         }
// 
//         // Validate IP:PORT format
//         char ip[256];
//         int port;
//         char* ipPort = argv[2];
//         char* colonPos = strchr(ipPort, ':');
//         
//         if (colonPos == NULL) {
//             printf("Error: Invalid IP:PORT format. Must be in format IP:PORT\n");
//             print_usage(argv[0]);
//             return 1;
//         }
// 
//         // Basic validation before starting receiver
//         int ipLen = colonPos - ipPort;
//         if (ipLen >= sizeof(ip)) {
//             printf("Error: IP address too long\n");
//             return 1;
//         }
//         strncpy(ip, ipPort, ipLen);
//         ip[ipLen] = '\0';
//         port = atoi(colonPos + 1);
// 
//         if (port <= 0 || port > 65535) {
//             printf("Error: Invalid port number. Must be between 1 and 65535\n");
//             return 1;
//         }
// 
//         printf("Starting receiver on %s:%d...\n", ip, port);
//         startReceiver(&hypervisor, argv[2]);
//     }
//     else if (strcmp(argv[1], "-v") == 0) {
//         // Sender mode
//         if (argc != 3) {
//             printf("Error: Sender mode requires config file argument\n");
//             print_usage(argv[0]);
//             return 1;
//         }
// 
//         // Initialize hypervisor with config file
//         hypervisor_init(&hypervisor, argc, argv);
//         
//         // Check if initialization was successful
//         if (hypervisor.vmBinary == NULL) {
//             printf("Error: Failed to initialize hypervisor. Check config file.\n");
//             return 1;
//         }
// 
//         // Run the hypervisor
//         printf("Starting sender with config file: %s\n", argv[2]);
//         hypervisor_run(&hypervisor);
//     }
//     else {
//         printf("Error: Invalid mode. Use -p for receiver or -v for sender\n");
//         print_usage(argv[0]);
//         return 1;
//     }
// 
//     // Cleanup
//     if (hypervisor.vmBinary) {
//         free(hypervisor.vmBinary);
//     }
//     for (int i = 0; i < hypervisor.vmFileCount; i++) {
//         if (hypervisor.vmFiles[i]) {
//             free(hypervisor.vmFiles[i]);
//         }
//         if (hypervisor.snapshotFiles[i]) {
//             free(hypervisor.snapshotFiles[i]);
//         }
//     }
// 
//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

typedef struct {
    char* vmFiles[10];
    char* snapshotFiles[10];
    int vmFileCount;
    char config[100][100];
    int configCount;
    int32_t registers[32];
    int execSlice;
    char* vmBinary;
    int programCounter;
    int isMigrated;  // New flag to track migration status
} Hypervisor;

// Function declarations (unchanged)
void hypervisor_init(Hypervisor* hypervisor, int argc, char** argv);
void hypervisor_run(Hypervisor* hypervisor);
void parseConfigFile(Hypervisor* hypervisor, const char* filename);
void executeInstructions(Hypervisor* hypervisor, int vmIndex);
void dumpProcessorState(Hypervisor* hypervisor);
void saveSnapshot(Hypervisor* hypervisor, const char* filename);
void loadSnapshot(Hypervisor* hypervisor, const char* filename);
void migrateVM(Hypervisor* hypervisor, const char* remoteHost);
// void startReceiver(Hypervisor* hypervisor);
void startReceiver(Hypervisor* hypervisor, const char* ipPort);  // Updated declaration



void saveSnapshot(Hypervisor* hypervisor, const char* filename) {
    FILE* snapshot = fopen(filename, "wb");
    if (snapshot == NULL) {
        printf("Error: Unable to create snapshot file %s\n", filename);
        return;
    }

    // Save program counter and registers
    fwrite(&hypervisor->programCounter, sizeof(int), 1, snapshot);
    fwrite(hypervisor->registers, sizeof(int32_t), 32, snapshot);
    fclose(snapshot);
    printf("Snapshot saved to %s with PC=%d\n", filename, hypervisor->programCounter);
}

void loadSnapshot(Hypervisor* hypervisor, const char* filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening snapshot file");
        return;
    }
    
    // Load program counter and registers
    fread(&hypervisor->programCounter, sizeof(int), 1, file);
    fread(hypervisor->registers, sizeof(int32_t), 32, file);
    fclose(file);
    printf("Snapshot loaded from %s with PC=%d\n", filename, hypervisor->programCounter);
}

// Modified hypervisor_init function to properly parse config files
void hypervisor_init(Hypervisor* hypervisor, int argc, char** argv) {
    hypervisor->vmFileCount = 0;
    hypervisor->configCount = 0;
    hypervisor->programCounter = 0;
    hypervisor->isMigrated = 0;
    hypervisor->vmBinary = NULL;  // Important to initialize to NULL
    
    // Initialize arrays
    for (int i = 0; i < 10; i++) {
        hypervisor->vmFiles[i] = NULL;
        hypervisor->snapshotFiles[i] = NULL;
    }

    // Parse command line arguments and process config files
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 && i + 1 < argc) {
            // Store the config file path
            hypervisor->vmFiles[hypervisor->vmFileCount] = strdup(argv[++i]);
            // Parse the config file immediately
            parseConfigFile(hypervisor, hypervisor->vmFiles[hypervisor->vmFileCount]);
            hypervisor->vmFileCount++;
        }
    }
    
    // Initialize registers
    for (int i = 0; i < 32; i++) {
        hypervisor->registers[i] = 0;
    }
    
    hypervisor->execSlice = 0;
}




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

    // Skip instructions up to the program counter if we're resuming after migration
    while (instructionCount < hypervisor->programCounter && fgets(line, sizeof(line), file)) {
        instructionCount++;
        printf("Skipping instruction %d: %s", instructionCount, line);
    }

    printf("\nStarting execution from instruction %d\n", instructionCount + 1);

    // Continue execution from current position
    while (fgets(line, sizeof(line), file)) {
        instructionCount++;
        printf("Executing instruction %d: %s", instructionCount, line);
        
        char lineCopy[256];
        strcpy(lineCopy, line);
        char* token = strtok(line, " ,\t\n");

        if (token == NULL) continue;

        // Handle MIGRATE command
        if (strncmp(token, "MIGRATE", 7) == 0) {
            char* remoteHost = strtok(NULL, " \t\n");
            if (remoteHost != NULL) {
                printf("Migration command encountered at PC=%d\n", instructionCount);
                migrateVM(hypervisor, remoteHost);
            }
            continue;
        }

        // Process MIPS instructions
        if (strcmp(token, "li") == 0) {
            int reg = atoi(strtok(NULL, " ,$\t\n"));
            int value = atoi(strtok(NULL, " ,\t\n"));
            hypervisor->registers[reg] = value;
            printf("Executed li: R%d = %d\n", reg, value);
        }
        else if (strcmp(token, "add") == 0) {
            int reg1 = atoi(strtok(NULL, " ,$\t\n"));
            int reg2 = atoi(strtok(NULL, " ,$\t\n"));
            int reg3 = atoi(strtok(NULL, " ,$\t\n"));
            hypervisor->registers[reg1] = hypervisor->registers[reg2] + hypervisor->registers[reg3];
            printf("Executed add: R%d = R%d + R%d = %d\n", 
                   reg1, reg2, reg3, hypervisor->registers[reg1]);
        }
        else if (strcmp(token, "sub") == 0) {
            int reg1 = atoi(strtok(NULL, " ,$\t\n"));
            int reg2 = atoi(strtok(NULL, " ,$\t\n"));
            int reg3 = atoi(strtok(NULL, " ,$\t\n"));
            hypervisor->registers[reg1] = hypervisor->registers[reg2] - hypervisor->registers[reg3];
            printf("Executed sub: R%d = R%d - R%d = %d\n", 
                   reg1, reg2, reg3, hypervisor->registers[reg1]);
        }
        else if (strcmp(token, "DUMP_PROCESSOR_STATE") == 0) {
            dumpProcessorState(hypervisor);
        }

        else if (strcmp(token, "addi") == 0) {
            int reg1 = atoi(strtok(NULL, " ,$\t\n"));
            int reg2 = atoi(strtok(NULL, " ,$\t\n"));
            int32_t value = atoi(strtok(NULL, " ,\t\n"));
            hypervisor->registers[reg1] = hypervisor->registers[reg2] + value;
        } else if (strcmp(token, "mul") == 0) {
            int reg1 = atoi(strtok(NULL, " ,$\t\n"));
            int reg2 = atoi(strtok(NULL, " ,$\t\n"));
            int reg3 = atoi(strtok(NULL, " ,$\t\n"));
            hypervisor->registers[reg1] = hypervisor->registers[reg2] * hypervisor->registers[reg3];
        } else if (strcmp(token, "and") == 0) {
            int reg1 = atoi(strtok(NULL, " ,$\t\n"));
            int reg2 = atoi(strtok(NULL, " ,$\t\n"));
            int reg3 = atoi(strtok(NULL, " ,$\t\n"));
            hypervisor->registers[reg1] = hypervisor->registers[reg2] & hypervisor->registers[reg3];
        } else if (strcmp(token, "or") == 0) {
            int reg1 = atoi(strtok(NULL, " ,$\t\n"));
            int reg2 = atoi(strtok(NULL, " ,$\t\n"));
            char* third = strtok(NULL, " ,\t\n");
            if (third[0] == '$') {
                int reg3 = atoi(third + 1);
                hypervisor->registers[reg1] = hypervisor->registers[reg2] | hypervisor->registers[reg3];
            } else {
                int32_t value = atoi(third);
                hypervisor->registers[reg1] = hypervisor->registers[reg2] | value;
            }
        } else if (strcmp(token, "xor") == 0) {
            int reg1 = atoi(strtok(NULL, " ,$\t\n"));
            int reg2 = atoi(strtok(NULL, " ,$\t\n"));
            int reg3 = atoi(strtok(NULL, " ,$\t\n"));
            hypervisor->registers[reg1] = hypervisor->registers[reg2] ^ hypervisor->registers[reg3];
        } else if (strcmp(token, "sll") == 0) {
            int reg1 = atoi(strtok(NULL, " ,$\t\n"));
            int reg2 = atoi(strtok(NULL, " ,$\t\n"));
            int shift = atoi(strtok(NULL, " ,\t\n"));
            hypervisor->registers[reg1] = hypervisor->registers[reg2] << shift;
        } else if (strcmp(token, "srl") == 0) {
            int reg1 = atoi(strtok(NULL, " ,$\t\n"));
            int reg2 = atoi(strtok(NULL, " ,$\t\n"));
            int shift = atoi(strtok(NULL, " ,\t\n"));
            hypervisor->registers[reg1] = (uint32_t)hypervisor->registers[reg2] >> shift;
        }
		  else if (strcmp(token, "DUMP_PROCESSOR_STATE") == 0) {
            dumpProcessorState(hypervisor);
        }
        
        // Update program counter
        hypervisor->programCounter = instructionCount;

        // Handle execution slice if specified
        if (hypervisor->execSlice > 0 && (instructionCount % hypervisor->execSlice) == 0) {
            printf("\nReached execution slice boundary at instruction %d\n", instructionCount);
            dumpProcessorState(hypervisor);
        }
    }

    printf("\nCompleted execution at PC=%d\n", hypervisor->programCounter);
    dumpProcessorState(hypervisor);
    fclose(file);
}

// Modified hypervisor_run to handle both initial run and post-migration
void hypervisor_run(Hypervisor* hypervisor) {
    if (hypervisor->vmBinary == NULL) {
        printf("Error: No VM binary specified\n");
        return;
    }

    printf("Starting execution with binary: %s\n", hypervisor->vmBinary);
    printf("Initial program counter: %d\n", hypervisor->programCounter);
    
    // Execute instructions
    executeInstructions(hypervisor, 0);
}

// Add this implementation for dumpProcessorState
void dumpProcessorState(Hypervisor* hypervisor) {
    printf("\nProcessor State:\n");
    printf("Program Counter: %d\n", hypervisor->programCounter);
    for (int i = 0; i < 32; i++) {
        if (hypervisor->registers[i] != 0) {
            printf("R%d = %d\n", i, hypervisor->registers[i]);
        }
    }
    printf("\n");
}

// Add this implementation for parseConfigFile
// Modified parseConfigFile function to properly handle VM binary path
void parseConfigFile(Hypervisor* hypervisor, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Unable to open config file %s\n", filename);
        return;
    }

    char line[256];
    char* directory = NULL;
    
    // Get the directory of the config file
    const char* last_slash = strrchr(filename, '/');
    if (last_slash != NULL) {
        size_t dir_len = last_slash - filename + 1;
        directory = (char*)malloc(dir_len + 1);
        strncpy(directory, filename, dir_len);
        directory[dir_len] = '\0';
    }

    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        char* key = strtok(line, "=");
        char* value = strtok(NULL, "=");
        
        if (key && value) {
            // Remove leading/trailing whitespace
            while (*key == ' ') key++;
            while (*value == ' ') value++;
            
            if (strcmp(key, "vm_binary") == 0) {
                if (directory != NULL) {
                    // Construct full path by combining directory and binary name
                    char* full_path = (char*)malloc(strlen(directory) + strlen(value) + 1);
                    strcpy(full_path, directory);
                    strcat(full_path, value);
                    hypervisor->vmBinary = full_path;
                } else {
                    hypervisor->vmBinary = strdup(value);
                }
                printf("VM Binary path set to: %s\n", hypervisor->vmBinary);
            }
            else if (strcmp(key, "vm_exec_slice_in_instructions") == 0) {
                hypervisor->execSlice = atoi(value);
            }
            
            // Store in config array for future reference
            if (hypervisor->configCount < 100) {
                strncpy(hypervisor->config[hypervisor->configCount], line, 99);
                hypervisor->config[hypervisor->configCount][99] = '\0';
                hypervisor->configCount++;
            }
        }
    }
    
    free(directory);
    fclose(file);
}


// Add new helper function for parsing IP:PORT
void parseIpPort(const char* ipPort, char* ip, int* port) {
    char* colonPos = strchr(ipPort, ':');
    if (colonPos != NULL) {
        int ipLen = colonPos - ipPort;
        strncpy(ip, ipPort, ipLen);
        ip[ipLen] = '\0';
        *port = atoi(colonPos + 1);
    } else {
        strcpy(ip, ipPort);
        *port = 12345;  // Default port if none specified
    }
}

// Modified startReceiver function
// void startReceiver(Hypervisor* hypervisor, const char* ipPort) {
//     char ip[256];
//     int port;
//     parseIpPort(ipPort, ip, &port);
// 
//     int serverSock = socket(AF_INET, SOCK_STREAM, 0);
//     if (serverSock < 0) {
//         perror("Socket creation failed");
//         return;
//     }
// 
//     struct sockaddr_in serverAddr;
//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_port = htons(port);
//     
//     if (strcmp(ip, "") == 0 || strcmp(ip, "0.0.0.0") == 0) {
//         serverAddr.sin_addr.s_addr = INADDR_ANY;
//     } else {
//         if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
//             printf("Invalid address/ Address not supported\n");
//             close(serverSock);
//             return;
//         }
//     }
// 
//     int opt = 1;
//     setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
// 
//     if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
//         perror("Bind failed");
//         close(serverSock);
//         return;
//     }
// 
//     if (listen(serverSock, 1) < 0) {
//         perror("Listen failed");
//         close(serverSock);
//         return;
//     }
// 
//     printf("Waiting for migration on %s:%d...\n", ip, port);
// 
//     struct sockaddr_in clientAddr;
//     socklen_t clientLen = sizeof(clientAddr);
// 
//     int clientSock = accept(serverSock, (struct sockaddr*)&clientAddr, &clientLen);
//     if (clientSock < 0) {
//         perror("Accept failed");
//         close(serverSock);
//         return;
//     }
// 
//     // Rest of the receiver code remains the same
//     // ... [previous implementation]
// 
//     close(clientSock);
//     close(serverSock);
// }
// Function to receive VM state
void startReceiver(Hypervisor* hypervisor, const char* ipPort) {
    char ip[256];
    int port;
    parseIpPort(ipPort, ip, &port);

    // Create socket
    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock < 0) {
        perror("Socket creation failed");
        return;
    }

    // Enable address reuse
    int opt = 1;
    if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(serverSock);
        return;
    }

    // Configure server address
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    // Handle IP binding
    if (strcmp(ip, "0.0.0.0") == 0 || strcmp(ip, "") == 0) {
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        printf("Listening on all interfaces on port %d\n", port);
    } else {
        if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
            printf("Invalid address/ Address not supported\n");
            close(serverSock);
            return;
        }
        printf("Listening on %s:%d\n", ip, port);
    }

    // Bind socket
    if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        printf("Error code: %d\n", errno);
        close(serverSock);
        return;
    }

    // Listen for connections
    if (listen(serverSock, 1) < 0) {
        perror("Listen failed");
        close(serverSock);
        return;
    }

    printf("Server is listening for incoming connections...\n");

    while (1) {  // Add continuous listening loop
        // Accept client connection
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSock = accept(serverSock, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSock < 0) {
            perror("Accept failed");
            continue;  // Continue listening if accept fails
        }

        // Log client connection
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
        printf("Received connection from %s:%d\n", clientIP, ntohs(clientAddr.sin_port));

        // Receive VM binary path length
        uint32_t networkPathLen;
        ssize_t receivedBytes = recv(clientSock, &networkPathLen, sizeof(networkPathLen), MSG_WAITALL);
        if (receivedBytes != sizeof(networkPathLen)) {
            printf("Failed to receive binary path length. Received %zd bytes\n", receivedBytes);
            close(clientSock);
            continue;
        }
        uint32_t pathLen = ntohl(networkPathLen);
        printf("Expecting binary path of length: %u\n", pathLen);

        // Receive VM binary path
        char* vmBinaryPath = (char*)malloc(pathLen + 1);
        receivedBytes = recv(clientSock, vmBinaryPath, pathLen, MSG_WAITALL);
        if (receivedBytes != pathLen) {
            printf("Failed to receive binary path. Received %zd bytes\n", receivedBytes);
            free(vmBinaryPath);
            close(clientSock);
            continue;
        }
        vmBinaryPath[pathLen] = '\0';
        hypervisor->vmBinary = vmBinaryPath;

        // Receive program counter
        int32_t networkPC;
        if (recv(clientSock, &networkPC, sizeof(networkPC), MSG_WAITALL) != sizeof(networkPC)) {
            perror("Failed to receive program counter");
            close(clientSock);
            continue;
        }
        hypervisor->programCounter = ntohl(networkPC);

        // Receive register states
        int32_t networkRegisters[32];
        if (recv(clientSock, networkRegisters, sizeof(networkRegisters), MSG_WAITALL) != sizeof(networkRegisters)) {
            perror("Failed to receive registers");
            close(clientSock);
            continue;
        }

        // Convert registers from network to host byte order
        for (int i = 0; i < 32; i++) {
            hypervisor->registers[i] = ntohl(networkRegisters[i]);
        }

        printf("Migration received successfully:\n");
        printf("VM Binary: %s\n", hypervisor->vmBinary);
        printf("Program Counter: %d\n", hypervisor->programCounter);
        printf("Register state received\n");

        // Set migration flag
        hypervisor->isMigrated = 1;

        // Close client socket
        close(clientSock);

        // Resume execution from migrated state
        printf("Resuming execution from PC=%d\n", hypervisor->programCounter);
        hypervisor_run(hypervisor);
        break;  // Exit loop after successful migration
    }

    // Close server socket
    close(serverSock);
}

// Function to send VM state
void migrateVM(Hypervisor* hypervisor, const char* ipPort) {
    char ip[256];
    int port;
    parseIpPort(ipPort, ip, &port);

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return;
    }

    // Configure server address
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported\n");
        close(sock);
        return;
    }

    // Connect to receiver
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Connection Failed to %s:%d\n", ip, port);
        close(sock);
        return;
    }

    printf("Connected to receiver at %s:%d\n", ip, port);

    // Save current state before migration
    printf("Preparing to migrate state with PC=%d\n", hypervisor->programCounter);

    // Send VM binary path length
    uint32_t pathLen = strlen(hypervisor->vmBinary);
    uint32_t networkPathLen = htonl(pathLen);
    if (send(sock, &networkPathLen, sizeof(networkPathLen), 0) != sizeof(networkPathLen)) {
        perror("Failed to send binary path length");
        close(sock);
        return;
    }

    // Send VM binary path
    if (send(sock, hypervisor->vmBinary, pathLen, 0) != pathLen) {
        perror("Failed to send binary path");
        close(sock);
        return;
    }

    // Send program counter
    int32_t networkPC = htonl(hypervisor->programCounter);
    if (send(sock, &networkPC, sizeof(networkPC), 0) != sizeof(networkPC)) {
        perror("Failed to send program counter");
        close(sock);
        return;
    }

    // Convert registers to network byte order and send
    int32_t networkRegisters[32];
    for (int i = 0; i < 32; i++) {
        networkRegisters[i] = htonl(hypervisor->registers[i]);
    }
    
    if (send(sock, networkRegisters, sizeof(networkRegisters), 0) != sizeof(networkRegisters)) {
        perror("Failed to send registers");
        close(sock);
        return;
    }

    printf("Migration state sent successfully:\n");
    printf("VM Binary: %s\n", hypervisor->vmBinary);
    printf("Program Counter: %d\n", hypervisor->programCounter);
    printf("Register state sent\n");

    close(sock);
    printf("Migration completed. Terminating local execution.\n");
    exit(0);
}


// // Modified main function
// int main(int argc, char** argv) {
//     Hypervisor hypervisor;
//     hypervisor_init(&hypervisor, argc, argv);
// 
//     // Parse command line arguments
//     if (argc >= 3 && strcmp(argv[1], "-p") == 0) {
//         // Receiver mode with specified IP:PORT
//         startReceiver(&hypervisor, argv[2]);
//     } else {
//         // Normal execution mode
//         hypervisor_run(&hypervisor);
//     }
// 
//     return 0;
// }

void print_usage(const char* program_name) {
    printf("Usage:\n");
    printf("  Receiver mode: %s -p <IP:PORT>\n", program_name);
    printf("  Sender mode:   %s -v <config_file>\n", program_name);
    printf("\nExamples:\n");
    printf("  %s -p 0.0.0.0:8080     # Listen on all interfaces, port 8080\n", program_name);
    printf("  %s -p 192.168.1.100:9090 # Listen on specific IP and port\n", program_name);
    printf("  %s -v vm_config.txt     # Run as sender with config file\n", program_name);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    Hypervisor hypervisor;
    memset(&hypervisor, 0, sizeof(Hypervisor)); // Initialize all fields to 0

    // Check the mode flag
    if (strcmp(argv[1], "-p") == 0) {
        // Receiver mode
        if (argc != 3) {
            printf("Error: Receiver mode requires IP:PORT argument\n");
            print_usage(argv[0]);
            return 1;
        }

        // Validate IP:PORT format
        char ip[256];
        int port;
        char* ipPort = argv[2];
        char* colonPos = strchr(ipPort, ':');
        
        if (colonPos == NULL) {
            printf("Error: Invalid IP:PORT format. Must be in format IP:PORT\n");
            print_usage(argv[0]);
            return 1;
        }

        // Basic validation before starting receiver
        int ipLen = colonPos - ipPort;
        if (ipLen >= sizeof(ip)) {
            printf("Error: IP address too long\n");
            return 1;
        }
        strncpy(ip, ipPort, ipLen);
        ip[ipLen] = '\0';
        port = atoi(colonPos + 1);

        if (port <= 0 || port > 65535) {
            printf("Error: Invalid port number. Must be between 1 and 65535\n");
            return 1;
        }

        printf("Starting receiver on %s:%d...\n", ip, port);
        startReceiver(&hypervisor, argv[2]);
    }
    else if (strcmp(argv[1], "-v") == 0) {
        // Sender mode
        if (argc != 3) {
            printf("Error: Sender mode requires config file argument\n");
            print_usage(argv[0]);
            return 1;
        }

        // Initialize hypervisor with config file
        hypervisor_init(&hypervisor, argc, argv);
        
        // Check if initialization was successful
        if (hypervisor.vmBinary == NULL) {
            printf("Error: Failed to initialize hypervisor. Check config file.\n");
            return 1;
        }

        // Run the hypervisor
        printf("Starting sender with config file: %s\n", argv[2]);
        hypervisor_run(&hypervisor);
    }
    else {
        printf("Error: Invalid mode. Use -p for receiver or -v for sender\n");
        print_usage(argv[0]);
        return 1;
    }

    // Cleanup
    if (hypervisor.vmBinary) {
        free(hypervisor.vmBinary);
    }
    for (int i = 0; i < hypervisor.vmFileCount; i++) {
        if (hypervisor.vmFiles[i]) {
            free(hypervisor.vmFiles[i]);
        }
        if (hypervisor.snapshotFiles[i]) {
            free(hypervisor.snapshotFiles[i]);
        }
    }

    return 0;
}
