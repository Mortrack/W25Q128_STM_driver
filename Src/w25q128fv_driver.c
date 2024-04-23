#include "w25q128fv_driver.h"
#include <string.h>	// Library from which "memset()" and "memcpy()" are located at.

#define W25Q128FV_SECTOR_SIZE_IN_PAGES                          (16)        /**< @brief Size in pages of a single Sector of a W25Q128FV Flash Memory Device. */
#define W25Q128FV_PAGE_SIZE_IN_BYTES                            (256)       /**< @brief Size in bytes of a single page of a W25Q128FV Flash Memory Device. */
#define W25Q128FV_TOTAL_PAGES                                   (65356)     /**< @brief Total number of pages in a W25Q128FV Flash Memory Device. */
#define W25Q128FV_TOTAL_SECTORS                                 (4085)      /**< @brief Total number of sectors in a W25Q128FV Flash Memory Device. */
#define W25Q128FV_TOTAL_SECTORS_MINUS_ONE                       (4084)      /**< @brief Total number of sectors in a W25Q128FV Flash Memory Device minus one. */
#define W25Q128FV_FLASH_MEMORY_TOTAL_SIZE_IN_BYTES              (16731136)  /**< @brief Total size in bytes that can be read/written in the W25Q128FV Flash Memory Device. */
#define W25Q128FV_SECTOR_SIZE_IN_BYTES                          (4096)      /**< @brief Total size in bytes of a Sector in the W25Q128FV Flash Memory Device. @details The value of this definition should equal that of @ref W25Q128FV_SECTOR_SIZE_IN_PAGES times @ref W25Q128FV_PAGE_SIZE_IN_BYTES . */
#define W25Q128FV_MAX_CONSECUTIVE_PROGRAMMABLE_BYTES            (255)       /**< @brief Total number of maximum consecutive programmable bytes that can be written at a single time (i.e., per request) in a W25Q128FV Flash Memory Device. */
#define W25Q128FV_TWO_MAX_CONSECUTIVE_PROGRAMMABLE_BYTES        (510)       /**< @brief Twice the total number of maximum consecutive programmable bytes that can be written at a single time (i.e., per request) in a W25Q128FV Flash Memory Device. */
#define W25Q128FV_ENABLE_RESET_INSTRUCTION                      (0x66)      /**< @brief Byte value that the W25Q128FV Flash Memory Device interprets as the Enable Reset Instruction. */
#define W25Q128FV_RESET_DEVICE_INSTRUCTION                      (0X99)      /**< @brief Byte value that the W25Q128FV Flash Memory Device interprets as the Reset Reset Instruction. */
#define W25Q128FV_READ_JEDEC_ID_INSTRUCTION                     (0x9F)      /**< @brief Byte value that the W25Q128FV Flash Memory Device interprets as the Read JEDEC ID Instruction. */
#define W25Q128FV_READ_DATA_INSTRUCTION                         (0x03)      /**< @brief Byte value that the W25Q128FV Flash Memory Device interprets as the Read Data Instruction. */
#define W25Q128FV_FAST_READ_INSTRUCTION                         (0x0B)      /**< @brief Byte value that the W25Q128FV Flash Memory Device interprets as the Fast Read Instruction. */
#define W25Q128FV_SECTOR_ERASE_INSTRUCTION                      (0x20)      /**< @brief Byte value that the W25Q128FV Flash Memory Device interprets as the Sector Erase Instruction. */
#define W25Q128FV_CHIP_ERASE_INSTRUCTION                        (0xC7)      /**< @brief Byte value that the W25Q128FV Flash Memory Device interprets as the Chip Erase Instruction. */
#define W25Q128FV_PAGE_PROGRAM_INSTRUCTION                      (0x02)      /**< @brief Byte value that the W25Q128FV Flash Memory Device interprets as the Page Program Instruction. */
#define W25Q128FV_PAGE_PROGRAM_INSTRUCTION_MAX_SIZE_IN_BYTES    (259)       /**< @brief Maximum number of bytes that can be contained in a single Page Program Instruction in a W25Q128FV Flash Memory Device. */
#define W25Q128FV_WRITE_ENABLE_INSTRUCTION                      (0x06)      /**< @brief Byte value that the W25Q128FV Flash Memory Device interprets as the Write Enable Instruction. */
#define W25Q128FV_WRITE_DISABLE_INSTRUCTION                     (0x04)      /**< @brief Byte value that the W25Q128FV Flash Memory Device interprets as the Write Disable Instruction. */

