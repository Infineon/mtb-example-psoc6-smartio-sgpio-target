/*****************************************************************************
* File Name  : sgpio_target.c
*
* Description: This file contains function definitions for implementing the
*              SGPIO Target interface.
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
#include "sgpio_target.h"

/*******************************************************************************
*                                 Global Variables
*******************************************************************************/
/** SPI Configuration */
static cy_stc_scb_spi_config_t sgpio_spi_config =
{
    .spiMode = CY_SCB_SPI_SLAVE,
    .subMode = CY_SCB_SPI_MOTOROLA,
    .sclkMode = CY_SCB_SPI_CPHA1_CPOL0,
    .oversample = 0UL,
    .rxDataWidth = 12UL,
    .txDataWidth = 12UL,
    .enableMsbFirst = false,
    .enableInputFilter = false,
    .enableFreeRunSclk = false,
    .enableMisoLateSample = false,
    .enableTransferSeperation = false,
    .ssPolarity = ((CY_SCB_SPI_ACTIVE_HIGH << CY_SCB_SPI_SLAVE_SELECT0) | \
                  (CY_SCB_SPI_ACTIVE_LOW << CY_SCB_SPI_SLAVE_SELECT1) | \
                  (CY_SCB_SPI_ACTIVE_LOW << CY_SCB_SPI_SLAVE_SELECT2) | \
                  (CY_SCB_SPI_ACTIVE_LOW << CY_SCB_SPI_SLAVE_SELECT3)),
    .enableWakeFromSleep = false,
    .rxFifoTriggerLevel = 63UL,
    .rxFifoIntEnableMask = 4UL,
    .txFifoTriggerLevel = 63UL,
    .txFifoIntEnableMask = 0UL,
    .masterSlaveIntEnableMask = 0UL,
};

/** Smart I/O Configuration */
static const cy_stc_smartio_lutcfg_t sgpio_smartio_lutCfg0 =
{
    .tr0 = CY_SMARTIO_LUTTR_IO0,
    .tr1 = CY_SMARTIO_LUTTR_IO0,
    .tr2 = CY_SMARTIO_LUTTR_IO0,
    .opcode = CY_SMARTIO_LUTOPC_COMB,
    .lutMap = 128,
};
static const cy_stc_smartio_lutcfg_t sgpio_smartio_lutCfg1 =
{
    .tr0 = CY_SMARTIO_LUTTR_CHIP1,
    .tr1 = CY_SMARTIO_LUTTR_CHIP1,
    .tr2 = CY_SMARTIO_LUTTR_CHIP1,
    .opcode = CY_SMARTIO_LUTOPC_GATED_OUT,
    .lutMap = 128,
};
static const cy_stc_smartio_lutcfg_t sgpio_smartio_lutCfg2 =
{
    .tr0 = CY_SMARTIO_LUTTR_IO2,
    .tr1 = CY_SMARTIO_LUTTR_IO2,
    .tr2 = CY_SMARTIO_LUTTR_IO2,
    .opcode = CY_SMARTIO_LUTOPC_COMB,
    .lutMap = 254,
};
static const cy_stc_smartio_lutcfg_t sgpio_smartio_lutCfg3 =
{
    .tr0 = CY_SMARTIO_LUTTR_IO3,
    .tr1 = CY_SMARTIO_LUTTR_LUT6_OUT,
    .tr2 = CY_SMARTIO_LUTTR_LUT5_OUT,
    .opcode = CY_SMARTIO_LUTOPC_GATED_OUT,
    .lutMap = 236,
};
static const cy_stc_smartio_lutcfg_t sgpio_smartio_lutCfg4 =
{
    .tr0 = CY_SMARTIO_LUTTR_LUT2_OUT,
    .tr1 = CY_SMARTIO_LUTTR_LUT2_OUT,
    .tr2 = CY_SMARTIO_LUTTR_LUT2_OUT,
    .opcode = CY_SMARTIO_LUTOPC_GATED_OUT,
    .lutMap = 128,
};
static const cy_stc_smartio_lutcfg_t sgpio_smartio_lutCfg5 =
{
    .tr0 = CY_SMARTIO_LUTTR_LUT2_OUT,
    .tr1 = CY_SMARTIO_LUTTR_LUT2_OUT,
    .tr2 = CY_SMARTIO_LUTTR_LUT4_OUT,
    .opcode = CY_SMARTIO_LUTOPC_GATED_OUT,
    .lutMap = 16,
};
static const cy_stc_smartio_lutcfg_t sgpio_smartio_lutCfg6 =
{
    .tr0 = CY_SMARTIO_LUTTR_DU_OUT,
    .tr1 = CY_SMARTIO_LUTTR_LUT3_OUT,
    .tr2 = CY_SMARTIO_LUTTR_LUT5_OUT,
    .opcode = CY_SMARTIO_LUTOPC_COMB,
    .lutMap = 68,
};
static const cy_stc_smartio_ducfg_t sgpio_smartio_duCfg =
{
    .tr0 = CY_SMARTIO_DUTR_LUT5_OUT,
    .tr1 = CY_SMARTIO_DUTR_ONE,
    .tr2 = CY_SMARTIO_DUTR_ZERO,
    .data0 = CY_SMARTIO_DUDATA_ZERO,
    .data1 = CY_SMARTIO_DUDATA_DATAREG,
    .opcode = CY_SMARTIO_DUOPC_INCR_WRAP,
    .size = CY_SMARTIO_DUSIZE_8,
    .dataReg = 255,
};
static const cy_stc_smartio_config_t sgpio_smartio_config =
{
    .clkSrc = CY_SMARTIO_CLK_DIVACT,
    .bypassMask = 0u|0u|0u|0u|CY_SMARTIO_CHANNEL4|CY_SMARTIO_CHANNEL5|0u|0u,
    .ioSyncEn = 0u|0u|CY_SMARTIO_CHANNEL2|CY_SMARTIO_CHANNEL3|0u|0u|0u|0u,
    .chipSyncEn = 0u|CY_SMARTIO_CHANNEL1|0u|0u|0u|0u|0u|0u,
    .lutCfg0 = &sgpio_smartio_lutCfg0,
    .lutCfg1 = &sgpio_smartio_lutCfg1,
    .lutCfg2 = &sgpio_smartio_lutCfg2,
    .lutCfg3 = &sgpio_smartio_lutCfg3,
    .lutCfg4 = &sgpio_smartio_lutCfg4,
    .lutCfg5 = &sgpio_smartio_lutCfg5,
    .lutCfg6 = &sgpio_smartio_lutCfg6,
    .lutCfg7 = NULL,
    .duCfg = &sgpio_smartio_duCfg,
    .hldOvr = false,
};

