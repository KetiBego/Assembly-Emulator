# Assembly-Emulator
This is Assembly emulator. It reads assembly code and executes it in c++.
This program knows how to do store, load, and ALU operations, as well as understand how to do branching, JUMP operations, and call and execute functions.
This package comes with 3 tests for assembly code:
* The first sample (`Assembly_Sample.txt`) tests basic functions as addition, subtraction, multiplication and division.
* The second sample (`Assembly_Sample_2.txt`) tests recursion by printing 10th number in fibonacci sequence.
* The third sample (`Assembly_Sample_3.txt`) tests casting by doing operations on a char type variable.
You can see additional information in the test files itself (each one of them includes c++ version of the code in the comments, as well as necessary information about the code).
Main code always prints return value in the end of the code. 
## Syntax to Use
* You can use `//` in the beginning of the line to write comments.
* No semicolon should be used at the end of the line.
* To declare a function, use the following format: `function_name:`.
* When using branch operations, use commas (`,`) after both values, i.e. `BNE R1, R2, PC + ...`
## How to Run the Code
Use command `g++ emulator.cpp && ./a.out` 