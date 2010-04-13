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


#include <avr/io.h>
#include <avr/interrupt.h>

#include <xpcc/architecture/general/atomic/queue.hpp>
#include <xpcc/architecture/general/atomic/lock.hpp>

#include <xpcc/architecture/general/gpio.hpp>

#include "uart_d1.hpp"
#include "xpcc_config.h"

#ifdef USARTD1

namespace
{
	static xpcc::atomic::Queue<char, UARTD1_RX_BUFFER_SIZE> rxBuffer;
	static xpcc::atomic::Queue<char, UARTD1_TX_BUFFER_SIZE> txBuffer;
	
	GPIO__INPUT(RXD, D, 6);
	GPIO__OUTPUT(TXD, D, 7);
}

// ----------------------------------------------------------------------------
ISR(USARTD1_RXC_vect)
{
	uint8_t data = USARTD1_DATA;
	
	// TODO Fehlerbehandlung
	//USARTD1_STATUS & (FERR, BUFOVF, PERR)
	
	rxBuffer.push(data);
}

// ----------------------------------------------------------------------------
ISR(USARTD1_DRE_vect)
{
	if (txBuffer.isEmpty())
	{
		// transmission finished, disable DRE interrupt
		USARTD1_CTRLA = USART_RXCINTLVL_MED_gc;
	}
	else {
		// get one byte from buffer and write it to the UART buffer
		// which starts the transmission
		USARTD1_DATA = txBuffer.get();
		txBuffer.pop();
	}
}

// ----------------------------------------------------------------------------
void
xpcc::BufferedUartD1::setBaudrateRegister(uint16_t ubrr, bool doubleSpeed)
{
	TXD::high();
	TXD::setOutput();
	
	RXD::setInput();
	
	// interrupts should be disabled during initialization
	atomic::Lock lock;
	
	// set baud rate
	if (doubleSpeed) {
		USARTD1.CTRLB = USART_CLK2X_bm;
	}
	else {
		USARTD1.CTRLB = 0;
	}
	USARTD1.BAUDCTRLB = static_cast<uint8_t>(ubrr >> 8);
	USARTD1.BAUDCTRLA = static_cast<uint8_t>(ubrr);
	
	// enable receive complete interrupt
	USARTD1.CTRLA = USART_RXCINTLVL_MED_gc;
	
	// setting the frame size to 8 bit
	USARTD1.CTRLC = USART_CHSIZE_8BIT_gc;
	
	// enable both, receiver and transmitter
	USARTD1.CTRLB |= USART_RXEN_bm | USART_TXEN_bm;
}

// ----------------------------------------------------------------------------
void
xpcc::BufferedUartD1::write(char c)
{
	uint16_t i(0);		
	while ( !txBuffer.push(c) && (i < 1000) ) {
		++i;
		// wait for a free slot in the buffer
		// but do not wait infinity
	}
	
	// disable interrupts
	atomic::Lock lock;
	
	// enable DRE interrupt
	USARTD1_CTRLA = USART_RXCINTLVL_MED_gc | USART_DREINTLVL_MED_gc;
}

// ----------------------------------------------------------------------------
void
xpcc::BufferedUartD1::write(const char *s)
{
	char c;
	while ((c = *s++)) {
		BufferedUartD1::write(c);
	}
}

// ----------------------------------------------------------------------------
bool
xpcc::BufferedUartD1::read(char& c)
{
	if (rxBuffer.isEmpty()) {
		return false;
	}
	else {
		c = rxBuffer.get();
		rxBuffer.pop();
		
		return true;
	}
}

// ----------------------------------------------------------------------------
uint8_t
xpcc::BufferedUartD1::read(char *buffer, uint8_t n)
{
	for (uint8_t i = 0; i < n; ++i)
	{
		if (rxBuffer.isEmpty()) {
			return n;
		}
		else {
			*buffer++ = rxBuffer.get();
			rxBuffer.pop();
		}
	}
	
	return n;
}

#endif // USARTD1