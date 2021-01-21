#ifndef _ELF_LOADER_H
#define _ELF_LOADER_H

#include <kernel/tasks/thread.h>
#include <elf/elf.h>

namespace Kernel
{

    class ElfLoader
    {
    public:
        static Thread *LoadElf(const ELF::Image &image);

    private:
        static void MapNewZeroedPage(Memory::PageDirectory *dir, vaddress_t vaddr);
    };

} // namespace Kernel

#endif