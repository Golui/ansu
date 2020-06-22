## ansu

### **a** **n**uż **s**ię **u**da (Maybe it'll work)

ansu is an implementation of the tANS compression algorithm, targeting Xilinx FPGAs. ansu also features a software backend, aimed mostly at debugging the logic. It has no external dependencies, aside from what is included in this repository.

The project is currently no more than a proof-of-concept. The existing implementation is able to process on average one symbol per FPGA clock cycle, by way of running multiple decompressors in parallel and merging the streams together.

Currently, ansu compressed files are tarballs containing a primitive header file, data blocks and accompanying metadata. **At the time, these files are not cross platform compatible, endianness is not taken into account whatsoever**.

### Libraries:
* CLI11 - command line interface
* microtar - tar archive management

### Installation:

```sh
make
```

### Usage:
```sh
./ansu --help
```

## TODO:
* Wrap exceptions in a user-friendly manner
* Proper Vivado HLS project
    * Tests on hardware
    * Optimization of processor - FPGA communication
* Archive
    * Integrity verification
    * Better TAR implementation/custom format
    * Embedded compression tables
* Dynamic compression table generation
