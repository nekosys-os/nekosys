#include <stdio.h>
#include <kernel/io.h>
#include <kernel/disk/atadisk.h>
#include <kernel/kdebug.h>

using namespace Kernel;

#define DRIVE_IRQ_BASE 14

#define DRIVE_PORT_DATA 0x1F0
#define DRIVE_PORT_SECTOR_COUNT 0x1F2
#define DRIVE_PORT_LBALO 0x1F3
#define DRIVE_PORT_LBAMID 0x1F4
#define DRIVE_PORT_LBAHI 0x1F5
#define DRIVE_PORT_SELECT 0x1F6
#define DRIVE_PORT_CMD 0x1F7
#define DRIVE_PORT_STATUS 0x1F7

#define DRIVE_COMMAND_IDENTIFY 0xEC
#define DRIVE_COMMAND_READ 0x20

#define ERR 0x01
#define DRQ 0x08
#define DRDY 0x40
#define BSY 0x80

namespace Disk
{

    ATADisk::ATADisk(unsigned int disk_num)
    {
        Interrupts::AddHandler(DRIVE_IRQ_BASE + disk_num, this);

        // Select disk
        IO::Out8(DRIVE_PORT_SELECT, 0x0A + disk_num);

        // Get disk info
        for (int i = DRIVE_PORT_SECTOR_COUNT; i <= DRIVE_PORT_LBAHI; i++)
            IO::Out8(i, 0);

        IO::Out8(DRIVE_PORT_CMD, DRIVE_COMMAND_IDENTIFY);
        auto status = IO::In8(DRIVE_PORT_STATUS);
        if (status == 0x00)
            return;

// wait for BSY flag to clear
#if DISK_DEBUG
        kdbg("disk: waiting for disk to get ready\n");
#endif
        while ((status = IO::In8(DRIVE_PORT_STATUS)) & BSY && status != 0xff)
            ;

        if (status == 0xff)
        {
            printf("Disk %d not present\n", disk_num);
            return;
        }

        if (IO::In8(DRIVE_PORT_LBAMID) != 0 || IO::In8(DRIVE_PORT_LBAHI) != 0)
        {
            printf("Disk at %d is not ATA.\n", disk_num);
            return;
        }

#if DISK_DEBUG
        kdbg("disk: waiting for disk to get ready\n");
#endif
        while (!((status = IO::In8(DRIVE_PORT_STATUS)) & DRQ) && !(status & ERR))
            ;

        if (status & ERR)
        {
            printf("Disk %d returned an error\n", disk_num);
            return;
        }

#if DISK_DEBUG
        kdbg("disk: reading identify data\n");
#endif
        uint16_t *identifyData = new uint16_t[256];
        ReadRaw(identifyData, 256);

        // Extract disk name
        char *diskName = new char[40];
        memcpy(diskName, ((uint8_t *)identifyData) + 54, 40);

        for (int i = 0; i < 40; i += 2)
        {
            auto tmp = diskName[i];
            diskName[i] = diskName[i + 1];
            diskName[i + 1] = tmp;
        }

        for (int i = 39; i >= 0; i--)
            if (diskName[i] == ' ')
                diskName[i] = 0x00;
            else
                break;

        // Print data
        uint32_t *numSectors = (uint32_t *)(identifyData + 60);
        max_sectors = *numSectors;
        available = true;
        printf("Disk %d reported as %s with %d sectors\n", disk_num, diskName, *numSectors);

        delete identifyData;
        delete diskName;
    }

    // FIXME: Disk hangs after reading more than one block
    void ATADisk::ReadBlock(uint64_t block_idx, uint64_t block_num, uint8_t *data)
    {
        while (IO::In8(DRIVE_PORT_STATUS) & BSY)
            ;

        if (block_num == 256)
            block_num = 0;

#if DISK_DEBUG
        kdbg("disk: reading %d blocks from %d\n", (int)block_num, (int)block_idx);
#endif

        IO::Out8(DRIVE_PORT_SELECT, 0xE0 | ((block_idx >> 24) & 0x0F));
        IO::Out8(DRIVE_PORT_SECTOR_COUNT, (uint8_t)block_num);
        IO::Out8(DRIVE_PORT_LBALO, (uint8_t)block_idx);
        IO::Out8(DRIVE_PORT_LBAMID, (uint8_t)(block_idx >> 8));
        IO::Out8(DRIVE_PORT_LBAHI, (uint8_t)(block_idx >> 16));

        while (!(IO::In8(DRIVE_PORT_STATUS) & DRDY))
            ;

        IO::Out8(DRIVE_PORT_CMD, DRIVE_COMMAND_READ);

        // FIXME: Busy waiting and polling is a big nono
        while (!(IO::In8(DRIVE_PORT_STATUS) & DRQ) || (IO::In8(DRIVE_PORT_STATUS) & BSY))
            ;

        for (size_t i = 0; i < block_num; i++)
        {
            ReadRaw((uint16_t *)(data + 256 * i), 256);
        }
    }

    void ATADisk::WriteBlock(uint64_t, uint64_t, uint8_t *)
    {
        // not yet implemented
    }

    void ATADisk::HandleInterrupt(unsigned int, RegisterStates *)
    {
        interrupted = true;
#if DISK_DEBUG
        kdbg("disk: interrupted\n");
#endif
    }

    void ATADisk::WaitForInterrupt()
    {
#if DISK_DEBUG
        kdbg("disk: waiting for interrupt\n");
#endif
        interrupted = false;
        Interrupts::Enable();
        while (!interrupted)
        {
            // FIXME: Needs yield and timeout
        }
        Interrupts::Disable();
    }

    void ATADisk::ReadRaw(uint16_t *dst, size_t num_words)
    {
        for (size_t i = 0; i < num_words; i++)
        {
            dst[i] = IO::In16(DRIVE_PORT_DATA);
        }
    }

}; // namespace Disk