/*******************************************************************************
* Function Name: SGPIO_Target_Init
****************************************************************************//**
*
* Initialize the SGPIO block.
*
* \param spi_base
* The pointer to the SPI.
*
* \param smartio_base
* The pointer to the Smart I/O.
*
* \param config
* The pointer to the configuration structure
*
* \param context
* The pointer to the context structure \ref stc_sgpio_target_context_t allocated
* by the user. The structure is used during the SGPIO operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
*******************************************************************************/
en_sgpio_target_status_t SGPIO_Target_Init(CySCB_Type *spi_base,
                                           SMARTIO_PRT_Type *smartio_base,
                                           stc_sgpio_target_config_t const *config,
                                           stc_sgpio_target_context_t *context)
{
    if (context == NULL || spi_base == NULL || smartio_base == NULL)
    {
        return SGPIO_TARGET_BAD_PARAM;
    }

    /* Set the frame sizes */
    context->bit_frame_size = config->num_drives*SGPIO_TARGET_BITS_PER_DRIVE;
    context->byte_frame_size = context->bit_frame_size / 8;
    /* Check if multiple of 8. If not, add an extra byte to the frame */
    if ((context->bit_frame_size % 8) != 0)
    {
        context->byte_frame_size++;
    }

    /* Check for the maximum frame size */
    if (context->byte_frame_size > SGPIO_TARGET_MAX_FRAME_SIZE)
    {
        return SGPIO_TARGET_BAD_PARAM;
    }

    /* Set SPI width */
    if (context->bit_frame_size <= 16)
    {
        context->spi_width = context->bit_frame_size;
    }
    else if (context->bit_frame_size <= 32)
    {
        context->spi_width = context->bit_frame_size/2;
    }
    else if (context->bit_frame_size <= 48)
    {
        context->spi_width = context->bit_frame_size/3;
    }
    else
    {
        context->spi_width = context->bit_frame_size/4;
    }

    /* Update SPI width in the config */
    sgpio_spi_config.rxDataWidth = context->spi_width;
    sgpio_spi_config.txDataWidth = context->spi_width;

    /* Set pointer to the hardware blocks */
    context->spi_base = spi_base;
    context->smartio_base = smartio_base;

    /* Init the Smart I/O block */
    Cy_SmartIO_Init(smartio_base, &sgpio_smartio_config);

    /* Init the SPI block */
    Cy_SCB_SPI_Init(context->spi_base, &sgpio_spi_config, NULL);

    /* Clear remaining variables */
    context->callback = NULL;
    context->builder_count = 0;
    for (uint32_t i = 0; i < SGPIO_TARGET_MAX_FRAME_SIZE; i++)
    {
        context->sdout_data[i] = 0;
        context->sdin_data[i] = 0;
        context->scratch_sdout[i] = 0;
        context->scratch_sdin[i] = 0;
    }
    context->has_data = false;

    return SGPIO_TARGET_SUCCESS;
}

