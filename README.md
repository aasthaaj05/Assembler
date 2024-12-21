# Assembler with Macro Processing
This project is designed to convert assembly language code into machine code. It is structured into two main parts: Macro Expansion and Assembler Implementation

## Macro Expansion
The macro expansion phase processes macro definitions in the assembly code and expands them during assembly. This phase consists of two passes:
### Pass 1: Macro Registration
1. Registers macros in the Macro Name Table (MNT) and Macro Definition Table (MDT).
2. Maintains an Argument List Array (ALA) to handle macro arguments and their corresponding values.
### Pass 2: Macro Expansion
1. Uses the output from Pass 1 to expand macros in the assembly code.
2. Replaces macro calls with their corresponding expanded code, ensuring accurate substitution of arguments.

## Assembler Implementation
The second part of the project involves the assembler itself, which translates the expanded assembly language code into machine code. This includes:

1. Lexical Analysis: Breaking the code into tokens.
2. Syntax and Semantic Analysis: Validating instructions, operands, and addressing modes.
3. Code Generation: Producing the final machine code output.
