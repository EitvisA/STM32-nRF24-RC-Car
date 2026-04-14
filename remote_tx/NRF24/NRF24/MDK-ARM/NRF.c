#include "NRF.h"

/*
  ***************************************************************************************************************
  ***************************************************************************************************************
  ***************************************************************************************************************

  File:		  NRF24L01.c
  Author:     ControllersTech.com
  Updated:    30th APRIL 2021

  ***************************************************************************************************************
  Copyright (C) 2017 ControllersTech.com

  This is a free software under the GNU license, you can redistribute it and/or modify it under the terms
  of the GNU General Public License version 3 as published by the Free Software Foundation.
  This software library is shared with public for educational purposes, without WARRANTY and Author is not liable for any damages caused directly
  or indirectly by this software, read more about this on the GNU General Public License.

  ***************************************************************************************************************
*/


#include "stm32f0xx_hal.h"
#include "NRF.h"
extern int a;
extern int TX_EMPTY;
extern int DataInFifo;
extern SPI_HandleTypeDef hspi1;
#define NRF24_SPI &hspi1

#define NRF24_CE_PORT   GPIOA
#define NRF24_CE_PIN    GPIO_PIN_3

#define NRF24_CSN_PORT   GPIOA
#define NRF24_CSN_PIN    GPIO_PIN_4


void CS_Select (void)
{
	HAL_GPIO_WritePin(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_PIN_RESET);
}

void CS_UnSelect (void)
{
	HAL_GPIO_WritePin(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_PIN_SET);
}


void CE_Enable (void)
{
	HAL_GPIO_WritePin(NRF24_CE_PORT, NRF24_CE_PIN, GPIO_PIN_SET);
}

void CE_Disable (void)
{
	HAL_GPIO_WritePin(NRF24_CE_PORT, NRF24_CE_PIN, GPIO_PIN_RESET);
}



// write a single byte to the particular register
void nrf24_WriteReg (uint8_t Reg, uint8_t Data)
{
	uint8_t buf[2];
	buf[0] = Reg|1<<5;
	buf[1] = Data;

	// Pull the CS Pin LOW to select the device
	CS_Select();

	HAL_SPI_Transmit(NRF24_SPI, buf, 2, 1000);

	// Pull the CS HIGH to release the device
	CS_UnSelect();
}

//write multiple bytes starting from a particular register
void nrf24_WriteRegMulti (uint8_t Reg, uint8_t *data, int size)
{
	uint8_t buf[2];
	buf[0] = Reg|1<<5;
//	buf[1] = Data;

	// Pull the CS Pin LOW to select the device
	CS_Select();

	HAL_SPI_Transmit(NRF24_SPI, buf, 1, 100);
	HAL_SPI_Transmit(NRF24_SPI, data, size, 1000);

	// Pull the CS HIGH to release the device
	CS_UnSelect();
}


uint8_t nrf24_ReadReg (uint8_t Reg)
{
	uint8_t data=0;

	// Pull the CS Pin LOW to select the device
	CS_Select();

	HAL_SPI_Transmit(NRF24_SPI, &Reg, 1, 100);
	HAL_SPI_Receive(NRF24_SPI, &data, 1, 100);

	// Pull the CS HIGH to release the device
	CS_UnSelect();

	return data;
}


/* Read multiple bytes from the register */
void nrf24_ReadReg_Multi (uint8_t Reg, uint8_t *data, int size)
{
	// Pull the CS Pin LOW to select the device
	CS_Select();

	HAL_SPI_Transmit(NRF24_SPI, &Reg, 1, 100);
	HAL_SPI_Receive(NRF24_SPI, data, size, 1000);

	// Pull the CS HIGH to release the device
	CS_UnSelect();
}


// send the command to the NRF
void nrfsendCmd (uint8_t cmd)
{
	// Pull the CS Pin LOW to select the device
	CS_Select();

	HAL_SPI_Transmit(NRF24_SPI, &cmd, 1, 100);

	// Pull the CS HIGH to release the device
	CS_UnSelect();
}