/*******************************************************************************
* Function Name: SGPIO_Target_DeInit
****************************************************************************//**
*
* De-initialize the SGPIO block.
*
* \param context
* The pointer to the context structure \ref stc_sgpio_target_context_t allocated
* by the user. The structure is used during the SGPIO operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
*******************************************************************************/
void SGPIO_Target_DeInit(stc_sgpio_target_context_t *context)
{
    context->callback = NULL;
    context->builder_count = 0;

    for (uint32_t i = 0; i < SGPIO_TARGET_MAX_FRAME_SIZE; i++)
    {
        context->sdout_data[i] = 0;
        context->sdin_data[i] = 0;
        context->scratch_sdout[i] = 0;
        context->scratch_sdin[i] = 0;
    }
}

/*******************************************************************************
* Function Name: SGPIO_Target_Enable
****************************************************************************//**
*
* Enable the SGPIO block. An interrupt is triggered on the background.
*
* \param context
* The pointer to the context structure \ref stc_sgpio_target_context_t allocated
* by the user. The structure is used during the SGPIO operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
*******************************************************************************/
void SGPIO_Target_Enable(stc_sgpio_target_context_t *context)
{
    context->builder_count = 0;

    /* Enable SPI and Smart I/0 */
    Cy_SmartIO_Enable(context->smartio_base);
    Cy_SCB_SPI_Enable(context->spi_base);
}

/*******************************************************************************
* Function Name: SGPIO_Target_Disable
****************************************************************************//**
*
* Disable the SGPIO block.
*
* \param context
* The pointer to the context structure \ref stc_sgpio_target_context_t allocated
* by the user. The structure is used during the SGPIO operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
*******************************************************************************/
void SGPIO_Target_Disable(stc_sgpio_target_context_t *context)
{
    /* Enable SPI and Smart I/0 */
    Cy_SCB_SPI_Disable(context->spi_base, NULL);
    Cy_SmartIO_Disable(context->smartio_base);
}

/*******************************************************************************
* Function Name: SGPIO_Target_RegisterCallback
****************************************************************************//**
*
* Register a callback to be executed when a frame is ready.
*
* \param callback
* Pointer to the callback function
*
* \param context
* The pointer to the context structure \ref stc_sgpio_target_context_t allocated
* by the user. The structure is used during the SGPIO operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
*******************************************************************************/
void SGPIO_Target_RegisterCallback(cb_sgpio_target_callback_t callback,
                                   stc_sgpio_target_context_t *context)
{
    context->callback = callback;
}

/*******************************************************************************
* Function Name: SGPIO_Target_HasData
****************************************************************************//**
*
* Check if any data is available. The flag is cleared on read.
*
* \param context
* The pointer to the context structure \ref stc_sgpio_target_context_t allocated
* by the user. The structure is used during the SGPIO operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
*******************************************************************************/
bool SGPIO_Target_HasData(stc_sgpio_target_context_t *context)
{
    bool ret = context->has_data;
    int32_t status;

    status = CyEnterCriticalSection();
    context->has_data = false;
    CyExitCriticalSection(status);

    return ret;
}

/*******************************************************************************
* Function Name: SGPIO_Target_Clear
****************************************************************************//**
*
* Clear internal FIFOs.
*
* \param context
* The pointer to the context structure \ref stc_sgpio_target_context_t allocated
* by the user. The structure is used during the SGPIO operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
*******************************************************************************/
void SGPIO_Target_Clear(stc_sgpio_target_context_t *context)
{
    Cy_SCB_SPI_ClearRxFifo(context->spi_base);
    Cy_SCB_SPI_ClearTxFifo(context->spi_base);
}

/*******************************************************************************
* Function Name: SGPIO_Target_SetNumDrives
****************************************************************************//**
*
* Not implemented yet.
*
* \param context
* The pointer to the context structure \ref stc_sgpio_target_context_t allocated
* by the user. The structure is used during the SGPIO operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
*******************************************************************************/
en_sgpio_target_status_t SGPIO_Target_SetNumDrives(stc_sgpio_target_context_t *context)
{
    /* TODO */
    return SGPIO_TARGET_SUCCESS;
}

