# ELF-Symbol-Inspector-Tool
This project implements a tool that prints information about symbols used by an ELF binary. The tool works with both stripped and unstripped binaries.

Features:

    Symbol Table Analysis: Displays all symbols of the symbol table when the binary is not stripped.
    
    Dynamic Symbol Table Analysis: Displays all symbols of the dynamic symbol table.
    
    Detailed Symbol Information: For each symbol, prints the name, type, value, and relevant       section (in hexadecimal).

Implementation Details:

    Developed in C using libbfd or libelf for symbol manipulation.
    
    The tool opens an ELF executable binary to extract and display symbol information.
    
    Ensures compatibility with both stripped and unstripped binaries by checking symbol tables.

Files Included:

    Source Code: Contains the implementation code.
    
    Makefile: For building the project.
    
    Documentation: Includes a description of the code structure and any additional features.

