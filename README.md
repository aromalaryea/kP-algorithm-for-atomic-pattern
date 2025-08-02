# Implementation of Giraud-Verneuil atomic pattern kP-algorithm.
This project is an implementation of the Right-to-Left (R2L) scalar multiplication algorithm based on the Giraud-Verneuil atomic pattern, targeting Elliptic Curve Cryptosystems (ECC). The implementation is done for a Texas Instruments microcontroller environment, using Code Composer Studio (CCS).

The aim is to investigate the distinguishability of Giraud-Verneuil atomic blocks on embedded systems.

## 🔍 Main File: R2LRam_main.c

This file contains the core implementation of the Right-to-Left (R2L) scalar multiplication algorithm using Giraud and Verneuil’s atomic blocks. The algorithm is written in C and implemented using the FLECC_IN_C cryptographic library, which provides essential field and ECC operations.

If you’re looking for the core logic of the algorithm, start here.

## 🛠 Development Environment

This project is configured for use with Code Composer Studio (CCS) by Texas Instruments. The rest of the files include:

    Linker command files (.cmd): Manage memory mapping for the MCU

    Device configuration folders: Set up the RAM, CPU, and system behavior

    Assembly startup files: Handle interrupt vectors and system startup

    Support files: GPIO, Pie control, ISR routines, and delay functions

The project is set up specifically for TI’s **LAUNCHXL-F28379D** development board with the **TMS320F28379D** MCU.
