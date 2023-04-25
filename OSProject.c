#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Change these variables to adjust the "physical memory" size.
#define NUM_FRAMES 256
#define MEMORY_SIZE 65536

// DO NOT TOUCH BELOW THIS LINE
#define FRAME_SIZE 256
#define TLB_SIZE 16
#define NUM_PAGES 256

#define TLB_MISS 65535
#define PAGE_TABLE_MISS 65535

// Global variables for page table, TLB, and memory
int page_table[NUM_PAGES] = { -1 };
int tlb[TLB_SIZE][2] = { -1 };
signed char memory[MEMORY_SIZE] = { -1 };

// Variables for tracking available frames, page faults, TLB hits, and total addresses
int available_frames = 0;
int page_faults = 0;
int tlb_hits = 0;
int total_addresses = 0;
uint8_t next_frame_number = 0;

// Function prototypes
void read_file_page(FILE *file, signed char buffer[FRAME_SIZE], int page);
void remove_from_page_table(uint8_t frame_number);
void remove_from_tlb(uint8_t frame_number);
void print_frame(signed char *buffer);
uint16_t search_tlb(int page_number);
void update_page_table(int page_number, uint8_t frame_number);
uint16_t search_page_table(int page_number);
void update_tlb(int page_number, uint8_t frame_number);

int main(int argc, char *argv[]) {
    // Check for correct arguments
    if (argc != 2) {
        printf("Usage: %s <addresses.txt>\n", argv[0]);
        exit(1);
    }

    // Initialize page table and TLB
    for (int i = 0; i < NUM_PAGES; i++) {
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
    signed char loaded_memory[FRAME_SIZE];
    while (fscanf(file, "%d", &logical_address) != EOF) {
        total_addresses++;

        // Calculate page_number and offset with binary algebra and 0xff masks
        int page_number = ((logical_address >> 8) & 0xFF);
        int offset = logical_address & 0xFF;
        
        uint16_t frame_number;
        if ((frame_number = search_tlb(page_number)) != TLB_MISS) ++tlb_hits;
        else if ((frame_number = search_page_table(page_number)) != PAGE_TABLE_MISS) {}
        else {
            frame_number = next_frame_number;

            // Flush "page_table" and "tlb" of all values that point to "frame_number" so that we aren't pointing to old data
            remove_from_page_table(frame_number);
            remove_from_tlb(frame_number);

            // Read data from the backing_store and load it into "memory"
            read_file_page(backing_store, loaded_memory, page_number);
            memcpy(memory + (FRAME_SIZE * frame_number), loaded_memory, FRAME_SIZE);

            // Update TLB and page table with the frame number
            update_tlb(page_number, frame_number);
            update_page_table(page_number, frame_number);


            // "FIFO" algorithm for frame numbers
            next_frame_number = (next_frame_number + 1) % NUM_FRAMES;
            ++page_faults;
        }

        uint16_t physical_address = (frame_number << 8) | offset;
        signed char value = memory[physical_address];

        printf("0x%04x -> 0x%04x: %d\n", logical_address, physical_address, value);
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

// Reads a frame in from a file
void read_file_page(FILE *file, signed char buffer[FRAME_SIZE], int page) {
    fseek(file, FRAME_SIZE * page, SEEK_SET);
    fread(buffer, 1, FRAME_SIZE, file);
}

// Printfs a frame buffer
void print_frame(signed char *buffer) {
    int i;
    for (i = 0; i < FRAME_SIZE; i++)
    {
        if (i > 0)
            if (i % 16 == 0) printf("\n");
            else printf(" ");
        printf("%02x", buffer[i] & 0xff);
    }
    printf("\n");
}

// Function to search for a page number in the TLB
uint16_t search_tlb(int page_number) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i][0] == page_number) {
            return tlb[i][1]; // Return frame number if found
        }
    }

    return TLB_MISS;
}

// Function to search for a page number in the page table
uint16_t search_page_table(int page_number) {
    return page_table[page_number];
}

// Update the page table with the reference for page_number -> frame_number
void update_page_table(int page_number, uint8_t frame_number) {
    page_table[page_number] = frame_number;
}

// Remove a referenced frame number from the page table
void remove_from_page_table(uint8_t frame_number) {
    for (int i = 0; i < NUM_PAGES; i++) {
        if (page_table[i] == frame_number) {
            page_table[i] = -1;
        }
    }
}

// Remove a referenced frame number from the tlb
void remove_from_tlb(uint8_t frame_number) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i][1] == frame_number) {
            tlb[i][0] = -1;
        }
    }
}

// Function to update TLB with a new entry
void update_tlb(int page_number, uint8_t frame_number) {
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
