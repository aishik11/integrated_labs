%{ 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

void yyerror(const char *s); 
extern int yylex(void); 
extern int yylineno;
extern FILE *yyin; 

// Output file 
FILE *out_file; 

// Bytecode buffer 
long bytecode[4096]; 
int pc = 0; // Program counter is now an index into the long array

// Symbol table for labels 
#define MAX_LABELS 100 
struct label { 
    char *name; 
    long address; 
};
struct label symbol_table[MAX_LABELS]; 
int label_count = 0; 

int pass = 1; // Current pass 

// Function to add a label to the symbol table 
void add_label(char *name, long address) { 
    if (pass == 1) { 
        if (label_count < MAX_LABELS) { 
            symbol_table[label_count].name = strdup(name); 
            symbol_table[label_count].address = address; 
            label_count++; 
        } else { 
            yyerror("Too many labels"); 
        } 
    }
}

// Function to lookup a label 
long lookup_label(char *name) { 
    for (int i = 0; i < label_count; i++) { 
        if (strcmp(symbol_table[i].name, name) == 0) { 
            return symbol_table[i].address; 
        }
    }
    return -1;
}

// Function to emit a long
void emit_long(long value) {
    if (pass == 2) {
        bytecode[pc] = value;
    }
    pc++;
}
%} 

%union { 
    long ival; 
    char *sval;
}

%token <sval> T_ID 
%token <ival> T_INTEGER 

%token T_PUSH T_POP T_DUP T_PEEKPRINT T_HALT 
%token T_ADD T_SUB T_MUL T_DIV T_CMP 
%token T_AND T_OR T_XOR T_NOT T_SHL T_SHR 
%token T_JMP T_JZ T_JNZ 
%token T_STORE T_LOAD 
%token T_CALL T_RET
%token <sval> T_LABEL
%type <sval> label_def 

%%

program:
    | program line 
    ;

line:
    '\n' 
    | instruction '\n' 
    | label_def '\n' { add_label($1, pc); } 
    ;

label_def:
    T_LABEL { $$ = $1; } 
    ;

instruction:
    T_PUSH T_INTEGER { emit_long(0x01); emit_long($2); } 
    | T_POP { emit_long(0x02); } 
    | T_DUP { emit_long(0x03); } 
    | T_PEEKPRINT { emit_long(0x04); } 
    | T_HALT { emit_long(0xFF); } 
    | T_ADD { emit_long(0x10); } 
    | T_SUB { emit_long(0x11); } 
    | T_MUL { emit_long(0x12); } 
    | T_DIV { emit_long(0x13); } 
    | T_CMP { emit_long(0x14); } 
    | T_AND { emit_long(0x15); } 
    | T_OR { emit_long(0x16); } 
    | T_XOR { emit_long(0x17); } 
    | T_NOT { emit_long(0x18); } 
    | T_SHL { emit_long(0x19); } 
    | T_SHR { emit_long(0x1A); } 
    | T_JMP T_ID { 
        emit_long(0x20); 
        if (pass == 2) { 
            long addr = lookup_label($2); 
            if (addr == -1) { 
                yyerror("Label not found"); 
            }
            emit_long(addr); 
        } else { 
            emit_long(0); // Placeholder for address
        }
    } 
    | T_JZ T_ID { 
        emit_long(0x21); 
        if (pass == 2) { 
            long addr = lookup_label($2); 
            if (addr == -1) { 
                yyerror("Label not found"); 
            }
            emit_long(addr); 
        } else { 
            emit_long(0); // Placeholder for address
        }
    } 
    | T_JNZ T_ID { 
        emit_long(0x22); 
        if (pass == 2) { 
            long addr = lookup_label($2); 
            if (addr == -1) { 
                yyerror("Label not found"); 
            }
            emit_long(addr); 
        } else { 
            emit_long(0); // Placeholder for address
        }
    } 
    | T_STORE T_INTEGER { emit_long(0x30); emit_long($2); } 
    | T_LOAD T_INTEGER { emit_long(0x31); emit_long($2); } 
    | T_CALL T_ID { 
        emit_long(0x40); 
        if (pass == 2) { 
            long addr = lookup_label($2); 
            if (addr == -1) { 
                yyerror("Label not found"); 
            }
            emit_long(addr); 
        } else { 
            emit_long(0); // Placeholder for address
        }
    } 
    | T_RET { emit_long(0x41); } 
    ;

%%

void yyerror(const char *s) { 
    fprintf(stderr, "Error at line %d: %s\n", yylineno, s); 
}

// Overriding yywrap to link multiple files 
int yywrap() { 
    return 1;
}

// Redefine main to be in the parser file 
int main(int argc, char **argv) { 
    if (argc < 3) { 
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]); 
        return 1;
    }

    yyin = fopen(argv[1], "r"); 
    if (!yyin) { 
        perror(argv[1]); 
        return 1;
    }

    out_file = fopen(argv[2], "wb"); 
    if (!out_file) { 
        perror(argv[2]); 
        fclose(yyin); 
        return 1;
    }

    // First pass 
    pass = 1; 
    pc = 0; 
    yyparse(); 

    // Second pass 
    pass = 2; 
    pc = 0; 
    rewind(yyin); 
    yylineno = 1; 
    yyparse(); 

    // Write the bytecode 
    fwrite(bytecode, sizeof(long), pc, out_file); 

    fclose(yyin); 
    fclose(out_file); 

    printf("Assembly successful. Wrote %ld bytes.\n", pc * sizeof(long)); 

    return 0;
}
