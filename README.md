# Microkit x86 machine dump

Microkit needs to know the details of the target hardware at compile
time. On embedded ARM and RISC-V, [device
tree](https://en.wikipedia.org/wiki/Devicetree) files are used to
describe the target. On x86 however the complete hardware details are
generally not known at compile time and are discovered at runtime
using mechanisms such as the
[ACPI](https://en.wikipedia.org/wiki/ACPI).

This repository builds a tool that can be booted on x86 hardware to
perform the runtime discovery of specific hardware details that are
required by Microkit to build a static OS image.

## Usage

The `machinedump` file is a Multiboot2 compatible ELF file that can be
loaded by a compatible boot loader such as
[GRUB](https://www.gnu.org/software/grub/). On boot it will collect
information about the target hardware and produce the `machine.json`
file on the first serial port. This file is necessary to build static
Microkit OS images on x86.

Some options can be passed at runtime via the multiboot2 command line.
The following options are recognised:

**serial=\<PORT>**

Specifiy the serial port to use in hexadecimal notation, e.g.
`serial=0x2f8`. By default all outputs are produced on the first
serial port at 0x3f8. Another port can be specified here, such as
0x2f8 for the second serial port. This is intended for servers with a
BMC that can remotely expose certain serial ports.

**on_exit={hang|reboot|shutdown}**

Specify the action to perform on exit. By default the computer will be
put in halted state. With `on_exit=reboot` the computer will be
rebooted. With `on_exit=shutdown` the computer will be halted. Note
that so far this does not use the proper ACPI mechanisms and may not
work on your system. In particular `shutdown` is only supported when
running in [QEMU](https://www.qemu.org/).