static SPI_HandleTypeDef *p_hspi;                               /**< @brief Pointer to the SPI Handle Structure of the SPI that will be used in this @ref w25q128fv to write/read data to/from the W25Q128FV Flash Memory Module. @details This pointer's value is defined in the @ref init_w25q128fv_module function. */
static W25Q128FV_peripherals_def_t *p_w25q128fv_peripherals;    /**< @brief Pointer to the W25Q128FV Device's Peripherals Definition Structure that will be used in this @ref w25q128fv to control the Peripherals towards which the terminals of the W25Q128FV device are connected to. @details This pointer's value is defined in the @ref init_w25q128fv_module function. */

/**@brief   Sends the Write Enable Instruction to the W25Q128FV Flash Memory Device.
 *
 * @note    This Instruction will enable Page Program, Quad Page Program, Sector Erase, Block Erase, Chip Erase, Write
 *          Status Register and Erase/Program Security Registers instructions in the W25Q128FV Device.
 *
 * @retval	W25Q128FV_EC_OK     if the Write Enable request was successfully sent to the W25Q128FV Flash Memory Device.
 * @retval  W25Q128FV_EC_NR     if there was no response from the W25Q128FV Flash Memory Device.
 * @retval  W25Q128FV_EC_ERR    if anything else went wrong.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date	April 10, 2024.
 */
static W25Q128FV_Status send_w25q128fv_write_enable_instruction(void);

/**@brief   Sends the Write Disable Instruction to the W25Q128FV Flash Memory Device.
 *
 * @note    This Instruction will disable Page Program, Quad Page Program, Sector Erase, Block Erase, Chip Erase, Write
 *          Status Register and Erase/Program Security Registers instructions in the W25Q128FV Device.
 *
 * @retval	W25Q128FV_EC_OK     if the Write Disable request was successfully sent to the W25Q128FV Flash Memory Device.
 * @retval  W25Q128FV_EC_NR     if there was no response from the W25Q128FV Flash Memory Device.
 * @retval  W25Q128FV_EC_ERR    if anything else went wrong.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date	April 10, 2024.
 */
static W25Q128FV_Status send_w25q128fv_write_disable_instruction(void);

/**@brief	Sets the State of the CS pin of the W25Q128FV Flash Memory Device to Reset (i.e., To Low State).
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    April 01, 2024.
 */
static void set_cs_pin_low(void);

/**@brief	Sets the State of the CS pin of the W25Q128FV Flash Memory Device to Set (i.e., To High State).
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    April 01, 2024.
 */
static void set_cs_pin_high(void);

/**@brief	Gets the corresponding @ref W25Q128FV_Status value depending on the given @ref HAL_StatusTypeDef value.
 *
 * @param HAL_status	HAL Status value (see @ref HAL_StatusTypeDef ) that wants to be converted into its equivalent
 * 						of a @ref W25Q128FV_Status value.
 *
 * @retval				W25Q128FV_EC_NR if \p HAL_status param equals \c HAL_BUSY or \c HAL_TIMEOUT .
 * @retval				W25Q128FV_EC_ERR if \p HAL_status param equals \c HAL_ERROR .
 * @retval				HAL_status param otherwise.
 *
 * @note	For more details on the returned values listed, see @ref HM10_Status and @ref HAL_StatusTypeDef .
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    April 01, 2024.
 */
static W25Q128FV_Status HAL_ret_handler(HAL_StatusTypeDef HAL_status);

