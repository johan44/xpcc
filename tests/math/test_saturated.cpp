
#include <avr/io.h>
#include "../../src/math/saturated.hpp"

using namespace xpcc;

Saturated<int8_t> a;
Saturated<int8_t> b(125);
Saturated<int8_t> c;

int
main(void)
{
	a += Saturated<int8_t>(12);
	
	c = a + b;
	
	PORTB = c.getValue();
	
	while (1) {
		
	}
}