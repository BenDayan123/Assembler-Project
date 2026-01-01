#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================= */
/* CONSTANTS & MACROS                        */
/* ========================================= */

/* Boolean logic for ANSI-C (C90) */
typedef enum boolean
{
    FALSE = 0,
    TRUE = 1
} boolean;

#define MAX_LINE_LEN 82   /* Max line length in source file (+ \n and \0) */
#define MAX_LABEL_LEN 31  /* Max length of a label (e.g., "MAIN:") */
#define MEMORY_SIZE 4096  /* Virtual memory size of the target machine */
#define IC_INIT_VALUE 100 /* Initial Instruction Counter address */

/** A | R | E Encoding (2 bits) */
#define ARE_ABSOLUTE 0    /* A: Absolute value */
#define ARE_EXTERNAL 1    /* E: External reference */
#define ARE_RELOCATABLE 2 /* R: Relocatable (internal label) */

/* ========================================= */
/* MACHINE ARCHITECTURE                      */
/* ========================================= */

/* Supported CPU Opcodes */
typedef enum
{
    MOV = 0,
    CMP,
    ADD,
    SUB,
    LEA,
    CLR,
    NOT,
    INC,
    DEC,
    JMP,
    BNE,
    JSR,
    RED,
    PRN,
    RTS,
    STOP
} opcode_name;

/*
 * The "Value" structure of our dictionary.
 * Holds all metadata for a specific command.
 */
typedef struct CmdInfo
{
    char *name;   /* The string representation (e.g., "mov") */
    int opcode;   /* The machine opcode (e.g., 2 for ADD/SUB) */
    int funct;    /* The funct code (e.g., 10 for ADD, 11 for SUB) */
    int op_count; /* How many operands are required (0, 1, or 2) */
} CmdInfo;

/* Addressing Modes for Operands */
typedef enum addressing_mode
{
    IMMEDIATE_ADDR = 0, /* e.g., #5 */
    DIRECT_ADDR = 1,    /* e.g., LABEL */
    INDEX_ADDR = 2,     /* e.g., LABEL[r2] */
    REGISTER_ADDR = 3   /* e.g., r3 */
} addressing_mode;

/* ========================================= */
/* GLOBAL VARIABLES                          */
/* ========================================= */

extern const CmdInfo operations[];

#endif