void init_w25q128fv_module(SPI_HandleTypeDef *hspi, W25Q128FV_peripherals_def_t *peripherals)
{
    /* Persist the pointer to the specific SPI that is desired for the W25Q128FV Flash Memory module to use. */
    p_hspi = hspi;

    /* Persist the pointer to the W25Q128FV Device's Peripherals Definition Structure. */
    p_w25q128fv_peripherals = peripherals;
}

W25Q128FV_Status w25q128fv_software_reset(void)
{
    /** <b>Local variable ret:</b> @ref uint8_t Type variable used to hold the Return value of either a HAL function or a @ref W25Q128FV_Status function type. */
    uint8_t ret;
    /** <b>Local variable reset_instruction:</b> @ref uint8_t array variable of two bytes in size that is used to hold the data containing both the Enable Reset and Reset Device instructions that are to be sent to the W25Q128FV Device in order to request to it a Software Reset. */
    uint8_t reset_instruction[2] = {W25Q128FV_ENABLE_RESET_INSTRUCTION, W25Q128FV_RESET_DEVICE_INSTRUCTION};

    /* Send both the Enable Reset and the Reset Device Instructions to the W25Q128FV Flash Memory Device in order to request to it a Software Reset. */
    set_cs_pin_low();
    ret = HAL_SPI_Transmit(p_hspi, reset_instruction, 2, W25Q128FV_SPI_TIMEOUT);
    set_cs_pin_high();
    ret = HAL_ret_handler(ret);
    if (ret != W25Q128FV_EC_OK)
    {
        return ret;
    }

    HAL_Delay(1); // NOTE: The datasheet states that the W25Q128FV Flash Memory Device will take approximately 30us to reset and that no commands will be accepted during that time.
    return W25Q128FV_EC_OK;
}

W25Q128FV_Status w25q128fv_read_id(uint32_t *w25q128fv_id)
{
    /** <b>Local variable ret:</b> @ref uint8_t Type variable used to hold the Return value of either a HAL function or a @ref W25Q128FV_Status function type. */
	uint8_t ret;
    /** <b>Local variable read_jedec_id_instruction:</b> @ref uint8_t type variable that is used to hold the data containing the Read JEDEC ID instruction that is to be sent to the W25Q128FV Device in order to request to it a read of its JEDEC ID. */
    uint8_t read_jedec_id_instruction = W25Q128FV_READ_JEDEC_ID_INSTRUCTION;
    /** <b>Local variable w25q128fv_resp:</b> @ref uint8_t array variable of three bytes in size that will be used to hold the response of the W25Q128FV Device after sending to it a request to read its JEDEC ID. */
    uint8_t w25q128fv_resp[3];

    /* Request to read the JEDEC ID to the W25Q128FV Device. */
    set_cs_pin_low();
    ret = HAL_SPI_Transmit(p_hspi, &read_jedec_id_instruction, 1, W25Q128FV_SPI_TIMEOUT);
    ret = HAL_ret_handler(ret);
    if (ret != W25Q128FV_EC_OK)
    {
        set_cs_pin_high();
        return ret;
    }

    /* Receive the W25Q128FV Device JEDEC ID response. */
    ret = HAL_SPI_Receive(p_hspi, w25q128fv_resp, 3, W25Q128FV_SPI_TIMEOUT);
    set_cs_pin_high();
    ret = HAL_ret_handler(ret);
    if (ret != W25Q128FV_EC_OK)
    {
        return ret;
    }

    /* Validate the received JEDEC ID response. */
    if ((w25q128fv_resp[0]==0x00) && (w25q128fv_resp[1]==0x00) && (w25q128fv_resp[2]==0x00))
    {
        return W25Q128FV_EC_NR;
    }

    /* Formulate and store the 24-bit W25Q128FV Device ID. */
    *w25q128fv_id = (w25q128fv_resp[0]<<16)|(w25q128fv_resp[1]<<8)|(w25q128fv_resp[2]);

    return W25Q128FV_EC_OK;
}