/*******************************************************************************
* Function Name: SGPIO_Target_Read
****************************************************************************//**
*
* Read the last SDataOut bits available. The least significant bits from the bus
* are placed in the last element of the frame array. 
* Below is an example of a 12-bit frame (frame = {X, Y}).
* | SRAM  |Y.0 |Y.1 |Y.2 |Y.3 |Y.4 |Y.5 |Y.6 |Y.7 |X.0 |X.1 | X.2 | X.3 |
* | Bits  |Bit0|Bit1|Bit2|Bit3|Bit4|Bit5|Bit6|Bit7|Bit8|Bit9|Bit10|Bit11| 
* | SGPIO |D0.0|D0.1|D0.2|D1.0|D1.1|D1.2|D2.0|D2.1|D2.2|D3.0|D3.1 |D3.2 |
*
* \param frame
* Pointer to the data array to store the SDout data. The size depends on the 
* number of drives.
*
* \param context
* The pointer to the context structure \ref stc_sgpio_target_context_t allocated
* by the user. The structure is used during the SGPIO operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
*******************************************************************************/
void SGPIO_Target_Read(uint8_t *sdout_data, stc_sgpio_target_context_t *context)
{
    int32_t status;

    status = CyEnterCriticalSection();

    for (uint32_t i = 0; i < context->byte_frame_size; i++)
    {
        sdout_data[i] = context->sdout_data[i];
    }

    CyExitCriticalSection(status);
}

/*******************************************************************************
* Function Name: SGPIO_Target_Write
****************************************************************************//**
*
* Setup the SDataIn bits to be serialized. It will write constantly to the bus
* as long the block is enabled. The first elements from the frame array are 
* placed in the most significant bits on the bus. 
* Below is an example of a 12-bit frame (frame = {X, Y}).
* | SRAM  |Y.0 |Y.1 |Y.2 |Y.3 |Y.4 |Y.5 |Y.6 |Y.7 |X.0 |X.1 | X.2 | X.3 |
* | Bits  |Bit0|Bit1|Bit2|Bit3|Bit4|Bit5|Bit6|Bit7|Bit8|Bit9|Bit10|Bit11| 
* | SGPIO |D0.0|D0.1|D0.2|D1.0|D1.1|D1.2|D2.0|D2.1|D2.2|D3.0|D3.1 |D3.2 |
*
* \param frame
* Pointer to the data array with the frame. The size depends on the number of
* drives.
*
* \param context
* The pointer to the context structure \ref stc_sgpio_target_context_t allocated
* by the user. The structure is used during the SGPIO operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
*******************************************************************************/
void SGPIO_Target_Write(uint8_t *sdin_data, stc_sgpio_target_context_t *context)
{
    int32_t status;

    status = CyEnterCriticalSection();

    for (uint32_t i = 0; i < context->byte_frame_size; i++)
    {
        context->sdin_data[i] = sdin_data[i];
    }

    CyExitCriticalSection(status);
}

