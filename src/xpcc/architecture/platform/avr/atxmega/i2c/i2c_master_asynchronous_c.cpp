// coding: utf-8
// ----------------------------------------------------------------------------
/* Copyright (c) 2009, Roboterclub Aachen e.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Roboterclub Aachen e.V. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ROBOTERCLUB AACHEN E.V. ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ROBOTERCLUB AACHEN E.V. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
// ----------------------------------------------------------------------------
/*
 * WARNING: This file is generated automatically, do not edit!
 * Please modify the corresponding *.in file instead and rebuild this file. 
 */
// ----------------------------------------------------------------------------

#include <avr/io.h>
#include <avr/interrupt.h>
#include <xpcc/architecture/driver/atomic/lock.hpp>
#include "i2c_master_c.hpp"
// uncomment to debugg your driver using simple uart
/*
#include "../uart.hpp"
#define DEBUG_TWI_PRINT_C(x) xpcc::atxmega::BufferedUartF0::write(x)
/*/
#define DEBUG_TWI_PRINT_C(x)
//*/

#if defined(TWIC)

// ----------------------------------------------------------------------------
namespace
{
	// parameter advice
	static bool reading;
	static bool restartAfterReading;
	static bool restartAfterWriting;
	static uint8_t twiAddress;
	
	// buffer management
	static const uint8_t *twiWriteBuffer;
	static uint8_t *twiReadBuffer;
	static uint8_t twiReadCounter;
	static uint8_t twiWriteCounter;
	static uint8_t twiReadBufferSize;
	static uint8_t twiWriteBufferSize;
	
	// state
	static xpcc::i2c::BusState busState = xpcc::i2c::BUS_RESET;
	static bool startConditionGenerated =  false;
	static bool occupied = false;
	static xpcc::i2c::ErrorState errorState = xpcc::i2c::NO_ERROR;
	
	// delegating
	static xpcc::i2c::Delegate *delegate = 0;
}