W25Q128FV_Status w25q128fv_read_flash_memory(uint32_t start_page, uint8_t page_bytes_offset, uint32_t size, uint8_t *dst)
{
    /** <b>Local variable ret:</b> @ref uint8_t Type variable used to hold the Return value of either a HAL function or a @ref W25Q128FV_Status function type. */
    uint8_t ret;
    /** <b>Local variable w25q128fv_flash_memory_addr:</b> @ref uint32_t Type variable used to hold the W25Q128FV Device 24-bit Flash Memory Address from which it is desired to start reading data. */
    uint32_t w25q128fv_flash_memory_addr = start_page * W25Q128FV_PAGE_SIZE_IN_BYTES + page_bytes_offset;

    /* Validate that the Flash Memory Data to be read from the W25Q128FV Device actually exists in it. */
    if ((w25q128fv_flash_memory_addr+size) > W25Q128FV_FLASH_MEMORY_TOTAL_SIZE_IN_BYTES)
    {
        return W25Q128FV_EC_ERR;
    }

    /** <b>Local variable read_data_instruction:</b> @ref uint8_t array type variable that is used to hold the data containing the Read Data instruction that is to be sent to the W25Q128FV Device in order to request reading data from it. */
    uint8_t read_data_instruction[4];
    read_data_instruction[0] = W25Q128FV_READ_DATA_INSTRUCTION;
    read_data_instruction[1] = (w25q128fv_flash_memory_addr>>16);
    read_data_instruction[2] = (w25q128fv_flash_memory_addr>>8);
    read_data_instruction[3] = (w25q128fv_flash_memory_addr);

    /* Request to reading data from the W25Q128FV Device. */
    set_cs_pin_low();
    ret = HAL_SPI_Transmit(p_hspi, read_data_instruction, 4, W25Q128FV_SPI_TIMEOUT);
    ret = HAL_ret_handler(ret);
    if (ret != W25Q128FV_EC_OK)
    {
        set_cs_pin_high();
        return ret;
    }

    /* Receive the W25Q128FV Device Read Data Instruction response. */
    ret = HAL_SPI_Receive(p_hspi, dst, size, W25Q128FV_SPI_TIMEOUT);
    set_cs_pin_high();
    ret = HAL_ret_handler(ret);
    if (ret != W25Q128FV_EC_OK)
    {
        return ret;
    }

    return W25Q128FV_EC_OK;
}

W25Q128FV_Status w25q128fv_fast_read_flash_memory(uint32_t start_page, uint8_t page_bytes_offset, uint32_t size, uint8_t *dst)
{
    /** <b>Local variable ret:</b> @ref uint8_t Type variable used to hold the Return value of either a HAL function or a @ref W25Q128FV_Status function type. */
    uint8_t ret;
    /** <b>Local variable w25q128fv_flash_memory_addr:</b> @ref uint32_t Type variable used to hold the W25Q128FV Device 24-bit Flash Memory Address from which it is desired to start reading data. */
    uint32_t w25q128fv_flash_memory_addr = start_page * W25Q128FV_PAGE_SIZE_IN_BYTES + page_bytes_offset;

    /* Validate that the Flash Memory Data to be read from the W25Q128FV Device actually exists in it. */
    if ((w25q128fv_flash_memory_addr+size) > W25Q128FV_FLASH_MEMORY_TOTAL_SIZE_IN_BYTES)
    {
        return W25Q128FV_EC_ERR;
    }

    /** <b>Local variable fast_read_instruction:</b> @ref uint8_t array type variable that is used to hold the data containing the Fast Read instruction that is to be sent to the W25Q128FV Device in order to request reading data from it. */
    uint8_t fast_read_instruction[5];
    fast_read_instruction[0] = W25Q128FV_FAST_READ_INSTRUCTION;
    fast_read_instruction[1] = (w25q128fv_flash_memory_addr>>16);
    fast_read_instruction[2] = (w25q128fv_flash_memory_addr>>8);
    fast_read_instruction[3] = (w25q128fv_flash_memory_addr);
    fast_read_instruction[4] = 0x00; // NOTE: This data is interpreted as don't care by the W25Q128FV Device, since these 8 bits will give place during the 8 Dummy Clocks of the Fast Read Instruction.

    /* Request fast reading data from the W25Q128FV Device. */
    set_cs_pin_low();
    ret = HAL_SPI_Transmit(p_hspi, fast_read_instruction, 5, W25Q128FV_SPI_TIMEOUT);
    ret = HAL_ret_handler(ret);
    if (ret != W25Q128FV_EC_OK)
    {
        set_cs_pin_high();
        return ret;
    }

    /* Receive the W25Q128FV Device Read Data Instruction response. */
    ret = HAL_SPI_Receive(p_hspi, dst, size, W25Q128FV_SPI_TIMEOUT);
    set_cs_pin_high();
    ret = HAL_ret_handler(ret);
    if (ret != W25Q128FV_EC_OK)
    {
        return ret;
    }

    return W25Q128FV_EC_OK;
}

