# Memory in June

This goes over several things:

- What are the two types of memory in June?
- How is in-vm memory managed in June?
- How is all system memory managed in June?
  - Is it reliable?
  - Is it possible to change how memory is managed?

## Benchmarks

|               ns/op |                op/s |    err% |     total | benchmark
|--------------------:|--------------------:|--------:|----------:|:----------
|              733.76 |        1,362,834.70 |    2.4% |      0.00 | `PoolAllocation`

## Introduction

Memory is a critical to any program from an OS to our VM here. Unfortunately, there are programs that do not manage memory well. Over the course of this document, we will define 2 types of memory in June. In-VM memory, specific to the VM and able to managed not only from C++ but also from the JuneVM execution loop. The second is system memory, only managed from C++. Additionally, it'll outline how memory is managed for both in-vm and system memory.

## What are the two types of memory in June?

In June there are 2 types of memory, In-VM memory and system memory. As outlined in the introduction, in-vm memory is manageable from C++ and June. System memory however can only be managed from C++.

### In-VM memory

In-VM memory is managed by the VM processes. While this does include system memory, the main difference is the access allowed to C++ and June itself. June is only allowed to access the in-vm memory, this includes but is not limited to the: variables, stack, typedefs, and functions/methods available. This is a relatively small portion of memory available to the VM but it is still able to manage many of the low level details. For example, while VM memory *is* managed by the VM's automatic processes, later we'll discuss managing the VM memory manually using built-in core libraries.

### System memory

System memory is managed by the OS and C++. This includes in-vm memory as a subset but for the sake of this section, we'll only focus on non-vm memory. This includes but is not limited to: bytecode, vm state, dynamic libraries, source files, etc. This is a much larger portion of memory and very important as it handles many of the lower-level things.

## How is in-vm memory managed in June?

TODO: Document the memory management happening for the VM

### Is it reliable?

Well... is it? TODO.

### Is it possible to change how memory is managed?

In theory, yes. In practice, we're unsure! TODO.
