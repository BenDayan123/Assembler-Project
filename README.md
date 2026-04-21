# 12-Bit Assembler Project

A simple and good Assembler written in **C (ANSI C90 standard)**.

This is a project I built for my "Systems Programming Laboratory" course (2026A) at the Open University of Israel. It’s a full assembler written in C, following the ANSI C90 standard.

## 🏗️ How it Works

The assembler processes the input files (.as) in three main steps:

### 1. Pre-Assembler (`pre_assembler.c`)

This is the first stage where the program cleans up the code.

- **Macro Expansion:** It finds all macro blocks (between mcro and mcroend).
- **Processing:** It saves the macros in memory and replaces every macro call with the actual lines of code.
- **Output:** It creates an expanded .am file, which is now ready for the next pass.

### 2. First Pass (`first_pass.c`)

This is where the program does the heavy lifting of checking the code.

- **Syntax Validation:** It looks for errors, bad commands, or wrong addressing modes.
- **Memory Calculation:** It tracks the Instruction Counter (IC) and Data Counter (DC) to see how much memory is needed.
- **Symbol Table:** It builds a table of all the labels and symbols used in the code.

### 3. Second Pass (`second_pass.c`)

The final stage that generates the actual machine code.

- **Final Encoding:** It converts all instructions and data into their final binary format.
- **Symbol Resolution:** It fills in the final addresses for all the labels.
- **Output Files:** If everything is correct, it generates the final output files.

---

## 🚀 How to Build and Run

### Requirements

- GCC Compiler (GNU Compiler Collection)
- Make (for makefile)

### Compilation

Open your terminal and compile the project using the following command:

```sh
make main
```

> <ins>**Important!</ins>**
> Make sure the folders 'build' and 'output' are exists before running, the "make main" command creats the 'output' folder automatically.

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

- `output/test.am`
- `output/test.ob`
- `output/test.ent` (if entries exist)
- `output/test.ext` (if externals exist)

---

### 📂 Output Files Description

| Extension | Description                                                                                            |
| :-------- | :----------------------------------------------------------------------------------------------------- |
| **.ob**   | **Object File:** The final machine code in hex format, showing memory addresses and A.R.E. attributes. |
| **.ent**  | **Entries File:** Created if you used .entry, listing labels that other files can access.              |
| **.ext**  | **Externals File:** Created if you used .extern, showing where external labels are used in the code.   |

---

### 👨‍💻 Author - Ben Dayan ©

- Project created as part of the Systems Programming Laboratory course.

- **Focus**: Low-level programming, Memory management, and Data structures in C.