W25Q128FV_Status w25q128fv_erase_sector(uint32_t sector_number)
{
    /** <b>Local variable ret:</b> @ref uint8_t Type variable used to hold the Return value of either a HAL function or a @ref W25Q128FV_Status function type. */
    uint8_t ret;

    /* Validate that the Sector Number given via the \p sector_number param actually exists in the W25Q128FV Flash Memory Device. */
    if (sector_number > W25Q128FV_TOTAL_SECTORS_MINUS_ONE)
    {
        return W25Q128FV_EC_ERR;
    }

    /* Send the Write Enable Instruction to the W25Q128FV Flash Memory Device. */
    ret = send_w25q128fv_write_enable_instruction();
    if (ret != W25Q128FV_EC_OK)
    {
        return W25Q128FV_EC_ERR;
    }

    /* Formulate the Sector Erase Instruction. */
    /** <b>Local variable w25q128fv_flash_memory_addr:</b> @ref uint32_t Type variable used to hold the W25Q128FV Device 24-bit Flash Memory Address of the Sector whose data is to be erased. */
    uint32_t w25q128fv_flash_memory_addr = sector_number * W25Q128FV_SECTOR_SIZE_IN_BYTES;
    /** <b>Local variable sector_erase_instruction:</b> @ref uint8_t array type variable that is used to hold the data containing the Sector Erase instruction that is to be sent to the W25Q128FV Device in order to erase the desired Sector's data. */
    uint8_t sector_erase_instruction[4];
    sector_erase_instruction[0] = W25Q128FV_SECTOR_ERASE_INSTRUCTION;
    sector_erase_instruction[1] = (w25q128fv_flash_memory_addr>>16);
    sector_erase_instruction[2] = (w25q128fv_flash_memory_addr>>8);
    sector_erase_instruction[3] = (w25q128fv_flash_memory_addr);

    /* Request erasing the desired Sector of the W25Q128FV Device. */
    set_cs_pin_low();
    ret = HAL_SPI_Transmit(p_hspi, sector_erase_instruction, 4, W25Q128FV_SPI_TIMEOUT);
    set_cs_pin_high();
    ret = HAL_ret_handler(ret);
    if (ret != W25Q128FV_EC_OK)
    {
        return ret;
    }
    HAL_Delay(400); // The W25Q128FV datasheet states that a maximum of 400ms of time is required for a W25Q128FV Device in order for it to finish erasing a Sector.

    /* Send the Write Disable Instruction to the W25Q128FV Flash Memory Device. */
    ret = send_w25q128fv_write_disable_instruction();
    if (ret != W25Q128FV_EC_OK)
    {
        return W25Q128FV_EC_ERR;
    }

    return W25Q128FV_EC_OK;
}

