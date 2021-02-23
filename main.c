/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the Smart I/O SGPIO Target PSoC6 
*              Application for ModusToolbox.
*
* Related Document: See Readme.md
*
*******************************************************************************
* (c) 2020, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cycfg.h"
#include "cy_retarget_io.h"

#include "sgpio_target.h"

#if !defined (TARGET_CY8CKIT_062_BLE) && !defined (TARGET_CY8CPROTO_063_BLE) && \
    !defined (TARGET_CY8CKIT_062_WIFI_BT) && !defined (TARGET_CY8CKIT_062S2_43012) && \
    !defined (TARGET_CY8CPROTO_062_4343W) && !defined (TARGET_CY8CKIT_064B0S2_4343W) && \
    !defined (TARGET_CYSBSYSKIT_01) && !defined (TARGET_CYSBSYSKIT_DEV_01)
    #error Unsupported kit. Choose another kit.
#endif


/*******************************************************************************
* Macros
********************************************************************************/
#define GET_BIT(value, bit)             ((value >> bit) & 0x1)
#define ALL_DRIVES_ACTIVE               0x492
#define ALL_DRIVES_ERROR                0x924

/*******************************************************************************
* Function Prototypes
********************************************************************************/
static void sgpio_interrupt_handler(void);
static void button_callback(void *arg, cyhal_gpio_event_t event);

/*******************************************************************************
* Global Variables
********************************************************************************/
static stc_sgpio_target_context_t sgpio_target_context;
static cy_stc_scb_spi_context_t   sgpio_initiator_context;

/* SGPIO Configuration */
stc_sgpio_target_config_t sgpio_config = {
     .num_drives = 4
};

/* SGPIO Interrupt */
cy_stc_sysint_t sgpio_target_intr =
{
    .intrSrc = SGPIO_TARGET_SPI_IRQ,
    .intrPriority = 7UL
};

volatile bool button_pressed = false;

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* The main function for Cortex-M4 CPU does the following:
*  Initialization:
*  - Initializes all the hardware blocks
*  Do forever loop:
*  - Check if SGPIO Target data is available. 
*  - Check if button is pressed, if yes, print SGPIO data
*  - Check if SGPIO Initiator FIFO, if not full, write data to the FIFO
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/

int main(void)
{
    cy_rslt_t result;
    uint16_t frame_word;
    uint8_t frame[SGPIO_TARGET_MAX_FRAME_SIZE] = {0x0A, 0xBC};

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize retarget-io to use the debug UART port */
    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);

    /* Initialize the User Button */
    cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);
    cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_RISE, CYHAL_ISR_PRIORITY_DEFAULT, true);
    cyhal_gpio_register_callback(CYBSP_USER_BTN, button_callback, NULL);

    __enable_irq();

    /* Setup SPI Interrupt */
    Cy_SysInt_Init(&sgpio_target_intr, sgpio_interrupt_handler);
    NVIC_EnableIRQ(sgpio_target_intr.intrSrc);

    /* Initialize the SPI acting as SGPIO Initiator */
    Cy_SCB_SPI_Init(SGPIO_INITIATOR_SPI_HW, &SGPIO_INITIATOR_SPI_config, &sgpio_initiator_context);
    Cy_SCB_SPI_Enable(SGPIO_INITIATOR_SPI_HW);

    /* Initialize the SGPIO */
    SGPIO_Target_Init(SGPIO_TARGET_SPI_HW, SMARTIO_HW, &sgpio_config, &sgpio_target_context);
    SGPIO_Target_Enable(&sgpio_target_context);

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("****************** \
    Smart I/O SGPIO Target \
    ****************** \r\n\n");

    /* Set SGPIO output data */
    SGPIO_Target_Write(frame, &sgpio_target_context);

    /* Clear frame */
    memset(frame,0,SGPIO_TARGET_MAX_FRAME_SIZE);

    for (;;)
    {
        if (SGPIO_Target_HasData(&sgpio_target_context))
        {
            SGPIO_Target_Read(frame, &sgpio_target_context);

            /* Convert the frame to a 16-bit word */
            frame_word = (frame[0] << 8) | frame[1];

            /* Prints frame info only when button is pressed */
            if (button_pressed == true)
            {
                printf("| Drive No | Active | Locate | Error |\n\r");
                printf("|----------|--------|--------|-------|\n\r");
                printf("| Drive 0  |    %d   |    %d   |   %d   |\n\r", GET_BIT(frame_word, 0),
                                                                        GET_BIT(frame_word, 1),
                                                                        GET_BIT(frame_word, 2));
                printf("| Drive 1  |    %d   |    %d   |   %d   |\n\r", GET_BIT(frame_word, 3),
                                                                        GET_BIT(frame_word, 4),
                                                                        GET_BIT(frame_word, 5));
                printf("| Drive 2  |    %d   |    %d   |   %d   |\n\r", GET_BIT(frame_word, 6),
                                                                        GET_BIT(frame_word, 7),
                                                                        GET_BIT(frame_word, 8));
                printf("| Drive 3  |    %d   |    %d   |   %d   |\n\r", GET_BIT(frame_word, 9),
                                                                        GET_BIT(frame_word, 10),
                                                                        GET_BIT(frame_word, 11));
                printf("--------------------------------------\n\r");

                button_pressed = false;
            }
        }

        /* Check if the SPI FIFO is not FULL, to continuously write data to the SGPIO bus */
        if (Cy_SCB_SPI_GetTxFifoStatus(SGPIO_INITIATOR_SPI_HW) & CY_SCB_SPI_TX_NOT_FULL)
        {
            /* Write a frame where all drives are active */
            Cy_SCB_SPI_Write(SGPIO_INITIATOR_SPI_HW, ALL_DRIVES_ACTIVE);
        }
    }
}

/*******************************************************************************
* Function Name: sgpio_interrupt_handler
********************************************************************************
* Summary:
*  SGPIO ISR handler. Handle the SGPIO frame.
*
*******************************************************************************/
static void sgpio_interrupt_handler(void)
{
    SGPIO_Target_Interrupt(&sgpio_target_context);
}

/*******************************************************************************
* Function Name: button_callback
********************************************************************************
* Summary:
*  Button callback. Set a flag to be processed in the main loop.
*
* Parameters:
*  arg: not used
*  event: event that occurred
*
*******************************************************************************/
static void button_callback(void *arg, cyhal_gpio_event_t event)
{
    (void) arg;

    if (event & CYHAL_GPIO_IRQ_RISE)
    {
        button_pressed = true;
    }
}

/* [] END OF FILE */
