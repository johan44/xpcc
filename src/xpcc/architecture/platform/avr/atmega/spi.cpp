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
 *
 * $Id: spi.cpp 750 2012-03-01 17:01:46Z salkinium $
 */
// ----------------------------------------------------------------------------

#include "spi.hpp"

#ifdef SPDR

namespace
{
	static uint8_t* transmitBuffer(0);
	static uint8_t* receiveBuffer(0);
	static uint16_t bufferLength(0);
	enum
	{
		BUFFER_TRANSMIT_INCR_bm = 0x01,
		BUFFER_RECEIVE_INCR_bm = 0x02,
		BUFFER_TRANSMIT_IS_NOT_ZERO_bm = 0x04,
		BUFFER_RECEIVE_IS_NOT_ZERO_bm = 0x08,
		BUFFER_IS_DUMMY_bm = 0x10,
		BUFFER_IS_BUSY_SYNC_bm = 0x20
	};
	static uint8_t status(0);
}

void
xpcc::atmega::SpiMaster::initialize(Mode mode, Prescaler prescaler)
{
	SPCR = (1 << SPE) | (1 << MSTR) | (prescaler & ~0x80) | mode;
	SPSR = (prescaler & 0x80) ? (1 << SPI2X) : 0;
}

// ----------------------------------------------------------------------------
bool
xpcc::atmega::SpiMaster::setBuffer(uint16_t length, uint8_t* transmit, uint8_t* receive, bool transmitIncr, bool receiveIncr)
{
	if (!isFinished()) return false;
	
	transmitBuffer = transmit;
	receiveBuffer = receive ? receive : transmit;
	bufferLength = length;
	status &= ~(BUFFER_TRANSMIT_INCR_bm | BUFFER_RECEIVE_INCR_bm);
	status |= (transmitIncr ? BUFFER_TRANSMIT_INCR_bm : 0) | (receiveIncr ? BUFFER_RECEIVE_INCR_bm : 0);
	
	return true;
}

bool
xpcc::atmega::SpiMaster::transfer(bool transmit, bool receive, bool /*wait*/)
{
	if (status & BUFFER_IS_BUSY_SYNC_bm)
		return false;
	
	uint8_t rx(0), tx(0xff);
	// send the buffer out, blocking
	status |= BUFFER_IS_BUSY_SYNC_bm;
	// check if we have to use a dummy buffer
	transmit &= static_cast<bool>(transmitBuffer);
	receive &= static_cast<bool>(receiveBuffer);
	
	if (status & BUFFER_TRANSMIT_INCR_bm) {
		for(uint_fast16_t i=0; i < bufferLength; ++i) {
			if (transmit) tx = transmitBuffer[i];
			rx = write(tx);
			if (receive) receiveBuffer[i] = rx;
		}
	}
	else {
		for(uint_fast16_t i=bufferLength; i > 0; --i) {
			if (transmit) tx = transmitBuffer[i-1];
			rx = write(tx);
			if (receive) receiveBuffer[i-1] = rx;
		}
	}
	status &= ~BUFFER_IS_BUSY_SYNC_bm;
	
	return true;
}

bool
xpcc::atmega::SpiMaster::isFinished()
{
	return !(status & BUFFER_IS_BUSY_SYNC_bm);
}

// ----------------------------------------------------------------------------
uint8_t
xpcc::atmega::SpiMaster::write(uint8_t data)
{
	SPDR = data;
	while (!(SPSR & (1 << SPIF))) {
		// wait for the transmission to complete
	}
	return SPDR;
}

#endif
