
MEMORY
{
	ROM (rx)  : ORIGIN = 0x08000000, LENGTH = 16k
	RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 6k
}

INCLUDE ../../common/memory_sections.ld
