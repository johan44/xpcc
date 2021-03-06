
#include <xpcc/architecture.hpp>

//xpcc::stm32::Usart1 uart(115200);

GPIO__OUTPUT(Out, 4, 0);
GPIO__INPUT(In, 4, 1);
GPIO__IO(Io, 4, 2);

int
main(void)
{
	Out::setOutput();
	Out::setOutput(true);
	Out::set();
	Out::reset();	
	Out::toggle();
	
	In::setInput();
	In::read();
	
	Io::setOutput();
	Io::set();
	Io::setInput();
	Io::read();
	
	// uart.write('x');
	
	while (1)
	{
	}
}
