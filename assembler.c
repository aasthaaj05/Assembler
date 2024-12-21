#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX 20
#define HASH_SIZE 20

// Structures for symbol, literal, and hash tables
typedef struct symbol {
    char sym[10]; // Symbol name
    int addr;     // Symbol address
} Symbol;

typedef struct litab {
    char lit[10]; // Literal value
    int addr;     // Literal address
} Literal;

typedef struct {
    char key[10]; // Key for the hash table
    int index;    // Index or value associated with the key
} HashEntry;

// Hash tables for various components
HashEntry optab_hash[HASH_SIZE], regtab_hash[HASH_SIZE], adtab_hash[HASH_SIZE], condtab_hash[HASH_SIZE];
HashEntry symtab_hash[HASH_SIZE];  // Symbol table hash
HashEntry pooltab_hash[HASH_SIZE]; // Pool table hash

// Symbol and Literal tables
Symbol S[MAX];   // Symbol table
Literal L[MAX];  // Literal table

// Global variables for counters and location tracking
int lc = 0, sc = 1, litcnt = 0;  // Location counter, symbol counter, literal count
int poolcnt = 1;                 // Pool counter starts from 1

// Hash function to calculate the index for a given key
int hash_function(char *key) {
    int hash = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        hash = (hash + key[i]) % HASH_SIZE; // Compute hash based on character sum
    }
    return hash;
}

// Insert a key and index into a hash table using linear probing
void insert_hash(HashEntry hash_table[], char *key, int index) {
    int hash = hash_function(key);
    while (hash_table[hash].key[0] != '\0') { // Resolve collisions using linear probing
        hash = (hash + 1) % HASH_SIZE;
    }
    strcpy(hash_table[hash].key, key); // Insert key
    hash_table[hash].index = index;   // Insert index/value
}

// Search for a key in a hash table and return the associated index, or -1 if not found
int search_hash(HashEntry hash_table[], char *key) {
    int hash = hash_function(key);
    int start = hash;
    while (hash_table[hash].key[0] != '\0') { // Iterate until an empty slot or match is found
        if (strcmp(hash_table[hash].key, key) == 0) {
            return hash_table[hash].index;
        }
        hash = (hash + 1) % HASH_SIZE; // Linear probing
        if (hash == start) break;     // Avoid infinite loop
    }
    return -1;
}

// Insert a pool entry into the pool table hash
void insert_pooltab(int pool_num, int start_index) {
    char pool_key[10];
    sprintf(pool_key, "P%d", pool_num);  // Convert pool number to string key
    insert_hash(pooltab_hash, pool_key, start_index);
}

// Search for a pool in the pool table hash
int search_pooltab(int pool_num) {
    char pool_key[10];
    sprintf(pool_key, "P%d", pool_num);  // Convert pool number to string key
    return search_hash(pooltab_hash, pool_key);
}

// Initialize all hash tables for the assembler
void initialize_hash_tables() {
    // Opcode Table
    char *optab[] = {"STOP", "ADD", "SUB", "MULT", "MOVER", "MOVEM", "COMP", "BC", "DIV", "READ", "PRINT"};
    for (int i = 0; i < 11; i++) {
        insert_hash(optab_hash, optab[i], i);
    }

    // Register Table
    char *regtab[] = {"AREG", "BREG", "CREG", "DREG"};
    for (int i = 0; i < 4; i++) {
        insert_hash(regtab_hash, regtab[i], i);
    }

    // Assembler Directive Table
    char *adtab[] = {"START", "END", "ORIGIN", "EQU", "LTORG"};
    for (int i = 0; i < 5; i++) {
        insert_hash(adtab_hash, adtab[i], i + 1);  // Index from 1
    }

    // Condition Table
    char *condtab[] = {"LT", "LE", "EQ", "GT", "GE", "ANY"};
    for (int i = 0; i < 6; i++) {
        insert_hash(condtab_hash, condtab[i], i);
    }

    // Initialize the first pool in the pool table
    insert_pooltab(poolcnt, 0);  // Pool 1 starts at index 0
}