W25Q128FV_Status w25q128fv_chip_erase(void)
{
    /** <b>Local variable ret:</b> @ref uint8_t Type variable used to hold the Return value of either a HAL function or a @ref W25Q128FV_Status function type. */
    uint8_t ret;
    /** <b>Local variable chip_erase_instruction:</b> @ref uint8_t variable that is used to hold the data containing the Chip Erase Instruction that is to be sent to the W25Q128FV Device in order to request to it to erase the entire data contained in the W25Q128FV Flash Memory Device. */
    uint8_t chip_erase_instruction = W25Q128FV_CHIP_ERASE_INSTRUCTION;

    /* Send the Write Enable Instruction to the W25Q128FV Flash Memory Device. */
    ret = send_w25q128fv_write_enable_instruction();
    if (ret != W25Q128FV_EC_OK)
    {
        return W25Q128FV_EC_ERR;
    }

    /* Send the Write Enable Instruction to the W25Q128FV Flash Memory Device. */
    set_cs_pin_low();
    ret = HAL_SPI_Transmit(p_hspi, &chip_erase_instruction, 1, W25Q128FV_SPI_TIMEOUT);
    set_cs_pin_high();
    ret = HAL_ret_handler(ret);
    if (ret != W25Q128FV_EC_OK)
    {
        return ret;
    }
    HAL_Delay(200000); // The W25Q128FV datasheet states that a maximum of 200s of time is required for a W25Q128FV Device in order for it to finish erasing a Sector.

    /* Send the Write Disable Instruction to the W25Q128FV Flash Memory Device. */
    ret = send_w25q128fv_write_disable_instruction();
    if (ret != W25Q128FV_EC_OK)
    {
        return W25Q128FV_EC_ERR;
    }

    return W25Q128FV_EC_OK;
}