void nrf24_reset(uint8_t REG)
{
	if (REG == STATUS)
	{
		nrf24_WriteReg(STATUS, 0x00);
	}

	else if (REG == FIFO_STATUS)
	{
		nrf24_WriteReg(FIFO_STATUS, 0x11);
	}

	else {
	nrf24_WriteReg(CONFIG, 0x08);
	nrf24_WriteReg(EN_AA, 0x3F);
	nrf24_WriteReg(EN_RXADDR, 0x03);
	nrf24_WriteReg(SETUP_AW, 0x03);
	nrf24_WriteReg(SETUP_RETR, 0x03);
	nrf24_WriteReg(RF_CH, 0x02);
	nrf24_WriteReg(RF_SETUP, 0x0E);
	nrf24_WriteReg(STATUS, 0x00);
	nrf24_WriteReg(OBSERVE_TX, 0x00);
	nrf24_WriteReg(CD, 0x00);
	uint8_t rx_addr_p0_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
	nrf24_WriteRegMulti(RX_ADDR_P0, rx_addr_p0_def, 5);
	uint8_t rx_addr_p1_def[5] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2};
	nrf24_WriteRegMulti(RX_ADDR_P1, rx_addr_p1_def, 5);
	nrf24_WriteReg(RX_ADDR_P2, 0xC3);
	nrf24_WriteReg(RX_ADDR_P3, 0xC4);
	nrf24_WriteReg(RX_ADDR_P4, 0xC5);
	nrf24_WriteReg(RX_ADDR_P5, 0xC6);
	uint8_t tx_addr_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
	nrf24_WriteRegMulti(TX_ADDR, tx_addr_def, 5);
	nrf24_WriteReg(RX_PW_P0, 0);
	nrf24_WriteReg(RX_PW_P1, 0);
	nrf24_WriteReg(RX_PW_P2, 0);
	nrf24_WriteReg(RX_PW_P3, 0);
	nrf24_WriteReg(RX_PW_P4, 0);
	nrf24_WriteReg(RX_PW_P5, 0);
	nrf24_WriteReg(FIFO_STATUS, 0x11);
	nrf24_WriteReg(DYNPD, 0);
	nrf24_WriteReg(FEATURE, 0);
	}
}




void NRF24_Init (void)
{
	// disable the chip before configuring the device
	CE_Disable();


	// reset everything
	//nrf24_reset (0);

	nrf24_WriteReg(CONFIG, 0);  // will be configured later

	nrf24_WriteReg(EN_AA, 0);  // No Auto ACK

	nrf24_WriteReg (EN_RXADDR, 0);  // Not Enabling any data pipe right now

	nrf24_WriteReg (SETUP_AW, 0x03);  // 5 Bytes for the TX/RX address

	nrf24_WriteReg (SETUP_RETR, 0);   // No retransmission

	nrf24_WriteReg (RF_CH, 0);  // will be setup during Tx or RX

	nrf24_WriteReg (RF_SETUP, 0x06);   // Power= 0db, data rate = 2Mbps
	
	nrf24_WriteReg (FEATURE, 0x00);
	nrf24_WriteReg (DYNPD, 0x00);

	// Enable the chip after configuring the device
	CE_Enable();

}


// set up the Tx mode

void NRF24_TxMode (uint8_t *Address, uint8_t channel)
{
	// disable the chip before configuring the device
	CE_Disable();

	nrf24_WriteReg (RF_CH, channel);  // select the channel

	nrf24_WriteRegMulti(TX_ADDR, Address, 5);  // Write the TX address
	
	nrf24_WriteReg (RX_PW_P0, 4);
	
	nrf24_WriteReg(RF_SETUP, 0x06);
	
	nrf24_WriteReg(DYNPD, 0x00);
	nrf24_WriteReg(FEATURE, 0x00);

	// clear IRQ flags
   nrf24_WriteReg(STATUS, (1<<6)|(1<<5)|(1<<4));

	// power up the device
	uint8_t config = nrf24_ReadReg(CONFIG);
	config |= (1<<2) | (1<<3);
	config = config | (1<<1);   // write 1 in the PWR_UP bit
	config &= ~(1<<0);   // PRIM_RX = 0 -> TX mode
//	config = config & (0xF2);    // write 0 in the PRIM_RX, and 1 in the PWR_UP, and all other bits are masked
	nrf24_WriteReg (CONFIG, config);

	// Enable the chip after configuring the device
	HAL_Delay(2);
//	CE_Enable();
}