// Function to search for various tables
int search_optab(char *s) { return search_hash(optab_hash, s); }
int search_regtab(char *s) { return search_hash(regtab_hash, s); }
int search_adtab(char *s) { return search_hash(adtab_hash, s); }
int search_condtab(char *s) { return search_hash(condtab_hash, s); }

// Insert a new symbol into the symbol table
void insert_symtab(char *s) {
    int index = sc;
    strcpy(S[sc].sym, s);
    S[sc].addr = -1;  // Address is uninitialized at insertion
    insert_hash(symtab_hash, s, index);
    sc++;
}

// Search for a symbol in the symbol table
int search_symtab(char *s) {
    return search_hash(symtab_hash, s);
}

// Insert a literal into the literal table
void insert_litab(char *lit) {
    strcpy(L[litcnt].lit, lit);
    L[litcnt].addr = -1;  // Address is uninitialized
    litcnt++;
}

// Search for a literal in the literal table
int search_litab(char *lit) {
    for (int i = 0; i < litcnt; i++) {
        if (strcmp(L[i].lit, lit) == 0) {
            return i;
        }
    }
    return -1;
}

// Assign addresses to all literals in the current pool
void assign_literals() {
    int start_index = search_pooltab(poolcnt);
    for (int i = start_index; i < litcnt; i++) {
        L[i].addr = lc++;  // Assign location counter to literal
    }
    insert_pooltab(++poolcnt, litcnt);  // Start a new pool
}

// Pass 1: Generates intermediate code and updates tables
void pass1(char *source) {
    int i, k, p, j;
    char buffer[200], tok1[10], tok2[10], tok3[10], tok4[10];
    
    FILE *fs = fopen(source, "r");
    FILE *ft = fopen("id.txt", "w");

    if (!fs || !ft) {
        printf("Error opening files.\n");
        return;
    }

    while (fgets(buffer, 80, fs)) {
        int n = sscanf(buffer, "%s%s%s%s", tok1, tok2, tok3, tok4);

        if (n == 1) {
            i = search_adtab(tok1);
            if (i == 5) {  // END directive
    assign_literals();  // Assign addresses to unprocessed literals
    fprintf(ft, "(AD, %02d)\n", i);
    break;  // Stop processing further instructions
}
            }
        

        if (n == 2) {
            i = search_adtab(tok1);
            if (i == 1) {  // START
                lc = atoi(tok2) - 1;
                fprintf(ft, "(AD, %02d) (C, %s)\n", i, tok2);
                lc++;
                continue;
            }
        }

        if (n == 3) {
            i = search_optab(tok1);
            if (i >= 0) {
                tok2[strlen(tok2) - 1] = '\0';  // Remove ',' at the end of tok2
                k = search_regtab(tok2);

                if (tok3[0] == '=') {
                    j = search_litab(tok3);
                    if (j == -1) {
                        insert_litab(tok3);
                        fprintf(ft, "(IS, %02d) %d (L, %02d)\n", i, k, litcnt - 1);
                    } else {
                        fprintf(ft, "(IS, %02d) %d (L, %02d)\n", i, k, j);
                    }
                } else {
                    p = search_symtab(tok3);
                    if (p == -1) {
                        insert_symtab(tok3);
                        fprintf(ft, "(IS, %02d) %d (S, %02d)\n", i, k, sc - 1);
                        S[sc - 1].addr = lc;  // Assign address to symbol
                    } else {
                        fprintf(ft, "(IS, %02d) %d (S, %02d)\n", i, k, p);
                    }
                }
            } else if (strcmp(tok2, "DS") == 0) {
                p = search_symtab(tok1);
                if (p == -1) {
                    insert_symtab(tok1);
                    fprintf(ft, "(DL, 02) (C, %s)\n", tok3);
                    S[sc - 1].addr = lc;  // Assign address to symbol
                }
                lc += atoi(tok3);
            } else if (strcmp(tok2, "DC") == 0) {
                p = search_symtab(tok1);
                if (p == -1) {
                    insert_symtab(tok1);
                    fprintf(ft, "(DL, 01) (C, %s)\n", tok3);
                    S[sc - 1].addr = lc;  // Assign address to symbol
                }
                lc++;
            }
        }

        lc++;
    }

    fclose(fs);
    fclose(ft);
}