W25Q128FV_Status w25q128fv_write_flash_memory(uint32_t start_page, uint8_t page_bytes_offset, uint32_t size, uint8_t *src)
{
    /** <b>Local variable ret:</b> @ref uint8_t Type variable used to hold the Return value of either a HAL function or a @ref W25Q128FV_Status function type. */
    uint8_t ret;
    /** <b>Local variable w25q128fv_flash_memory_addr_start:</b> @ref uint32_t Type variable used to hold the W25Q128FV Device 24-bit Flash Memory Address where it is desired to start writing data. */
    uint32_t w25q128fv_flash_memory_addr_start = start_page * W25Q128FV_PAGE_SIZE_IN_BYTES + page_bytes_offset;
    /** <b>Local variable w25q128fv_flash_memory_addr_end_plus_one:</b> @ref uint32_t Type variable used to hold the W25Q128FV Device 24-bit Flash Memory Address that is right after where it is expected for this function to write the last byte of the request data. */
    uint32_t w25q128fv_flash_memory_addr_end_plus_one = w25q128fv_flash_memory_addr_start + size;

    /* Validate that the Flash Memory Addresses where the Data to be written into the W25Q128FV Device actually exists in it. */
    if (w25q128fv_flash_memory_addr_end_plus_one > W25Q128FV_FLASH_MEMORY_TOTAL_SIZE_IN_BYTES)
    {
        return W25Q128FV_EC_ERR;
    }

    /** <b>Local variable w25q128fv_flash_memory_page_start:</b> @ref uint32_t Type variable used to hold the W25Q128FV Device Flash Memory Page where it is expected for that Device to start writing the desired data. */
    uint32_t w25q128fv_flash_memory_page_start = start_page + page_bytes_offset/W25Q128FV_PAGE_SIZE_IN_BYTES;
    /** <b>Local variable page_program_instruction:</b> @ref uint8_t array type variable that is used to hold the data containing the Page Program instruction that is to be sent to the W25Q128FV Device in order to request writing data into it. */
    uint8_t page_program_instruction[W25Q128FV_PAGE_PROGRAM_INSTRUCTION_MAX_SIZE_IN_BYTES];
    page_program_instruction[0] = W25Q128FV_PAGE_PROGRAM_INSTRUCTION;
    /** <b>Local Pointer page_program_instruction_data:</b> @ref uint8_t Pointer type variable that is used to point to the byte at which the @ref page_program_instruction Local Variable holds the actual data to be sent to the W25Q128FV Device in the next Page Program Instruction. */
    uint8_t *page_program_instruction_data = (uint8_t *) &page_program_instruction[4];
    /** <b>Local variable currently_written_bytes:</b> @ref uint32_t Type variable that will hold the value standing for the currently written bytes into the W25Q128FV Flash Memory Device. */
    uint32_t currently_written_bytes = 0;
    /** <b>Local variable current_page_program_instruction_size:</b> @ref uint16_t Type variable that holds the current size in bytes of the W25Q128FV Page Program Instruction that is currently being formulated. */
    uint16_t current_page_program_instruction_size = 4;
    /** <b>Local variable current_page_to_write:</b> @ref uint32_t Type variable that will hold the currently W25Q128FV Flash Memory Page at which this function is currently writing the desired data. */
    uint32_t current_page_to_write = w25q128fv_flash_memory_page_start;
    /** <b>Local variable next_page_to_write:</b> @ref uint32_t Type variable that will hold the next W25Q128FV Flash Memory Page at which this function is currently writing the desired data. */
    uint32_t next_page_to_write = w25q128fv_flash_memory_page_start + 1;
    /** <b>Local variable remaining_writable_bytes_in_current_page:</b> @ref uint8_t Type variable that will hold the currently remaining writable bytes in the current W25Q128FV Flash Memory Page at which this function is currently to write the desired data. */
    uint16_t remaining_writable_bytes_in_current_page = W25Q128FV_PAGE_SIZE_IN_BYTES - (w25q128fv_flash_memory_addr_start - w25q128fv_flash_memory_page_start * W25Q128FV_PAGE_SIZE_IN_BYTES);
    if (remaining_writable_bytes_in_current_page == 0)
    {
        remaining_writable_bytes_in_current_page = W25Q128FV_PAGE_SIZE_IN_BYTES;
    }

    /* Write the desired data into the W25Q128FV Flash Memory Device. */
    for (uint32_t current_w25q128fv_flash_memory_address=w25q128fv_flash_memory_addr_start; current_w25q128fv_flash_memory_address<w25q128fv_flash_memory_addr_end_plus_one;)
    {
        /* Send the Write Enable Instruction to the W25Q128FV Flash Memory Device. */
        ret = send_w25q128fv_write_enable_instruction();
        if (ret != W25Q128FV_EC_OK)
        {
            return W25Q128FV_EC_ERR;
        }

        /* Populate the Flash Memory Address field in the Page Program Instruction that is currently being formulated. */
        page_program_instruction[1] = (current_w25q128fv_flash_memory_address>>16);
        page_program_instruction[2] = (current_w25q128fv_flash_memory_address>>8);
        page_program_instruction[3] = (current_w25q128fv_flash_memory_address);
        set_cs_pin_low();

        /* Populate the Data field in the Page Program Instruction that is currently being formulated. */
        current_page_program_instruction_size = 4; // Reset the size counter of the current Page Program Instruction.
        if (remaining_writable_bytes_in_current_page == W25Q128FV_PAGE_SIZE_IN_BYTES)
        {
            page_program_instruction_data[0] = src[currently_written_bytes++];
            current_page_program_instruction_size++;
            current_w25q128fv_flash_memory_address++;
            remaining_writable_bytes_in_current_page--;
        }
        else
        {
            for (uint8_t current_byte=0; current_page_to_write<next_page_to_write; current_byte++)
            {
                page_program_instruction_data[current_byte] = src[currently_written_bytes++];
                current_page_program_instruction_size++;
                current_w25q128fv_flash_memory_address++;
                remaining_writable_bytes_in_current_page--;
                if (remaining_writable_bytes_in_current_page == 0)
                {
                    current_page_to_write++;
                    remaining_writable_bytes_in_current_page = W25Q128FV_PAGE_SIZE_IN_BYTES;
                }
                else if (currently_written_bytes == size)
                {
                    break;
                }
            }
            if (current_page_to_write == next_page_to_write)
            {
                next_page_to_write++;
            }
        }

        /* Sent the currently formulated Page Program Instruction. */
        ret = HAL_SPI_Transmit(p_hspi, page_program_instruction, current_page_program_instruction_size, W25Q128FV_SPI_TIMEOUT);
        set_cs_pin_high();
        ret = HAL_ret_handler(ret);
        if (ret != W25Q128FV_EC_OK)
        {
            return ret;
        }
        HAL_Delay(3); // The W25Q128FV datasheet states that a maximum of 3ms of time is required for a W25Q128FV Device in order for it to finish writing data into one of its Flash Memory Pages.

        /* Send the Write Disable Instruction to the W25Q128FV Flash Memory Device. */
        ret = send_w25q128fv_write_disable_instruction();
        if (ret != W25Q128FV_EC_OK)
        {
            return W25Q128FV_EC_ERR;
        }
    }

    return W25Q128FV_EC_OK;
}

