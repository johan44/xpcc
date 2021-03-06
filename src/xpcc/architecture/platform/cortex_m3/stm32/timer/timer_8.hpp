// coding: utf-8
// ----------------------------------------------------------------------------
/* Copyright (c) 2011, Roboterclub Aachen e.V.
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

#ifndef XPCC_STM32__TIMER_8_HPP
#define XPCC_STM32__TIMER_8_HPP

#include "timer_base.hpp"

namespace xpcc
{
	namespace stm32
	{
		/**
		 * @brief		Advanced Control Timer 8
		 * 
		 * 
		 * Interrupt handler for High Density:
		 * - TIM8_BRK_IRQHandler
		 * - TIM8_UP_IRQHandler
		 * - TIM8_TRG_COM_IRQHandler
		 * - TIM8_CC_IRQHandler
		 * 
		 * Interrupt handler for XL Density:
		 * - TIM8_BRK_TIM12_IRQn
		 * - TIM8_UP_TIM13_IRQn
		 * - TIM8_TRG_COM_TIM14_IRQn
		 * - TIM8_CC_IRQn
		 * 
		 * 
		 * Example:
		 * \code
		 * extern "C" void
		 * TIM8_UP_IRQHandler(void)
		 * {
		 *     Timer8::resetInterruptFlags(Timer8::...);
		 *     
		 *     ...
		 * }
		 * \endcode
		 * 
		 * \warning	The Timer has much more possibilities than presented by this
		 * 			interface (e.g. Input Capture, Trigger for other Timers, DMA).
		 * 			It might be expanded in the future.
		 * 
		 * @author		Fabian Greif
		 * @ingroup		stm32
		 */
		class Timer8 : public AdvancedControlTimer
		{
#if defined(STM32F2XX) || defined(STM32F4XX)
		public:
			
#else
		
#endif
			
		public:
			enum SlaveModeTrigger
			{
				TRIGGER_INTERNAL_0 = 0,
				TRIGGER_INTERNAL_1 = TIM_SMCR_TS_0,
				TRIGGER_INTERNAL_2 = TIM_SMCR_TS_1,
				TRIGGER_INTERNAL_3 = TIM_SMCR_TS_1 | TIM_SMCR_TS_0,
				TRIGGER_TI1_EDGE_DETECTOR = TIM_SMCR_TS_2,
				TRIGGER_FILTERED_TI1 = TIM_SMCR_TS_2 | TIM_SMCR_TS_0,
				TRIGGER_FILTERED_TI2 = TIM_SMCR_TS_2 | TIM_SMCR_TS_1,
				TRIGGER_EXTERNAL = TIM_SMCR_TS_2 | TIM_SMCR_TS_1 | TIM_SMCR_TS_0,
			};
			
			enum SlaveMode
			{
				SLAVE_DISABLED	= 0, // Slave mode disabled - if CEN = '1' then the prescaler is clocked directly by the internal clock.
				SLAVE_ENCODER_1	= TIM_SMCR_SMS_0, // Counter counts up/down on TI2FP2 edge depending on TI1FP1 level.
				SLAVE_ENCODER_2	= TIM_SMCR_SMS_1, // Counter counts up/down on TI1FP1 edge depending on TI2FP2 level.
				SLAVE_ENCODER_3	= TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0, // Counter counts up/down on both TI1FP1 and TI2FP2 edges depending on the level of the other input.
				SLAVE_RESET		= TIM_SMCR_SMS_2, // Rising edge of the selected trigger input (TRGI) reinitializes the counter and generates an update of the registers.
				SLAVE_GATED		= TIM_SMCR_SMS_2 | TIM_SMCR_SMS_0, // The counter clock is enabled when the trigger input (TRGI) is high. The counter stops (but is not reset) as soon as the trigger becomes low. Both start and stop of the counter are controlled.
				SLAVE_TRIGGER	= TIM_SMCR_SMS_2 | TIM_SMCR_SMS_1, // The counter starts at a rising edge of the trigger TRGI (but it is not reset). Only the start of the counter is controlled.
				SLAVE_EXTERNAL_CLOCK = TIM_SMCR_SMS_2 | TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0, // Rising edges of the selected trigger (TRGI) clock the counter.
			};
			
			static void
			enable();
			
			static void
			disable();
			
			static inline void
			pause()
			{
				TIM8->CR1 &= ~TIM_CR1_CEN;
			}
			
			static inline void
			start()
			{
				TIM8->CR1 |= TIM_CR1_CEN;
			}
			
			static void
			setMode(Mode mode,
					SlaveMode slaveMode = SLAVE_DISABLED,
					SlaveModeTrigger slaveModeTrigger = TRIGGER_INTERNAL_0);
			
			static inline void
			setPrescaler(uint16_t prescaler)
			{
				// Because a prescaler of zero is not possible the actual
				// prescaler value is \p prescaler - 1 (see Datasheet)
				TIM8->PSC = prescaler - 1;
			}
			
			static inline void
			setOverflow(uint16_t overflow)
			{
				TIM8->ARR = overflow;
			}
			
			static uint16_t
			setPeriod(uint32_t microseconds, bool autoApply = true);
			
			static inline void
			applyAndReset()
			{
				// Generate Update Event to apply the new settings for ARR
				TIM8->EGR |= TIM_EGR_UG;
			}
			
			static inline uint16_t
			getValue()
			{
				return TIM8->CNT;
			}
			
			static inline void
			setValue(uint16_t value)
			{
				TIM8->CNT = value;
			}
			
			static inline void
			enableOutput()
			{
				TIM8->BDTR |= TIM_BDTR_MOE;
			}
			
		public:
			static void
			configureInputChannel(uint32_t channel, InputCaptureMapping input,
					InputCapturePrescaler prescaler, InputCapturePolarity polarity, uint8_t filter);
			
			static void
			configureOutputChannel(uint32_t channel, OutputCompareMode mode,
					uint16_t compareValue);
			
			static inline void
			setCompareValue(uint32_t channel, uint16_t value)
			{
#if defined(STM32F2XX) || defined(STM32F4XX)
				*(&TIM8->CCR1 + (channel - 1)) = value;
#else
				*(&TIM8->CCR1 + ((channel - 1) * 2)) = value;
#endif
			}
			
			static inline uint16_t
			getCompareValue(uint32_t channel)
			{
#if defined(STM32F2XX) || defined(STM32F4XX)
				return *(&TIM8->CCR1 + (channel - 1));
#else
				return *(&TIM8->CCR1 + ((channel - 1) * 2));
#endif
			}
			
		public:
			/**
			 * Enables or disables Interrupt Vectors.
			 * 
			 * You must pass an or-conjuncted list of entries from the
			 * Interrupt enum. Interrupt vectors which fit to 
			 * will be enabled (or disabled if \p enable = false).
			 * 
			 * The Adanvced timers have four interrupt vectors:
			 * - UP (used by INTERRUPT_UPDATE)
			 * - BRK (used by INTERRUPT_BREAK)
			 * - TRG_COM (used by INTERRUPT_TRIGGER and INTERRUPT_COM)
			 * - CC (used by INTERRUPT_CAPTURE_COMPARE_1..3)
			 */
			static void
			enableInterruptVector(Interrupt interrupt, bool enable, uint32_t priority);
			
			static inline void
			enableInterrupt(Interrupt interrupt)
			{
				TIM8->DIER |= interrupt;
			}
			
			static inline void
			disableInterrupt(Interrupt interrupt)
			{
				TIM8->DIER &= ~interrupt;
			}
			
			static inline void
			enableDmaRequest(DmaRequestEnable dmaRequests)
			{
				TIM8->DIER |= dmaRequests;
			}
			
			static inline void
			disableDmaRequest(DmaRequestEnable dmaRequests)
			{
				TIM8->DIER &= ~dmaRequests;
			}
			
			static inline InterruptFlag
			getInterruptFlags()
			{
				return (InterruptFlag) TIM8->SR;
			}
			
			static inline void
			resetInterruptFlags(InterruptFlag flags)
			{
				// Flags are cleared by writing a zero to the flag position.
				// Writing a one is ignored.
				TIM8->SR = ~flags;
			}
		};
	}
}

#endif // XPCC_STM32__TIMER_8_HPP