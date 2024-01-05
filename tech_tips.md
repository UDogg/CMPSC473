# These are some helpful tech tips for this class.
1. How to compile a C file
    I already have gcc installed so we can navigate to the repo with the C file and type:
    ```bash
    gcc -Wall -g main.c -o main
    ```
For example, if you have a C source file named main.c
and you want to compile it into an executable file named main
you would use the above command. To run the executable file just run ./main  
  
To run the executable with gdb for debugging execute the following:
2. How to debug a C file
    I already have gdb installed so run the following:  
    ```
    gdb ./main
    ```

Here are some useful GDB commands along with their descriptions:

- **`run` or `r`**: Executes the program from start to end.
- **`break` or `b`**: Sets a breakpoint on a particular line or function.
- **`next` or `n`**: Executes the next line of code without diving into functions.
- **`step` or `s`**: Goes to the next instruction, diving into the function if the instruction is a function call.
- **`print` or `p`**: Displays the value of a variable.
- **`quit` or `q`**: Exits out of GDB.

For example, if you want to set a breakpoint at line 10,
 you would use the command `break 10` or `b 10`.
 Then, you can start the program with the `run` command.
 When the program execution reaches line 10,
 it will stop, and you can inspect the state of your program.`

# Handy x86 Assembly Commands

When inspecting x86 assembly code, understanding certain commands can be beneficial. Here are some handy x86 assembly commands:

### Registers

- **`mov`**: Move data from one location to another.
- **`add`**: Add two operands.
- **`sub`**: Subtract the second operand from the first.
- **`inc`**: Increment the value of a register or memory location.
- **`dec`**: Decrement the value of a register or memory location.
- **`cmp`**: Compare two operands.
- **`lea`**: Load effective address.

### Control Flow

- **`jmp`**: Unconditional jump to a specified address.
- **`je` or `jz`**: Jump if equal or jump if zero.
- **`jne` or `jnz`**: Jump if not equal or jump if not zero.
- **`jl` or `jb`**: Jump if less than or jump if below.
- **`jg` or `ja`**: Jump if greater than or jump if above.
- **`call`**: Call a procedure.

### Stack Operations

- **`push`**: Push a value onto the stack.
- **`pop`**: Pop a value from the stack.
- **`mov %eax, %ebx`**: Move the value in `%eax` to `%ebx`.
- **`mov %rbp, %rsp`**: Move the value in `%rbp` to `%rsp`

### Data Movement

- **`movsb`**, **`movsw`**, **`movsd`**: Move bytes, words, or double-words from one location to another.

### Miscellaneous

- **`nop`**: No operation, essentially does nothing.
- **`int`**: Software interrupt.
- **`syscall`**: Invoke a system call (on 64-bit systems).

### SIMD (Single Instruction, Multiple Data)

- **`xmm`**: Registers used for SIMD operations.
- **`movaps`**: Move aligned packed single-precision floating-point values.

### Debugging

- **`int 3`**: Software breakpoint for debugging.
- **`nop` or `xchg`**: Used for patching code during debugging.


