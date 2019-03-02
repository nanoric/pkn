# pkn
core of pkn game hacking project. Including mainly for process management, memory management, and DLL injecttion. Also PE analysis, windows registry management, compile-time sting encryption, byte-code emulator, etc. Most of them can run under kernel mode.

## Notice
Don't use these code to do anything illegal.

## Useful parts:
 * core/base   type support, compile-time string/number encryption(xor based only), randomize generator, string hash.
 * core/pe_structure: As the name says. PE format analysizer. Worked under kernel mode. Used for Manual Map.
 * core/remote_process:  Anything you want about Windows Process: Read, Write, List, Info, Memory, Thread, etc. Both Kernel Mode and User Mode.
 * kernel      As the name says.
 * utils/dsefix     Load kernel driver by disabling Driver Signature Enforcement.
 * tools/cpuz_based_loader  a sample that load kernel driver using dsefix and mmap.

other parts are either too old or just some code for test.
