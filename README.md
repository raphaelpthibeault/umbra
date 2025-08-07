# umbra OS

umbra is an operating system for x86-64 PCs with plans to support more architectures in the future.

## Notable Components
- **Bootloader**, [bootloader/](bootloader/), a multiboot2-compliant hard-drive boot bootloader
- **Kernel**, [kernel/](kernel/), a monolithic kernel, and the core of the operating system

## Current Goals

The following are in progress:

- Finish memory management
- Implement tasking and SMP
- Add a network stack
- Implement a libc
- Support more hardware drivers (e.g. USB, audio...)
- Support more architectures (e.g. ARM)

## Building / Installation

### Building From Scratch

Currently, the only supported build method is installing the x86-64 toolchain manually (I'll add a Dockerfile eventually). Here is a tutorial on building toolchains [from OSDEV](https://wiki.osdev.org/GCC_Cross-Compiler).

## Running umbra

### QEMU
Development of umbra is done with QEMU since it provides the best debugging experience. By default, the make commands will use QEMU.

```
make run
```

The command `make run` will build and run with the following options `qemu-system-x86_64 -drive format=raw,file=umbra.hdd -no-reboot -M q35 -m 2G -serial mon:stdio`

Note: in general QEMU flags are not stable. 