// transmit the data

uint8_t NRF24_Transmit (uint8_t *data)
{
    uint8_t cmdtosend;
	
	  // Clear IRQ flags: RX_DR, TX_DS, MAX_RT
    nrf24_WriteReg(STATUS, (1<<6)|(1<<5)|(1<<4));
	
		// Flush old stuff
    cmdtosend = FLUSH_TX;
    nrfsendCmd(cmdtosend);
	
		nrf24_WriteReg (STATUS, (1<<5) | (1<<4));
	
    // Load the payload
    CS_Select();
    cmdtosend = W_TX_PAYLOAD;
    HAL_SPI_Transmit(NRF24_SPI, &cmdtosend, 1, 100);
    HAL_SPI_Transmit(NRF24_SPI, data, 4, 1000);
    CS_UnSelect();
	
//	  uint8_t status = nrf24_ReadReg(FIFO_STATUS);
//	  if (!(status&(1<<4))){
//			DataInFifo = 0;
//		}
			

    // Pulse CE to start transmission
    CE_Enable();
    HAL_Delay(1); // ~15us min
    CE_Disable();

		 // Wait for TX_DS
    uint32_t t0 = HAL_GetTick();
    while ((HAL_GetTick() - t0) < 20)
    {
        uint8_t status = nrf24_ReadReg(STATUS);

        if (status & (1<<5))  // TX_DS
        {
            nrf24_WriteReg(STATUS, (1<<5)); // clear TX_DS by writing 1
						a = 1;
            return 1;
        }

        HAL_Delay(1);
    }
    // Wait for TX_DS or MAX_RT
//    HAL_Delay(1);
//    status = nrf24_ReadReg(FIFO_STATUS);

//    if (status & TX_DS) {
//        nrf24_WriteReg(STATUS, TX_DS);  // Clear TX_DS
//				a = 20;
//        return 1;  // Success
//    }

//    if ((status&(1<<4)) && !(status&(1<<3))) {
//        //nrf24_WriteReg(STATUS, MAX_RT);  // Clear MAX_RT
//        cmdtosend = FLUSH_TX;
//        nrfsendCmd(cmdtosend);
//				a = 10;
//        return 1;  // Failed
//    }

//    if (status & MAX_RT) {
//        nrf24_WriteReg(STATUS, MAX_RT);  // Clear MAX_RT
//        cmdtosend = FLUSH_TX;
//        nrfsendCmd(cmdtosend);
//				a = 10;
//        return 0;  // Failed
//    }

    return 0;  // Nothing happened
}


void NRF24_RxMode (uint8_t *Address, uint8_t channel)
{
	// disable the chip before configuring the device
	CE_Disable();

//	nrf24_reset (STATUS);

	nrf24_WriteReg (RF_CH, channel);  // select the channel

	// select data pipe 2
	uint8_t en_rxaddr = nrf24_ReadReg(EN_RXADDR);
	en_rxaddr = en_rxaddr | (1<<0);
	nrf24_WriteReg (EN_RXADDR, en_rxaddr);

	/* We must write the address for Data Pipe 1, if we want to use any pipe from 2 to 5
	 * The Address from DATA Pipe 2 to Data Pipe 5 differs only in the LSB
	 * Their 4 MSB Bytes will still be same as Data Pipe 1
	 *
	 * For Eg->
	 * Pipe 1 ADDR = 0xAABBCCDD11
	 * Pipe 2 ADDR = 0xAABBCCDD22
	 * Pipe 3 ADDR = 0xAABBCCDD33
	 *
	 */
	nrf24_WriteRegMulti(RX_ADDR_P0, Address, 5);  // Write the Pipe1 address
	//nrf24_WriteReg(RX_ADDR_P2, 0xEE);  // Write the Pipe2 LSB address

	nrf24_WriteReg (RX_PW_P0, 4);   // 32 bit payload size for pipe 2


	// power up the device in Rx mode
	uint8_t config = nrf24_ReadReg(CONFIG);
	config = config | (1<<1) | (1<<0);
	nrf24_WriteReg (CONFIG, config);

	// Enable the chip after configuring the device
	CE_Enable();
}


