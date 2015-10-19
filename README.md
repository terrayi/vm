# VirtualMachine
Unsafe simple VM and assembler in C/C++ for in-memory execution or in standalone

This was an attempt to create a small and simple in-memory virtual machine to clone virtual machines used in some commercial executable protection but for being used in game loader possibly together with my loader framework. However that was never materalised neither by me nor by others whom I contributed this code to. So why not put it up just in case someone out there might have a use for it.

The VM itself is entirely in C for size and for portability so it won't be an issue to compile in any 32 bit platforms where as the assembler is coded in C++ to use std:vector and other features of C++.

It was so much fun coding this us.
