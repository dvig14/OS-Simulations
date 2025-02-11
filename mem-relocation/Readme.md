# Overview

This program allows you to see how address translations are performed in a
system with base and bounds registers. As before, there are two steps to
running the program to test out your understanding of base and bounds. First,
run without the -c flag to generate a set of translations and see if you can
correctly perform the address translations yourself. Then, when done, run with
the -c flag to check your answers.

```sh
  -------------- 0KB
  |    Code    |
  -------------- 2KB
  |   Stack    |
  -------------- 4KB
  |    Heap    |
  |     |      |
  |     v      |
  -------------- 7KB
  |   (free)   |
  |     ...    |
```

In the figure, the bounds register would be set to 7~KB, as that represents
the end of the address space. References to any address within the bounds
would be considered legal; references above this value are out of bounds and
thus the hardware would raise an exception.

To run with the default flags, type relocation.py at the command line. The
result should be something like this:

```sh
prompt> ./build.sh relocation
...
Base-and-Bounds register information:

  Base   : 0x0000193d (decimal 6461)
  Limit  : 471

Virtual Address Trace
  VA  0: 0x00000321 (decimal 801) -> PA or violation?
  VA  1: 0x00000331 (decimal 817) -> PA or violation?
  VA  2: 0x000003a5 (decimal 933) -> PA or violation?
  VA  3: 0x000000ca (decimal 202) -> PA or violation?
  VA  4: 0x00000157 (decimal 343) -> PA or violation?
```

For each virtual address, either write down the physical address it 
translates to OR write down that it is an out-of-bounds address 
(a segmentation violation). For this problem, you should assume a 
simple virtual address space of a given size.

As you can see, this simply generates randomized virtual
addresses. For each, you should determine whether it is in bounds, and if so,
determine to which physical address it translates. Running with -c (the
"compute this for me" flag) gives us the results of these translations, i.e.,
whether they are valid or not, and if valid, the resulting physical
addresses. For convenience, all numbers are given both in hex and decimal.

```sh
prompt> ./build.sh relocation -c
...
Virtual Address Trace
  VA  0: 0x00000321 (decimal 801) -> SEGMENTATION VIOLATION
  VA  1: 0x00000331 (decimal 817) -> SEGMENTATION VIOLATION
  VA  2: 0x000003a5 (decimal 933) -> SEGMENTATION VIOLATION
  VA  3: 0x000000ca (decimal 202) -> VALID: 00001a07 (dec:6663)
  VA  4: 0x00000157 (decimal 343) -> VALID: 00001a94 (dec:6804)
```

With a base address of 6461 (decimal), address 343 is within bounds (i.e., it
is less than the limit register of 471) and thus translates to 343 added to
6461 or 6663. A few of the addresses shown above are out of bounds (933,
801), as they are in excess of the bounds. Pretty simple, no? Indeed, that is
one of the beauties of base and bounds: it's so darn simple!

There are a few flags you can use to control what's going on better:

```sh
prompt> ./build.sh relocation -h
Usage: ./build.sh relocation [options]

Options:
  -h, --help            show this help message and exit
  -s SEED, --seed the random seed
  -a ASIZE, --asize address space size (e.g., 16, 64k, 32m)
  -p PSIZE, --physmem physical memory size (e.g., 16, 64k)
  -n NUM, --addresses # of virtual addresses to generate
  -b BASE, --base     value of base register
  -l LIMIT, --limit   value of limit register
  -c, --compute         compute answers for me
```

In particular, you can control the virtual address-space size (-a), the size
of physical memory (-p), the number of virtual addresses to generate (-n), and
the values of the base and bounds registers for this process (-b and -l,
respectively).