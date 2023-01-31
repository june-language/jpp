# Memory in June

This goes over several things:

- What are the two types of memory in June?
- How is in-vm memory managed in June?
- How is all system memory managed in June?
  - Is it reliable?
  - Is it possible to change how memory is managed?

## Introduction

Memory is a critical to any program from an OS to our VM here. Unfortunately, there are programs that do not manage memory well. Over the course of this document, we will define 2 types of memory in June. In-VM memory, specific to the VM and able to managed not only from C++ but also from the JuneVM execution loop. The second is system memory, only managed from C++. Additionally, it'll outline how memory is managed for both in-vm and system memory.

## What are the two types of memory in June?

In June there are 2 types of memory, In-VM memory and system memory. As outlined in the introduction, in-vm memory is manageable from C++ and June. System memory however can only be managed from C++. 

### In-VM memory

TODO: Document In-VM memory

### System memory

TODO: Document system memory

## How is in-vm memory managed in June?

TODO: Document the memory management happening for the VM

## How is all system memory managed in June?

TODO: Document the memory management happening for the entire system

### Is it reliable?

Well... is it? TODO.

### Is it possible to change how memory is managed?

In theory, yes. In practice, we're unsure! TODO.
