# sc
SC is a modem C++ bare metal framework which provides OS type primities and libraries that allow to write code in a similar way you would write a userspace application.

## Build status
![build status](https://github.com/glfernando/sc/workflows/SC%20CI/badge.svg)

## Features
- Written in modem C++
- Minimal C++ standard is C++20 (-std=c++2a)
- Uses the **Big Four** C++20 features:
  - Modules
  - Concepts
  - Coroutines (no supported yet)
  - Ranges (no supported yet)
- Compact, it should be suitable for microcontrollers with 100KB+ of flash and 60KB+ of RAM
- Dynamic memory support
- C++ expcetions support

## Requirements
SC only supports clang compiler, the recommended version is clang-11 but minimal version that can be used to compile is clang-10. It also uses LLVM linker and llvm-objcopy.
### MacOs
Download clang-11 binary prebuilt from https://releases.llvm.org/download.html. Extract it and add the bin directory to your $PATH. Initial development is done in QEMU, you can use homebrew or macports to install it
```sh
brew install qemu
```
```sh
sudo port install qemu
```
Also, binutils is required to compile final binary
```sh
brew install binutils
```
```sh
sudo port install binutils
```

### Ubuntu
If you are using ubuntu or other linux distribution you can use the package manager to install clang-11, lld linker and qemu
```
sudo apt-get install clang-11
sudo apt-get install lld-11
sudo apt-get install qemu-system-arm
```

# How to Build
Due the lack c++20 modules support in the major build system, currently we are using a very simple build system using Makefiles (check section [Build System](#build-system) for more info). To compile you just need to execute make follow by TARGET=<board name>. Currently, only 2 targets are supported.
- Aarch64 qemu
- Raspberry pi 4
  
You can compile them like this
```
make TARGET=qemu-aarch64 -j8
```
```
make TARGET=rpi4 -j8
```
If not TARGET is passed then it defaults to qemu-aarch64.

Output files can be found under `build-<target name>/` directory. The important ones are:
- sc.elf -> SC ELF file
- sc.bin -> SC binary, the one that you will use in your device.

# How to run
## QEMU
Just execute aarch64 qemu using `sc.bin` as the kernel
```
qemu-system-aarch64 -M virt,secure=on,virtualization=on -cpu cortex-a53 -m 512 -smp 1 -nographic -semihosting -kernel build-qemu-aarch64/sc.
```
Or you can use the qemu target defined by the qemu-aarch64 board
```
make TARGET=qemu-aarch64 qemu -j8
```
Either way you will end up with the SC shell in the current terminal
```
Welcome to SC
scsh> 
```
There is no much now, but more and more commands will be added for debugging. Currently, you can use `mem` command to read/write memory/registers
```
scsh> mem
mem [options] <[0x]hex_addr>[:size|..<[0x]hex_range> [value]
options:
   -o          print offset during memory dump
   -a          print ascii code during memory dump
   -g <num>    group by <num> during memory dump
Note:
mem command enter memory dump mode when size is different from 1/2/4/8
scsh> mem 40000000
580000c0
scsh> mem 40000000 0xcafecafe
scsh> mem 40000000
cafecafe
scsh> 
```

## Raspberry PI 4
You can use `sc.bin` as your kernel8.img in your sdcard. It should boot to the SC shell. Later on, more support will be added to allow sending the kernel image via UART or USB.

# Directory structure
This is the current directory structure
```
% tree -L 1
.
├── LICENSE
├── Makefile
├── README.md
├── external
├── make
├── sc_linker.lds
├── scripts
├── src
│   ├── app
│   ├── arch
│   ├── board
│   ├── core
│   ├── device
│   ├── include
│   ├── lib
│   ├── libc
│   ├── libcxx
│   └── libcxxabi
└── target
```
- **external:**   code taken from other place. currently only libunwind is used for supporting exceptions. The idea is to have as minimal as possible external dependencies
- **make:**       Additional makefile which can provide either new functionality or makefiles for different ARCH or CPUs
- **scripts:**    host scripts
- **src:** This is whre all SC code is, it is divided into subdirectories like this:
  - **app:** Application code, code that runs when the system is complete up and it can make use of all libraries
  - **arch:** Code specific for HW architecture
  - **board:** All code related to a specfic board, such as QEMU, Raspberry PI, etc.
  - **core:** SC core code, this code provides OS primites that for rest of the code, such as threads, mutex, events, etc.
  - **device:** Device driver code goes here
  - **lib:** Everything that can be packed as a library should be here
  - **libc:** All C code providing standard lib C like features goes here, such as printf, strcpy, memcpy. We try to use as less as possible C code.
  - **libcxx:** All C++ code that provides a STD C++ like features goes here, such as vector, string, etc. However, it should be written as modern C++, that means they should be as a c++ 20 module.
  - **libcxxabi:** C++ ABI code, currently only minimal needed to support C++ exceptions is implemented.
- **target**: Makefile describing a target
    
# Build system
Since we are using C++ 20 modules feature, we need a build system that support them. However, current status of build2, bazel, cmake, etc is that the support is not there or just experimental. After trying to use them, it was clear I could not use them at the moment, so we are using our own build system based on Makefiles.

The long tearm plan is to migrate to one of the major build system once they fully support C++20 modules, short term is use Makefiles and medium term is improve (make less painful) using Mafiles.

## Module makefile
This is an example of a module makefile:
`src/board/qemu/aarch64/Makefile`
```
LOCAL_DIR = $(GET_LOCAL_DIR)

mod_srcs += $(LOCAL_DIR)/peripherals.cppm
mod_srcs += $(LOCAL_DIR)/power.cppm

dirs := $(LOCAL_DIR)/init
dirs += $(LOCAL_DIR)/debug

include $(foreach dir,$(dirs),$(dir)/Makefile
```
Use `mod_srcs` to specified C++ module interface source files (we only support module interface units and not module implementation units), use `dirs` to add more directories to the current compilation, you need to call those directories manually by doing `include $(foreach dir,$(dirs),$(dir)/Makefile`. You can also add C/C++ or assembly files (.S files) but you need to use `srcs` variable.

If a source file should condionally added based on the arch you can do something like this:
```
ifeq ($(ARCH), aarch64)
mod_srcs += $(LOCAL_DIR)/lock_aarch64.cppm
endif
```

Device driver code should be under a config name
```
ifeq ($(CONFIG_PL011), y)
mod_srcs += $(LOCAL_DIR)/pl011.cppm
endif
```
Then CONFIG_PL011 can be set in a board `config.h` file.

# TODO
- [x] Build system using C++ modules
- [x] Qemu Build
- [x] SC shell
- [x] Debug Command to read memory
- [x] Support for Dynamic memory
- [x] Support for spinlocks
- [x] Support for C++ exceptions
- [x] Support for C++ std string
- [x] Support for C++ vector
- [x] Support for Real HW, e.g. Raspberry PI
- [x] Support for Concepts
- [x] Support for C++ type traits (partial)
- [x] Support for handling AARCH64 HW exceptions
- [ ] Support for unique_ptr
- [ ] Support for interrupts
- [ ] Support for timers
- [ ] Support for threads
- [ ] Support for blocking and waking up treads
- [ ] Support for Events
- [ ] Support for Mutex
- [ ] Support for Coroutines
- [ ] Support for Ranges
