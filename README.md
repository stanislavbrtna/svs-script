# SVS

SVS ( SDA Virtuous Script) is simple script interpreter written for my DIY PDA project. Its main goal is simple integration to applications in C and capability to run on microcontrollers as well as on PCs.
SVS is a part of a larger software stack

# Disclaimer
The current state of code is not great. It barely works. I put it here mainly to educate people on how not to write script interpreters.

# Usage

## Compilation
You can compile the SVS interpreter by running:

     bash build.sh

SVS for command line is simple application, you shouldn't need anything more than some build-essentials package. (Tested on linux mint and debian)

## Usage
After succesfull compilation you will end up with *svs* binary in the bin folder. Now you can execute svs scripts.

    ./svs somescript.svs
 You can start with example scripts from the *examples* directory or you can look at language **documentation** and write your own.

## How to use SVS in your app

-> basic C API
	-> stm32f0
	-> stm32f4 basics setup
	-> PC

-> advanced C API
	-> custom wrappers
