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