static W25Q128FV_Status send_w25q128fv_write_enable_instruction(void)
{
    /** <b>Local variable ret:</b> @ref uint8_t Type variable used to hold the Return value of either a HAL function or a @ref W25Q128FV_Status function type. */
    uint8_t ret;
    /** <b>Local variable write_enable_instruction:</b> @ref uint8_t variable that is used to hold the data containing the Write Enable Instructions that is to be sent to the W25Q128FV Device in order to request to it to enable Page Program, Quad Page Program, Sector Erase, Block Erase, Chip Erase, Write Status Register and Erase/Program Security Registers instructions. */
    uint8_t write_enable_instruction = W25Q128FV_WRITE_ENABLE_INSTRUCTION;

    /* Send the Write Enable Instruction to the W25Q128FV Flash Memory Device. */
    set_cs_pin_low();
    ret = HAL_SPI_Transmit(p_hspi, &write_enable_instruction, 1, W25Q128FV_SPI_TIMEOUT);
    set_cs_pin_high();
    ret = HAL_ret_handler(ret);
    if (ret != W25Q128FV_EC_OK)
    {
        return ret;
    }

    return W25Q128FV_EC_OK;
}

static W25Q128FV_Status send_w25q128fv_write_disable_instruction(void)
{
    /** <b>Local variable ret:</b> @ref uint8_t Type variable used to hold the Return value of either a HAL function or a @ref W25Q128FV_Status function type. */
    uint8_t ret;
    /** <b>Local variable write_disable_instruction:</b> @ref uint8_t variable that is used to hold the data containing the Write Disable Instructions that is to be sent to the W25Q128FV Device in order to disable Page Program, Quad Page Program, Sector Erase, Block Erase, Chip Erase, Write Status Register and Erase/Program Security Registers instructions. */
    uint8_t write_disable_instruction = W25Q128FV_WRITE_DISABLE_INSTRUCTION;

    /* Send the Write Disable Instruction to the W25Q128FV Flash Memory Device. */
    set_cs_pin_low();
    ret = HAL_SPI_Transmit(p_hspi, &write_disable_instruction, 1, W25Q128FV_SPI_TIMEOUT);
    set_cs_pin_high();
    ret = HAL_ret_handler(ret);
    if (ret != W25Q128FV_EC_OK)
    {
        return ret;
    }

    return W25Q128FV_EC_OK;
}

static void set_cs_pin_low(void)
{
    HAL_GPIO_WritePin(p_w25q128fv_peripherals->CS.GPIO_Port, p_w25q128fv_peripherals->CS.GPIO_Pin, GPIO_PIN_RESET);
}

static void set_cs_pin_high(void)
{
    HAL_GPIO_WritePin(p_w25q128fv_peripherals->CS.GPIO_Port, p_w25q128fv_peripherals->CS.GPIO_Pin, GPIO_PIN_SET);
}

static W25Q128FV_Status HAL_ret_handler(HAL_StatusTypeDef HAL_status)
{
    switch (HAL_status)
    {
        case HAL_BUSY:
        case HAL_TIMEOUT:
            return W25Q128FV_EC_NR;
        case HAL_ERROR:
            return W25Q128FV_EC_ERR;
        default:
            return HAL_status;
    }
}
