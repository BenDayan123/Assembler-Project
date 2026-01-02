# 12-Bit Assembler Project

![Language](https://img.shields.io/badge/language-C-blue.svg)
![Standard](https://img.shields.io/badge/std-C90-green.svg)

A comprehensive Assembler written in **C (ANSI C90 standard)**.
This program translates assembly language source code into machine code for a hypothetical 12-bit CPU architecture. It handles macro expansion, symbol resolution, and binary encoding through a multi-stage process.

## 🏗️ Architecture & Workflow

The assembler processes input files (`.as`) in three distinct stages:

### 1. Pre-Assembler (`pre_assembler.c`)

The first phase scans the source code for macros.

- **Macro Expansion:** Identifies macro definitions blocks (enclosed by `mcro` and `mcroend`).
- **Processing:** It adds the macro content to a memory dictionary and replaces every occurrence of the macro name with its corresponding code lines.
- **Output:** Generates an intermediate file (`.am`) with all macros expanded, ready for the main assembly process.

### 2. First Pass (`first_pass.c`)

This stage performs the initial lexical and syntactic analysis of the `.am` file.

- **Syntax Validation:** Checks for syntax errors, invalid commands, or addressing mode violations.
- **Memory Calculation:** Calculates the Instruction Counter (IC) and Data Counter (DC) to determine the memory footprint.
- **Symbol Table Construction:** Identifies labels (Symbols) and stores them in a symbol table with their relative addresses and attributes (Code, Data, External).

### 3. Second Pass (`second_pass.c`)

The final stage completes the translation to machine code.

- **Encoding:** Converts instructions and data into 12-bit binary words (represented in Hexadecimal).
- **Address Resolution:** Resolves the addresses of operands using the symbol table created in the first pass.
- **Entry/Extern Handling:** Marks entry points and records external symbol usage.
- **Output Generation:** Produces the final object file (`.ob`), entries file (`.ent`), and externals file (`.ext`).

---

## 🚀 How to Build and Run

### Prerequisites

- GCC Compiler (GNU Compiler Collection)
- Make (for makefile)

### Compilation

Open your terminal and compile the project using the following command:

```sh
make main
```

### Running

Run the assembler by providing the file names as command-line arguments:

```bash
./main file1 file2...
```

> <ins>**Important!</ins>**
> Do not include the .as extension in the arguments. For example, pass 'my_file' instead of 'my_file.as'. The program appends the extension automatically.

**For Example**

If you have a file named `test.as`:

1. **And Run**: `./main test`
2. So the program will generate:

- `output/test.ob`
- `output/test.ent` (if entries exist)
- `output/test.ext` (if externals exist)

---

### 📂 Output Files Description

| Extension | Description                                                                                                                                                                                                                                                                |
| :-------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **.ob**   | **Object File:** Contains the final machine code in hexadecimal format. The first line lists the size of the Code and Data sections. The following lines show the memory address, the 12-bit content (in hex), and the A.R.E. attribute (Absolute, Relocatable, External). |
| **.ent**  | **Entries File:** generated only if the code contains `.entry` directives. It lists the names of labels defined in this file that are exported for use in other files, along with their memory addresses.                                                                  |
| **.ext**  | **Externals File:** Generated only if the code uses external symbols (defined via `.extern`). It lists the name of the external label and the memory addresses where it is referenced in the code.                                                                         |

---

### 👨‍💻 Author - Ben Dayan ©

- Project created as part of the Systems Programming Laboratory course.

- **Focus**: Low-level programming, Memory management, and Data structures in C.