uint8_t isDataAvailable (int pipenum)
{
	uint8_t status = nrf24_ReadReg(STATUS);

	if ((status&(1<<6))&&(status&(pipenum<<1)))
	{

		nrf24_WriteReg(STATUS, (1<<6));

		return 1;
	}

	return 0;
}


void NRF24_Receive (uint8_t *data)
{
	uint8_t cmdtosend = 0;

	// select the device
	CS_Select();

	// payload command
	cmdtosend = R_RX_PAYLOAD;
	HAL_SPI_Transmit(NRF24_SPI, &cmdtosend, 1, 100);

	// Receive the payload
	HAL_SPI_Receive(NRF24_SPI, data, 32, 1000);

	// Unselect the device

	CS_UnSelect();

	HAL_Delay(1);

//	cmdtosend = FLUSH_RX;
//	nrfsendCmd(cmdtosend);
}



// Read all the Register data
void NRF24_ReadAll (uint8_t *data)
{
	for (int i=0; i<10; i++)
	{
		*(data+i) = nrf24_ReadReg(i);
	}

	nrf24_ReadReg_Multi(RX_ADDR_P0, (data+10), 5);

	nrf24_ReadReg_Multi(RX_ADDR_P1, (data+15), 5);

	*(data+20) = nrf24_ReadReg(RX_ADDR_P2);
	*(data+21) = nrf24_ReadReg(RX_ADDR_P3);
	*(data+22) = nrf24_ReadReg(RX_ADDR_P4);
	*(data+23) = nrf24_ReadReg(RX_ADDR_P5);

	nrf24_ReadReg_Multi(RX_ADDR_P0, (data+24), 5);

	for (int i=29; i<38; i++)
	{
		*(data+i) = nrf24_ReadReg(i-12);
	}

}



















//extern int a;

//extern SPI_HandleTypeDef hspi1; // Your SPI handle from CubeMX

///* Pin control - Update these GPIO definitions to match your actual pins */
//#define NRF24_CSN_PORT GPIOA
//#define NRF24_CSN_PIN  GPIO_PIN_4
//#define NRF24_CE_PORT  GPIOA
//#define NRF24_CE_PIN   GPIO_PIN_3

///* NRF24L01 Register addresses */
//#define CONFIG     0x00
//#define EN_AA      0x01  
//#define EN_RXADDR  0x02
//#define SETUP_AW   0x03
//#define SETUP_RETR 0x04
//#define RF_CH      0x05
//#define RF_SETUP   0x06
//#define STATUS     0x07
//#define TX_ADDR    0x10
//#define RX_ADDR_P0 0x0A
//#define RX_PW_P0   0x11
//#define FIFO_STATUS   0x17

///* NRF24L01 Commands */
//#define R_REGISTER    0x00
//#define W_REGISTER    0x20
//#define R_RX_PAYLOAD  0x61
//#define W_TX_PAYLOAD  0xA0
//#define FLUSH_TX      0xE1
//#define FLUSH_RX      0xE2
//#define NOP           0xFF

///* Pin control functions */
//void NRF24_CSN_LOW(void) {
//    HAL_GPIO_WritePin(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_PIN_RESET);
//}

//void NRF24_CSN_HIGH(void) {
//    HAL_GPIO_WritePin(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_PIN_SET);
//}

//void NRF24_CE_LOW(void) {
//    HAL_GPIO_WritePin(NRF24_CE_PORT, NRF24_CE_PIN, GPIO_PIN_RESET);
//}