// ----------------------------------------------------------------------------
/// TWI state machine interrupt handler
ISR(TWIC_TWIM_vect)
{
	switch(TWIC_MASTER_STATUS & TWI_MASTER_BUSSTATE_gm)
	{
		case TWI_MASTER_BUSSTATE_IDLE_gc:
			// Stop condition has been sent
			// chillax and get some coffee
			DEBUG_TWI_PRINT_C('i');
			break;
			
		case TWI_MASTER_BUSSTATE_OWNER_gc:
			// We are writing
			if (TWIC_MASTER_STATUS & TWI_MASTER_WIF_bm)
			{
				// Abitration and Bus Error is handled by the Busy state.
				
				// The Received Acknowledge flag must be checked for each data packet
				// transmitted before the next data packet can be transferred. The master
				// is not allowed to continue transmitting data if the slave signals a NACK.
				// 1 = NACK, 0 = ACK !!!
				if (TWIC_MASTER_STATUS & TWI_MASTER_RXACK_bm) {
					errorState = xpcc::i2c::DATA_NACK;
					if (startConditionGenerated) errorState = xpcc::i2c::ADDRESS_NACK;
					DEBUG_TWI_PRINT_C('N');
					// send stop condition, this will clear the master interrupt flags
					xpcc::atxmega::AsynchronousI2cMasterC::stop();
					// (M2)
					if (delegate) {
						// buffer and reset the callback function pointer
						xpcc::i2c::Delegate *bufferedDelegate = delegate;
						delegate = 0;
						bufferedDelegate->twiError(errorState);
					}
					// calling this after the callback to be able to get some extra
					// information of the error during the callback
					busState = xpcc::i2c::BUS_RESET;
					startConditionGenerated = false;
					TWIC_MASTER_STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
					occupied = false;
					break;
				}
				
				// Data byte has been transmitted and ACK received
				if (twiWriteCounter > 0)
				{
					// Writing the DATA register will trigger a data byte transfer, followed by
					// the master receiving the acknowledge bit from the slave.
					// The WIF and the CLKHOLD flag are set.
					// Accessing the DATA register will clear the master interrupt flags
					// and the CLKHOLD flag.
					TWIC_MASTER_DATA = *twiWriteBuffer++;
					twiWriteCounter--;
					twiWriteBufferSize++;
					DEBUG_TWI_PRINT_C('A');
				}
				else {
					// Transfer finished
					if (twiReadCounter == 0)
					{
						if (restartAfterWriting) {
							xpcc::atxmega::AsynchronousI2cMasterC::restart(twiAddress);
							// the bus is now reserved for further operations
							busState = xpcc::i2c::BUS_WRITE;
							DEBUG_TWI_PRINT_C('C');
						}
						else xpcc::atxmega::AsynchronousI2cMasterC::stop();
						
						if (delegate) {
							xpcc::atomic::Unlock unlock;
							xpcc::i2c::Delegate *bufferedDelegate = delegate;
							delegate = 0;
							bufferedDelegate->twiCompletion(twiWriteBuffer-twiWriteBufferSize, twiWriteBufferSize, xpcc::i2c::WRITE);
						}
					}
					else
					{
						xpcc::atxmega::AsynchronousI2cMasterC::restart(twiAddress);
						TWIC_MASTER_ADDR = twiAddress | xpcc::i2c::READ;
						TWIC_MASTER_CTRLA |= TWI_MASTER_RIEN_bm;
						DEBUG_TWI_PRINT_C('R');
					}
				}
			}
			// We are reading
			else if (TWIC_MASTER_STATUS & TWI_MASTER_RIF_bm)
			{
				// Accessing the DATA register will clear the master interrupt flags
				// and the CLKHOLD flag.
				*twiReadBuffer++ = TWIC_MASTER_DATA;
				twiReadCounter--;
				twiReadBufferSize++;
				
				// Indicating a NACK might not be successfully executed since arbitration
				// can be lost during the transmission.
				if (twiReadCounter > 0) {
					// Send ACK after reception
					TWIC_MASTER_CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
					DEBUG_TWI_PRINT_C('a');
				}
				else {
					if (restartAfterReading) {
						xpcc::atxmega::AsynchronousI2cMasterC::restart(twiAddress);
						// the bus is now reserved for further read operations
						busState = xpcc::i2c::BUS_HOLD;
						DEBUG_TWI_PRINT_C('c');
					}
					else xpcc::atxmega::AsynchronousI2cMasterC::stop();
					
					if (delegate) {
						xpcc::atomic::Unlock unlock;
						xpcc::i2c::Delegate *bufferedDelegate = delegate;
						delegate = 0;
						bufferedDelegate->twiCompletion(twiReadBuffer-twiReadBufferSize, twiReadBufferSize, xpcc::i2c::READ);
					}
				}
			}
			startConditionGenerated = false;
			break;
			
		case TWI_MASTER_BUSSTATE_BUSY_gc:
			// Arbitration Lost, or other master is using the bus
			// wait until bus status returns to IDLE
			// cancel all pending messages
			errorState = xpcc::i2c::ARBITRATION_LOST;
			DEBUG_TWI_PRINT_C('E');
			// send stop condition, this will clear the master interrupt flags
			xpcc::atxmega::AsynchronousI2cMasterC::stop();
			if (delegate) {
				xpcc::i2c::Delegate *bufferedDelegate = delegate;
				delegate = 0;
				bufferedDelegate->twiError(errorState);
			}
			
		default:
			twiReadCounter = 0;
			twiWriteCounter = 0;
			busState = xpcc::i2c::BUS_RESET;
			startConditionGenerated =  false;
			occupied = false;
			delegate = 0;
			TWIC_MASTER_STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
			break;
	}
	DEBUG_TWI_PRINT_C('\n');
}

// ----------------------------------------------------------------------------
void
xpcc::atxmega::AsynchronousI2cMasterC::initialize(uint8_t baud, bool pullUpResistors)
{
	reset();
	
	// SDA, set as input
	PORTC_DIRCLR = (1 << 0);
	// SCL, set as output
	PORTC_DIRSET = (1 << 1);
	// set pull up resistors if wanted
	if (pullUpResistors) {
		PORTC_PIN0CTRL = PORT_OPC_PULLUP_gc;
		PORTC_PIN1CTRL = PORT_OPC_PULLUP_gc;
	}
	
	// SDA Hold Time and External Driver Interface disabled
	TWIC_CTRL = 0;
	
	// set the baud rate register
	TWIC_MASTER_BAUD = baud;
	
	// Enable the Master, and the interrupts
	TWIC_MASTER_CTRLA = TWI_MASTER_INTLVL_MED_gc | TWI_MASTER_ENABLE_bm;
	// clear register b
	TWIC_MASTER_CTRLB = 0;
	// clear register c
	TWIC_MASTER_CTRLC = 0;
	
	// Set the bus state to idle
	TWIC_MASTER_STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
}

void
xpcc::atxmega::AsynchronousI2cMasterC::reset()
{	
	twiReadCounter = 0;
	twiWriteCounter = 0;
	busState = xpcc::i2c::BUS_RESET;
	startConditionGenerated =  false;
	occupied = false;
	errorState = xpcc::i2c::NO_ERROR;
	delegate = 0;
}

