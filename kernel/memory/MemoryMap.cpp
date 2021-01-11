#include <stdint.h>
#include <memory/MemoryMap.h>
#include <kernel/panic.h>

namespace Memory
{

    void MemoryMap::Parse(void *basePtr)
    {
        header = (MemoryMapHeader *)basePtr;
        entries = (MemoryMapEntry *)(basePtr + sizeof(MemoryMapHeader));

        if (header->result == 1 && header->length == 0)
        {
            Kernel::Panic("MemoryMap: Empty memory map");
        }
        else if (header->result == 2)
        {
            Kernel::Panic("MemoryMap: Bad BIOS signature");
        }
        else if (header->result == 3)
        {
            Kernel::Panic("MemoryMap: Function not available");
        }
    }

    MemoryMapEntry *MemoryMap::GetEntry(int idx)
    {
        return &entries[idx];
    }

    uint32_t MemoryMap::GetLength()
    {
        return header->length;
    }

    void *MemoryMap::GetLargestChunk()
    {
        MemoryMapEntry *largestEntry = GetEntry(0);
        for (int i = 0; i < GetLength(); i++)
        {
            auto *entry = GetEntry(i);
            if (entry->type == 0x01 && entry->lengthLow > largestEntry->lengthLow)
                largestEntry = entry;
        }
        return largestEntry;
    }

}; // namespace Memory