//void NRF24_CE_HIGH(void) {
//    HAL_GPIO_WritePin(NRF24_CE_PORT, NRF24_CE_PIN, GPIO_PIN_SET);
//}

///* Write register */
//void NRF24_WriteReg(uint8_t reg, uint8_t value) {
//    uint8_t command = W_REGISTER | reg;     
//    
//    NRF24_CSN_LOW();
//    HAL_SPI_Transmit(&hspi1, &command, 1, 10);
//    HAL_SPI_Transmit(&hspi1, &value, 1, 10);
//    NRF24_CSN_HIGH();
//}

///* Read register */
//uint8_t NRF24_ReadReg(uint8_t reg) {
//    uint8_t command = R_REGISTER | reg;
//    uint8_t value;
//    
//    NRF24_CSN_LOW();
//    HAL_SPI_Transmit(&hspi1, &command, 1, 10);
//    HAL_SPI_Receive(&hspi1, &value, 1, 10);
//    NRF24_CSN_HIGH();
//    
//    return value;
//}

///* Write multiple bytes to register */
//void NRF24_WriteRegMulti(uint8_t reg, uint8_t *data, uint8_t length) {
//    uint8_t command = W_REGISTER | reg;
//    
//    NRF24_CSN_LOW();
//    HAL_SPI_Transmit(&hspi1, &command, 1, 10);
//    HAL_SPI_Transmit(&hspi1, data, length, 10);
//    NRF24_CSN_HIGH();
//}

///* Get status register */
//uint8_t NRF24_GetStatus(void) {
//    uint8_t cmd = NOP;
//    uint8_t status;
//    
//    NRF24_CSN_LOW();
//    HAL_SPI_TransmitReceive(&hspi1, &cmd, &status, 1, 10);
//    NRF24_CSN_HIGH();
//    
//    return status;
//}

///* Write payload to TX FIFO */
//void NRF24_WritePayload(uint8_t *data, uint8_t length) {
//    uint8_t cmd = W_TX_PAYLOAD;
//    
//    NRF24_CSN_LOW();
//    HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
//    HAL_SPI_Transmit(&hspi1, data, length, 10);
//    NRF24_CSN_HIGH();
//}

///* Flush TX FIFO */
//void NRF24_FlushTx(void) {
//    uint8_t cmd = FLUSH_TX;
//    
//    NRF24_CSN_LOW();
//    HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
//    NRF24_CSN_HIGH();
//}

///* Flush RX FIFO */
//void NRF24_FlushRx(void) {
//    uint8_t cmd = FLUSH_RX;
//    
//    NRF24_CSN_LOW();
//    HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
//    NRF24_CSN_HIGH();
//}

///* Initialize NRF24L01 for TX mode */
//void NRF24_Init(void) {
//    NRF24_CE_LOW();
//    NRF24_CSN_HIGH();
//    HAL_Delay(10);
//    
//    // Basic configuration
//    NRF24_WriteReg(CONFIG, 0x02);       // PWR_UP=1, TX mode initially
//    NRF24_WriteReg(EN_AA, 0x00);        // Disable auto-ack for simplicity
//    NRF24_WriteReg(EN_RXADDR, 0x01);    // Enable pipe 0
//    NRF24_WriteReg(SETUP_AW, 0x03);     // 5-byte address
//    NRF24_WriteReg(SETUP_RETR, 0x00);   // No retransmissions
//    NRF24_WriteReg(RF_CH, 76);          // Channel 76
//    NRF24_WriteReg(RF_SETUP, 0x07);     // 1Mbps, 0dBm
//    NRF24_WriteReg(RX_PW_P0, 1);        // bytes payload (sizeof int)
//    
//    // Set address (same for TX and RX)
////    uint8_t addr[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
//		uint8_t addr[5] = {0xE7, 0xD3, 0xF0, 0x35, 0x77};
//    NRF24_WriteRegMulti(TX_ADDR, addr, 5);
//    NRF24_WriteRegMulti(RX_ADDR_P0, addr, 5);
//    
//    // Clear status
//    NRF24_WriteReg(STATUS, 0x70);
//    
//    // Flush both FIFOs
//    NRF24_FlushTx();
//    NRF24_FlushRx();
//    
//    HAL_Delay(2); // Let radio settle
//}