// MARK: - management
bool
xpcc::atxmega::AsynchronousI2cMasterC::start(uint8_t slaveAddress)
{
	if (getBusyState() == xpcc::i2c::FREE)
	{
		occupied = true;
		// Copy address; clear R/~W bit in SLA+R/W address field
		twiAddress = slaveAddress & 0xfe;
		busState = xpcc::i2c::BUS_STANDBY;
		startConditionGenerated = true;
		errorState = xpcc::i2c::NO_ERROR;
		
		// Disable the interrupts
		TWIC_MASTER_CTRLA &= ~(TWI_MASTER_RIEN_bm | TWI_MASTER_WIEN_bm);
		DEBUG_TWI_PRINT_C('\r');
		DEBUG_TWI_PRINT_C('s');
		DEBUG_TWI_PRINT_C('\n');
		
		return true;
	}
	return false;
}

void
xpcc::atxmega::AsynchronousI2cMasterC::restart(uint8_t slaveAddress)
{
	if (!startConditionGenerated)
	{
		occupied = true;
		// Copy address; clear R/~W bit in SLA+R/W address field
		twiAddress = slaveAddress & 0xfe;
		busState = xpcc::i2c::BUS_STANDBY;
		startConditionGenerated = true;
		
		// Initiate a START condition, disable the interrupts
		TWIC_MASTER_CTRLA &= ~(TWI_MASTER_RIEN_bm | TWI_MASTER_WIEN_bm);
		DEBUG_TWI_PRINT_C('s');
	}
}

void
xpcc::atxmega::AsynchronousI2cMasterC::stop()
{
	if (busState != xpcc::i2c::BUS_STOPPED && busState != xpcc::i2c::BUS_RESET)
	{
		// send nack and stop
		TWIC_MASTER_CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;
		busState = xpcc::i2c::BUS_STOPPED;
		startConditionGenerated = false;
		occupied = false;
		DEBUG_TWI_PRINT_C('S');
	}
}

// MARK: - operations
void
xpcc::atxmega::AsynchronousI2cMasterC::read(uint8_t *data, std::size_t size, xpcc::i2c::OperationParameter param)
{
	writeRead(data, 0, size, false, (param == xpcc::i2c::READ_RESTART));
}

void
xpcc::atxmega::AsynchronousI2cMasterC::write(uint8_t *data, std::size_t size, xpcc::i2c::OperationParameter param)
{
	writeRead(data, size, 0, (param == xpcc::i2c::WRITE_RESTART), false);
}

void
xpcc::atxmega::AsynchronousI2cMasterC::writeRead(uint8_t *data, std::size_t writeSize, std::size_t readSize, xpcc::i2c::OperationParameter param)
{
	writeRead(data, writeSize, readSize, false, (param == xpcc::i2c::READ_RESTART));
}

// MARK: - delegate
void
xpcc::atxmega::AsynchronousI2cMasterC::attachDelegate(xpcc::i2c::Delegate *object)
{
	delegate = object;
}

// MARK: - status
xpcc::i2c::ErrorState
xpcc::atxmega::AsynchronousI2cMasterC::getErrorState()
{
	return errorState;
}

xpcc::i2c::BusyState
xpcc::atxmega::AsynchronousI2cMasterC::getBusyState()
{
	if (occupied) return xpcc::i2c::OCCUPIED;
	return static_cast<xpcc::i2c::BusyState>(TWIC_MASTER_STATUS & TWI_MASTER_BUSSTATE_gm);
}

xpcc::i2c::BusState
xpcc::atxmega::AsynchronousI2cMasterC::getBusState()
{
	return busState;
}

// MARK: - private
void
xpcc::atxmega::AsynchronousI2cMasterC::writeRead(uint8_t *data, std::size_t writeSize, std::size_t readSize, bool restartW, bool restartR)
{
	// Save pointer to data and number of bytes to send
	twiWriteBuffer = data;
	twiReadBuffer = data;
	twiWriteCounter = writeSize;
	twiReadCounter = readSize;
	twiWriteBufferSize = 0;
	twiReadBufferSize = 0;
	
	restartAfterWriting = restartW;
	restartAfterReading = restartR;
	reading = !writeSize;
	
	// Generate START condition, this also sends the address and R/W
	TWIC_MASTER_ADDR = twiAddress | reading;
	
	TWIC_MASTER_CTRLA |= reading ? TWI_MASTER_RIEN_bm : TWI_MASTER_WIEN_bm;
}

#endif