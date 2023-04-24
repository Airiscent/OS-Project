#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define PAGE_SIZE 256
#define TLB_SIZE 16
#define PAGE_ENTRIES 256
#define FRAMES 256
#define MEMORY_SIZE 65536

// Global variables for page table, TLB, and memory
int page_table[PAGE_ENTRIES];
int tlb[TLB_SIZE][2];
signed char memory[MEMORY_SIZE];

// Variables for tracking available frames, page faults, TLB hits, and total addresses
int available_frames = 0;
int page_faults = 0;
int tlb_hits = 0;
int total_addresses = 0;

// Function prototypes
int search_tlb(int page_number);
int search_page_table(int page_number);
void update_tlb(int page_number, int frame_number);

int main(int argc, char *argv[]) {
    // Check for correct arguments
    if (argc != 2) {
        printf("Usage: %s <addresses.txt>\n", argv[0]);
        exit(1);
    }

    // Initialize page table and TLB
    for (int i = 0; i < PAGE_ENTRIES; i++) {
        page_table[i] = -1;
    }

    for (int i = 0; i < TLB_SIZE; i++) {
        tlb[i][0] = -1;
        tlb[i][1] = -1;
    }

    // Open address file and backing store
    FILE *file = fopen(argv[1], "r");
    FILE *backing_store = fopen("BACKING_STORE.bin", "rb");

    // Check if files are opened successfully
    if (!file || !backing_store) {
        perror("Error opening file");
        exit(1);
    }

    // Read logical addresses and translate them to physical addresses
    int logical_address;
    while (fscanf(file, "%d", &logical_address) != EOF) {
        total_addresses++;
        int page_number = (logical_address & 0xFF00) >> 8; // Not sure if this is right
        int offset = logical_address & 0x00FF; // Same with this
        // TODO - Convert logical address to physical address by searching TLB first, then if not found, page table, and if not found there, then handle page fault
    }

    // Close files
    fclose(file);
    fclose(backing_store);

    // Calculate and print statistics
    double page_fault_rate = (double) page_faults / total_addresses;
    double tlb_hit_rate = (double) tlb_hits / total_addresses;

    printf("Page-fault rate: %.2f%%\n", page_fault_rate * 100);
    printf("TLB hit rate: %.2f%%\n", tlb_hit_rate * 100);

    return 0;
}

// Function to search for a page number in the TLB
int search_tlb(int page_number) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i][0] == page_number) {
            return tlb[i][1]; // Return frame number if found
        }
    }
    return -1; // If not found, return -1
}

// Function to search for a page number in the page table
int search_page_table(int page_number) {
    return page_table[page_number];
}

// Function to update TLB with a new entry
void update_tlb(int page_number, int frame_number) {
    int empty_index = -1;

    // Search for an empty slot in the TLB
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i][0] == -1) {
            empty_index = i;
            break;
        }
    }

    // If an empty slot is found, add the new entry
    if (empty_index != -1) {
        tlb[empty_index][0] = page_number;
        tlb[empty_index][1] = frame_number;
    } else {
        // If no empty slot, use FIFO replacement
        for (int i = 1; i < TLB_SIZE; i++) {
            tlb[i-1][0] = tlb[i][0];
            tlb[i-1][1] = tlb[i][1];
        }
        tlb[TLB_SIZE-1][0] = page_number;
        tlb[TLB_SIZE-1][1] = frame_number;
    }
}