///* Transmit data */
//uint8_t NRF24_Transmit(uint8_t *data, uint8_t length) {
//    if (length > 32) length = 32; // Max payload size
//    
//    NRF24_CE_LOW();
//    
//    // Clear status flags
//    NRF24_WriteReg(STATUS, 0x70);
//    
//    // Load payload
//    NRF24_WritePayload(data, length);
//    
//    // Start transmission
//    NRF24_CE_HIGH();
//    HAL_Delay(1); // 10µs minimum
//    NRF24_CE_LOW();
//    
//    // Wait for transmission complete (simplified)
//    HAL_Delay(10);
//    
//    // Check status
//    uint8_t status = NRF24_GetStatus();
//    
//    // Clear TX_DS flag if set
//		uint8_t fifo_status = NRF24_ReadReg(FIFO_STATUS);
//		if (fifo_status & (1 << 4)) {
//				a = 20;
//				return 1;
//				// TX FIFO empty ? data has been sent out
//		}
//		
////    if (status & 0x20) {
////        NRF24_WriteReg(STATUS, 0x20);
////				a = 20;
////        return 1; // Success
////    }
//    
//    return 0; // Failed
//}

///* Switch to TX mode */
//void NRF24_TxMode(void) {
//    NRF24_CE_LOW();

//		// Sets channel to 2.476 GHz
//		NRF24_WriteReg(RF_CH, 76);  
//	
////		uint8_t addr[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
//		uint8_t addr[5] = {0xE7, 0xD3, 0xF0, 0x35, 0x77};
//    NRF24_WriteRegMulti(TX_ADDR, addr, 5);
//		
//    // Clear status flags
//    NRF24_WriteReg(STATUS, 0x70);

//    // Flush TX FIFO
//    NRF24_FlushTx();

//    // Switch to TX mode: PWR_UP=1, PRIM_RX=0
//    NRF24_WriteReg(CONFIG, 0x02);

//    HAL_Delay(1); // Let the radio settle
//		
//		NRF24_CE_HIGH();
//}



///* Switch to RX mode */
//void NRF24_RxMode(void) {
//    NRF24_CE_LOW();
//    
//    // Clear status flags
//    NRF24_WriteReg(STATUS, 0x70);
//    
//    // Flush RX FIFO
//    NRF24_FlushRx();
//    
//    // Switch to RX mode
//    NRF24_WriteReg(CONFIG, 0x0B);  // PWR_UP=1, PRIM_RX=1 (RX mode)
//    
//    HAL_Delay(1); // Allow radio to settle
//    
//    // Start listening
//    NRF24_CE_HIGH();
//}

///* Check if data is available */
//uint8_t NRF24_DataAvailable(void) {
//    uint8_t status = NRF24_GetStatus();
//    
//    // Check RX_DR bit (bit 6)
//    if (status & 0x40) {
//        return 1;
//    }
//    
//    // Also check if RX FIFO is not empty
//    uint8_t fifoStatus = NRF24_ReadReg(0x17); // FIFO_STATUS register
//    if ((fifoStatus & 0x01) == 0) { // RX_EMPTY bit = 0 means data available
//        return 1;
//    }
//    
//    return 0;
//}

///* Receive data */
//uint8_t NRF24_Receive(uint8_t *data, uint8_t length) {
//    if (!NRF24_DataAvailable()) {
//        return 0; // No data available
//    }
//    
//    // Read payload
//    uint8_t cmd = R_RX_PAYLOAD;
//    
//    NRF24_CSN_LOW();
//    HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
//    HAL_SPI_Receive(&hspi1, data, length, 10);
//    NRF24_CSN_HIGH();
//    
//    // Clear RX_DR flag
//    NRF24_WriteReg(STATUS, 0x40);
//    
//    return 1; // Data received successfully
//}