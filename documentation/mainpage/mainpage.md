This documentation generated with Doxygen contains all the information required to be able to understand the variables,
definitions and functions provided to use this driver library in order to be able to implement the W25Q128FV Flash
Memory Device in a desired application. This library was written in C programming language and it is meant to be used
in STMicroelectronics microcontrollers and microprocessors together with the libraries provided by that manufacturer.

Feel free to review the other tabs of this offline documentation in order to learn more about both the features of what
this library has to offer and also any considerations that you might need to have in mind before considering to acquire
and implement a W25Q128FV Flash Memory Device into your project.

Also, in the case that you are interested in contributing to this project, know that all the functions, variables and
definitions of the source file (i.e., the .c file) of this library are also documented with Doxygen syntax/format in
the code files (i.e., in both the Header and Source files), but where here only the documentation from the Header files
shown since it is what you will need to know about in case you are only interested in using this driver library and not
to contribute in developing more features to it.

On the other hand, for reaching out a code example for implementing this library, review the detailed description of the
W25Q128FV Flash Memory Driver module at the provided offline documentation.

Last but not least, the configurations and details of the microcontroller used for testing and validating this library
can be reviewed at the
<a href=https://github.com/Mortrack/W25Q128_STM_driver/blob/main/documentation/pdfs/STM32CubeMX_configurations_report.pdf>STM32CubeMX_configurations_report.pdf</a>
, where it is important to highlight that the UART1 was used for the printf() function of the stdio.h library.

<i><b><u>NOTE:</u></b> To be able to link the printf() function to the UART1 as it was done during the testings and
validations made for this library, make sure to copy-paste/implement the following code into your main.c file:</i>

```c
/*@brief	Compiler definition to be able to use the @ref printf function from stdio.h library in order to print
 *          characters via the UART1 Peripheral but by using that @ref printf function.
 *
 * @return	The \p ch param.
 *
 * author	César Miranda Meza
 * @date	July 07, 2023
 */
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf set to 'Yes') calls __io_putchar(). */
int __io_putchar(int ch)
#else
int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
{
/* Place your implementation of fputc here. */
/* NOTE: The characters written into the UART1 Protocol will be looped until the end of transmission. */
HAL_UART_Transmit(&huart1, (uint8_t *) &ch, 1, HAL_MAX_DELAY);
return ch;
}
```

