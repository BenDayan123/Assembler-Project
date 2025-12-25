#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================= */
/* CONSTANTS & MACROS                        */
/* ========================================= */

/** Boolean logic for ANSI-C (C90) */
typedef enum
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
/* MACHINE ARCHITECTURE ENUMS                */
/* ========================================= */

/** Supported CPU Opcodes */
typedef enum
{
    /* Two Operands Group */
    MOV = 0,
    CMP,
    ADD,
    SUB,
    LEA,

    /* One Operand Group */
    NOT,
    CLR,
    INC,
    DEC,
    JMP,
    BNE,
    RED,
    PRN,
    JSR,

    /* No Operands Group */
    RTS,
    STOP,

} opcode_name;

/**
 * The actual dictionary implementation.
 * ORDER MUST MATCH THE 'opcode_name' ENUM IN GLOBALS.H!
 */
const cmd_info operations[] = {
    /* { "name", opcode, funct, operand_count } */
    {"mov", 0, 0, 2},  /* MOV (0) */
    {"cmp", 1, 0, 2},  /* CMP (1) */
    {"add", 2, 10, 2}, /* ADD (2) */
    {"sub", 2, 11, 2}, /* SUB (3) */
    {"lea", 6, 0, 2},  /* LEA (4) */
    {"not", 4, 10, 1}, /* NOT (5) */
    {"clr", 5, 10, 1}, /* CLR (6) */
    {"inc", 5, 11, 1}, /* INC (7) */
    {"dec", 5, 12, 1}, /* DEC (8) */
    {"jmp", 9, 10, 1}, /* JMP (9) */
    {"bne", 9, 11, 1}, /* BNE (10)*/
    {"red", 11, 0, 1}, /* RED (11)*/
    {"prn", 12, 0, 1}, /* PRN (12)*/
    {"jsr", 9, 12, 1}, /* JSR (13)*/
    {"rts", 14, 0, 0}, /* RTS (14)*/
    {"stop", 15, 0, 0} /* STOP(15)*/
};

/** CPU Registers */
typedef enum
{
    R0 = 0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
} reg;

/**
 * The "Value" structure of our dictionary.
 * Holds all metadata for a specific command.
 */
typedef struct
{
    char *name;   /* The string representation (e.g., "mov") */
    int opcode;   /* The machine opcode (e.g., 2 for ADD/SUB) */
    int funct;    /* The funct code (e.g., 10 for ADD, 11 for SUB) */
    int op_count; /* How many operands are required (0, 1, or 2) */
} cmd_info;

/** Addressing Modes for Operands */
typedef enum
{
    IMMEDIATE_ADDR = 0, /* e.g., #5 */
    DIRECT_ADDR = 1,    /* e.g., LABEL */
    INDEX_ADDR = 2,     /* e.g., LABEL[r2] */
    REGISTER_ADDR = 3,  /* e.g., r3 */
} addressing_mode;

/* ========================================= */
/* GLOBAL STATE                              */
/* ========================================= */

extern const cmd_info operations[];

#endif