/*******************************************************************************
* Function Name: SGPIO_Target_Interrupt
****************************************************************************//**
*
* SGPIO Interrupt handler. It builds the SDataIn and SDataOut bits.
*
* \param context
* The pointer to the context structure \ref stc_sgpio_target_context_t allocated
* by the user. The structure is used during the SGPIO operation for internal
* configuration and data retention. The user must not modify anything in this
* structure.
*
*******************************************************************************/
void SGPIO_Target_Interrupt(stc_sgpio_target_context_t *context)
{
    uint16_t sdataout;
    volatile uint16_t sdatain;
    uint32_t status = Cy_SCB_GetRxInterruptStatus(context->spi_base);

    /* Process the SPI Interrupt */
    if (0UL != (CY_SCB_RX_INTR_NOT_EMPTY & status))
    {
        /* Read one word from the SPI RX FIFO */
        sdataout = Cy_SCB_SPI_Read(context->spi_base);

        /* Set the 8 MSB bits for data out*/
        sdatain = (context->scratch_sdin[context->builder_count]) << 8;

        /* Get the 8 MSB bits for data in */
        context->scratch_sdout[context->builder_count++] = CY_HI8(sdataout);

        /* Set/Get the 8 LSB bits, if any */
        if (context->spi_width > 8)
        {
            /* Set the 8 LSB bits for data out */
            sdatain |= context->scratch_sdin[context->builder_count];

            /* Get the 8 LSB bits for data in */
            context->scratch_sdout[context->builder_count++] = CY_LO8(sdataout);
        }

        /* Write to the SPI TX FIFO */
        Cy_SCB_SPI_Write(context->spi_base, sdatain);

        /* Check if read all bytes of a frame */
        if (context->builder_count >= context->byte_frame_size)
        {
            /* Change State to Ready */
            context->has_data = true;

            /* Copy data from scratch to the SDOUT data */
            if (context->byte_frame_size == 2)
            {
                memcpy((uint8_t *) context->sdout_data, context->scratch_sdout, context->byte_frame_size);
            }
            else if (context->byte_frame_size == 3)
            {
                context->sdout_data[0]  = (context->scratch_sdout[2] << (context->spi_width - 8));
                context->sdout_data[0] |= (context->scratch_sdout[3] >> (16 - context->spi_width));
                context->sdout_data[1]  = (context->scratch_sdout[3] << (context->spi_width - 8));
                context->sdout_data[1] |= (context->scratch_sdout[0]);
                context->sdout_data[2]  = (context->scratch_sdout[1]);
            }
            else if (context->byte_frame_size == 4)
            {
                /* TODO */
            }
            else if (context->byte_frame_size == 5)
            {
                context->sdout_data[0]  = (context->scratch_sdout[4]);
                context->sdout_data[1]  = (context->scratch_sdout[5]);
                context->sdout_data[2]  = (context->scratch_sdout[2] << (context->spi_width - 8));
                context->sdout_data[2] |= (context->scratch_sdout[3] >> (16 - context->spi_width));
                context->sdout_data[3]  = (context->scratch_sdout[3] << (context->spi_width - 8));
                context->sdout_data[3] |= (context->scratch_sdout[0]);
                context->sdout_data[4]  = (context->scratch_sdout[1]);
            }
            else if (context->byte_frame_size == 6)
            {
                context->sdout_data[0]  = (context->scratch_sdout[4]);
                context->sdout_data[1]  = (context->scratch_sdout[5]);
                context->sdout_data[2]  = (context->scratch_sdout[2]);
                context->sdout_data[3]  = (context->scratch_sdout[3]);
                context->sdout_data[4]  = (context->scratch_sdout[0]);
                context->sdout_data[5]  = (context->scratch_sdout[1]);
            }

            /* Clear build counter */
            context->builder_count = 0;

            /* Copy data from SDataIn to scratch */
            if (context->byte_frame_size == 2)
            {
                memcpy(context->scratch_sdin, (uint8_t *) context->sdin_data, context->byte_frame_size);
            }
            else if (context->byte_frame_size == 3)
            {
                context->scratch_sdin[0]  = (context->sdin_data[0] >> (context->spi_width - 8));
                context->scratch_sdin[1]  = (context->sdin_data[0] << (16 - context->spi_width));
                context->scratch_sdin[1] |= (context->sdin_data[1] >> (context->spi_width - 8));
                context->scratch_sdin[2]  = (context->sdin_data[1]);
                context->scratch_sdin[3]  = (context->sdin_data[2]);
            }
            else if (context->byte_frame_size == 4)
            {
                /* TODO */
            }
            else if (context->byte_frame_size == 5)
            {
                context->scratch_sdin[0]  = (context->sdin_data[1]);
                context->scratch_sdin[1]  = (context->sdin_data[2]);
                context->scratch_sdin[1] |= (context->sdin_data[1] >> (context->spi_width - 8));
                context->scratch_sdin[2]  = (context->sdin_data[0] >> (context->spi_width - 8));
                context->scratch_sdin[3]  = (context->sdin_data[0] << (16 - context->spi_width));
                context->scratch_sdin[3] |= (context->sdin_data[1] >> (context->spi_width - 8));
                context->scratch_sdin[4]  = (context->sdin_data[3] >> (context->spi_width - 8));
                context->scratch_sdin[5]  = (context->sdin_data[3] << (16 - context->spi_width));
                context->scratch_sdin[5] |= (context->sdin_data[4] >> (context->spi_width - 8));
            }
            else if (context->byte_frame_size == 6)
            {
                context->scratch_sdin[0]  = (context->sdin_data[2]);
                context->scratch_sdin[1]  = (context->sdin_data[3]);
                context->scratch_sdin[2]  = (context->sdin_data[0]);
                context->scratch_sdin[3]  = (context->sdin_data[1]);
                context->scratch_sdin[4]  = (context->sdin_data[4]);
                context->scratch_sdin[5]  = (context->sdin_data[5]);
            }

            if (context->callback != NULL)
            {
                context->callback();
            }
        }
    }

    Cy_SCB_ClearRxInterrupt(context->spi_base, status);
}
