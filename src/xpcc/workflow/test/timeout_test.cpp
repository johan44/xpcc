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

#include <xpcc/workflow/timeout.hpp>

#include "timeout_test.hpp"

// ----------------------------------------------------------------------------
// dummy implementation to control the time

namespace
{
	class DummyClock
	{
	public:
		static xpcc::Timestamp
		now()
		{
			return time;
		}
		
		static void
		setTime(xpcc::Timestamp time)
		{
			DummyClock::time = time;
		}
		
	private:
		static xpcc::Timestamp time;
	};
	
	xpcc::Timestamp DummyClock::time = 0;
}

// ----------------------------------------------------------------------------
void
TimeoutTest::setUp()
{
	DummyClock::setTime(0);
}

void
TimeoutTest::testBasics()
{
	xpcc::Timeout<DummyClock> timeout(10);
	
	TEST_ASSERT_FALSE(timeout.isExpired());
	
	int i;
	for (i = 0; i < 9; ++i) {
		DummyClock::setTime(i);
		TEST_ASSERT_FALSE(timeout.isExpired());
	}
	
	DummyClock::setTime(10);
	TEST_ASSERT_TRUE(timeout.isExpired());
	
	// check if the class holds the state
	DummyClock::setTime(9);
	TEST_ASSERT_TRUE(timeout.isExpired());
}

void
TimeoutTest::testDefaultConstructor()
{
	xpcc::Timeout<DummyClock> timeout;
	TEST_ASSERT_TRUE(timeout.isExpired());
}

void
TimeoutTest::testTimeOverflow()
{
	xpcc::Timestamp::Type time = xpcc::ArithmeticTraits<xpcc::Timestamp::Type>::max();
	
	// overflow after 65535 for uint16_t => 32767+100 = 32867
	DummyClock::setTime(time / 2 + 100);
	
	xpcc::Timeout<DummyClock> timeout(time / 2 - 1);	//=> 32867 + 32767 = 65634 - 65535 = 99
	
	TEST_ASSERT_FALSE(timeout.isExpired());
	
	DummyClock::setTime(time);
	TEST_ASSERT_FALSE(timeout.isExpired());
	
	DummyClock::setTime(0);
	TEST_ASSERT_FALSE(timeout.isExpired());
	
	// Overflow happened. This needs to be avoided by the user!
	DummyClock::setTime(100);
	TEST_ASSERT_TRUE(timeout.isExpired());
}

void
TimeoutTest::testRestart()
{
	xpcc::Timeout<DummyClock> timeout;
	TEST_ASSERT_TRUE(timeout.isExpired());
	
	timeout.restart(42);
	TEST_ASSERT_FALSE(timeout.isExpired());
	
	DummyClock::setTime(10);
	TEST_ASSERT_FALSE(timeout.isExpired());
	
	DummyClock::setTime(600);
	TEST_ASSERT_TRUE(timeout.isExpired());
}
