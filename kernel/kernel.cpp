#include <kernel/tty.h>
#include <kernel/heap.h>
#include <kernel/gdt.h>
#include <kernel/panic.h>
#include <kernel/interrupts.h>
#include <device/devicemanager.h>
#include <memory/MemoryMap.h>
#include <stdio.h>

using namespace Kernel;
using namespace Device;
using namespace Memory;

/* nekosys Kernel entry point */
extern "C" void nkmain()
{
	Interrupts::Disable();

	// Banner
	TTY::Clear();
	TTY::SetColor(0x9f);
	printf("neko_kernel 0.01 <by Twometer>\n");
	TTY::SetColor(0x07);

	// Init
	printf("Initializing...\n");
	GDT gdt(3);
	gdt.Set(0, {0, 0, 0});			   // Selector 0x00: NULL
	gdt.Set(1, {0, 0xffffffff, 0x9A}); // Selector 0x08: Code
	gdt.Set(2, {0, 0xffffffff, 0x92}); // Selector 0x10: Data
	// Task state segment here
	gdt.Load();

	printf("Loading memory map...\n");

	TTY::SetColor(0x08);
	MemoryMap memoryMap;
	memoryMap.Parse((void *)0x8000);

	for (int i = 0; i < memoryMap.GetLength(); i++)
	{
		auto *entry = memoryMap.GetEntry(i);
		if (entry->type == 0x01)
			printf("  base=%x len=%d KB\n", entry->baseLow, (entry->lengthLow / 1024));
	}
	TTY::SetColor(0x07);

	void *heapBase = memoryMap.GetLargestChunk();
	heap_init(heapBase);

	Interrupts::SetupIdt();
	DeviceManager::Initialize();

	Interrupts::Enable();

	// Dummy terminal
	printf("Initialized.\n\n");
	printf("$ ");

	// Idle
	Device::CPU::Halt();
}