// Print Symbol Table
void print_symbol_table() {
    printf("\nSymbol Table:\nIndex\tSymbol\tAddress\n");
    for (int i = 1; i < sc; i++) {
        printf("%d\t%s\t%d\n", i, S[i].sym, S[i].addr);
    }
}

// Print Literal Table
void print_literal_table() {
    printf("\nLiteral Table:\nIndex\tLiteral\tAddress\n");
    for (int i = 0; i < litcnt; i++) {
        printf("%d\t%s\t%d\n", i, L[i].lit, L[i].addr);
    }
}

// Print Pool Table
void print_pool_table() {
    printf("\nPool Table:\nPool\tStarting Index\n");
    for (int i = 1; i < poolcnt; i++) {
        printf("%d\t%d\n", i, search_pooltab(i));
    }
}

// Pass 2: Generates target machine code
void pass2() {
    char buffer[200], tok1[10], tok2[10], tok3[10], tok4[10], tok5[10];
    int i, j, k, n;

    FILE *fs = fopen("id.txt", "r");
    FILE *ft = fopen("tar.txt", "w");

    if (!fs || !ft) {
        printf("Error opening files.\n");
        return;
    }

    printf("\nMachine Code Output:\n");
    printf("LC    OPCODE  REGISTER  MEMORY\n");

    lc = 0; // Initialize location counter

    while (fgets(buffer, 80, fs)) {
        n = sscanf(buffer, "%s%s%s%s%s", tok1, tok2, tok3, tok4, tok5);

        switch (n) {
            case 2:  // Handle directives like LTORG and END
                if (strcmp(tok1, "(AD,") == 0 && atoi(tok2) == 5) {  // END directive
    for (j = search_pooltab(poolcnt - 1); j < litcnt; j++) {
        fprintf(ft, "%03d) 00 0 %03d\n", lc, L[j].addr);
        printf("%03d    00      0      %03d\n", lc, L[j].addr);
        lc++;
    }
    break;  // Stop processing further instructions
}
                
                break;

            case 4:  // Handle START and DS directives
                if (strcmp(tok1, "(AD,") == 0) {  // START directive
                    lc = atoi(tok4);  // Update location counter
                    break;
                }
                if (strcmp(tok1, "(DL,") == 0) {  // DS directive
                    int size = atoi(tok4);
                    for (j = 0; j < size; j++) {
                        fprintf(ft, "%03d) 00 0 000\n", lc);
                        printf("%03d    00      0      000\n", lc);
                        lc++;
                    }
                    break;
                }
                break;

            case 5:  // Handle machine instructions
                tok2[strlen(tok2) - 1] = '\0';  // Remove trailing ',' in tok2
                tok5[strlen(tok5) - 1] = '\0';  // Remove trailing ')' in tok5

                i = atoi(tok2);  // Opcode
                j = atoi(tok3);  // Register
                k = atoi(tok5);  // Operand index

                if (strcmp(tok4, "(S,") == 0) {  // Operand is a symbol
                    fprintf(ft, "%03d) %02d %d %03d\n", lc, i, j, S[k].addr);
                    printf("%03d    %02d      %d      %03d\n", lc, i, j, S[k].addr);
                } else if (strcmp(tok4, "(L,") == 0) {  // Operand is a literal
                    fprintf(ft, "%03d) %02d %d %03d\n", lc, i, j, L[k].addr);
                    printf("%03d    %02d      %d      %03d\n", lc, i, j, L[k].addr);
                }
                lc++;
                break;
        }
    }

    fclose(fs);
    fclose(ft);

    printf("\nPass 2 completed. Target code written to 'tar.txt'.\n");
}

// Print a file
void print_file(char *filename) {
    char buffer[200];
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Error: Unable to open file '%s'\n", filename);
        return;
    }
    printf("\nIntermediate Code:\n");
    while (fgets(buffer, sizeof(buffer), fp)) {
        printf("%s", buffer);
    }
    fclose(fp);
}

// Main function
int main() {
    initialize_hash_tables();

    char source[80];
    printf("Enter source file name: ");
    scanf("%s", source);

    pass1(source);
    print_file("id.txt");
    print_symbol_table();
    print_literal_table();
    print_pool_table();
    pass2();
    return 0;
}

