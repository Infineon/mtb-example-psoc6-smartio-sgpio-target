/*****************************************************************************
* File Name  : sgpio_target.h
*
* Description: This file contains definitions of constants and structures for
*              the SGPIO Target implementation using the SPI and Smart I/O.
*
*******************************************************************************
* Copyright (2020), Cypress Semiconductor Corporation.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software") is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and 
* foreign), United States copyright laws and international treaty provisions. 
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the 
* Cypress source code and derivative works for the sole purpose of creating 
* custom software in support of licensee product, such licensee product to be
* used only in conjunction with Cypress's integrated circuit as specified in the
* applicable agreement. Any reproduction, modification, translation, compilation,
* or representation of this Software except as specified above is prohibited 
* without the express written permission of Cypress.
* 
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the Right to make changes to the Software without notice. 
* Cypress does not assume any liability arising out of the application or use
* of Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use as critical components in any products 
* where a malfunction or failure may reasonably be expected to result in 
* significant injury or death ("ACTIVE Risk Product"). By including Cypress's 
* product in a ACTIVE Risk Product, the manufacturer of such system or application
* assumes all risk of such use and in doing so indemnifies Cypress against all
* liability. Use of this Software may be limited by and subject to the applicable
* Cypress software license agreement.
*******************************************************************************/
#ifndef SGPIO_TARGET_H_
#define SGPIO_TARGET_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "cy_pdl.h"

/*******************************************************************************
*                              Enumerated Types
*******************************************************************************/
typedef enum
{
    /** Operation completed successfully */
    SGPIO_TARGET_SUCCESS   = 0u,

    /** One ore more input parameters are invalid */
    SGPIO_TARGET_BAD_PARAM = 1u,

} en_sgpio_target_status_t;

/*******************************************************************************
*                                 API Constants
*******************************************************************************/
/** Maximum number of bytes in a frame */
#define SGPIO_TARGET_MAX_FRAME_SIZE            8u

/** Number of bits per drive */
#define SGPIO_TARGET_BITS_PER_DRIVE            3u

/*******************************************************************************
*                              Type Definitions
*******************************************************************************/
typedef void (* cb_sgpio_target_callback_t)(void);

/** Config Structure */
typedef struct
{
    uint32_t num_drives;
} stc_sgpio_target_config_t;

/** Context Structure */
typedef struct
{
    // Constants
    uint32_t spi_width;
    uint32_t bit_frame_size;
    uint32_t byte_frame_size;
    CySCB_Type *spi_base;
    SMARTIO_PRT_Type *smartio_base;
    cb_sgpio_target_callback_t callback;

    // Variables
    volatile uint32_t builder_count;
    volatile uint8_t sdin_data[SGPIO_TARGET_MAX_FRAME_SIZE];
    volatile uint8_t sdout_data[SGPIO_TARGET_MAX_FRAME_SIZE];
    uint8_t  scratch_sdin[SGPIO_TARGET_MAX_FRAME_SIZE];
    uint8_t  scratch_sdout[SGPIO_TARGET_MAX_FRAME_SIZE];
    volatile bool has_data;
} stc_sgpio_target_context_t;


/*******************************************************************************
*                            Function Prototypes
*******************************************************************************/
en_sgpio_target_status_t SGPIO_Target_Init(CySCB_Type *spi_base,
                                           SMARTIO_PRT_Type *smartio_base,
                                           stc_sgpio_target_config_t const *config,
                                           stc_sgpio_target_context_t *context);
void SGPIO_Target_DeInit(stc_sgpio_target_context_t *context);
void SGPIO_Target_Enable(stc_sgpio_target_context_t *context);
void SGPIO_Target_Disable(stc_sgpio_target_context_t *context);
void SGPIO_Target_RegisterCallback(cb_sgpio_target_callback_t callback,
                                   stc_sgpio_target_context_t *context);
bool SGPIO_Target_HasData(stc_sgpio_target_context_t *context);
void SGPIO_Target_Clear(stc_sgpio_target_context_t *context);
en_sgpio_target_status_t SGPIO_Target_SetNumDrives(stc_sgpio_target_context_t *context);
void SGPIO_Target_Read(uint8_t *sdout_data, stc_sgpio_target_context_t *context);
void SGPIO_Target_Write(uint8_t *sdin_data, stc_sgpio_target_context_t *context);
void SGPIO_Target_Interrupt(stc_sgpio_target_context_t *context);

#endif /* SGPIO_TARGET_H_ */
