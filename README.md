# W25Q128FV Flash Memory C STMicroelectronics driver library

Version: X.X.X.X

This library provides the definitions, variables and functions necessary so that its implementer can use and communicate
with a W25Q128FV Flash Memory Device from a microcontroller/microprocessor of the STMicroelectronics device 
family and, in particular, via the STM32CubeIDE app. However, know that this driver library contains only the necessary
functions required to read its JEDEC ID and to write, read and erase data into the W25Q128FV Flash Memory, but in
Standard SPI only (for now). For more details about this and to learn how to use this library, feel free to read the
<a href=#>documentation of this project</a>.

# How to explore the project files.
The following will describe the general purpose of the folders that are located in the current directory address:

- **/'Inc'**:
    - This folder contains the <a href=#>header code file for this library</a>.
- **/'Src'**:
    - This folder contains the <a href=#>source code file for this library</a>.
- **/documentation**:
    - This folder provides the documentation to learn all the details of this library and to know how to use it. 

## Future additions planned for this library

Although I cannot commit to do the following soon, at some point if this project is useful for other people, then I plan
to continue adding more functions so that this can be a complete library with all the actual features of a W25Q128FV
Flash Memory Device.
