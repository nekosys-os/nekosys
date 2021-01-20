#include <stdio.h>
#include <fs/fat16.h>
#include <nk/buffer.h>

#define FAT16_DEBUG 0

#define DIRENT_SIZE 32

namespace FS
{
    // FIXME: Fat16 FS currently only knows the root directory

    Fat16::Fat16(Disk::IBlockDevice *blockDevice, Partition *partition) : FileSystem(blockDevice, partition)
    {
        auto baseSector = new uint8_t[512];
        blockDevice->ReadBlock(partition->startSector, 1, baseSector);

        nk::Buffer buf(baseSector, 512);
        bytes_per_block = buf.Read<uint16_t>(0x0b);
        blocks_per_alloc = buf.Read<uint8_t>(0x0d);
        reserved_blocks = buf.Read<uint16_t>(0x0e);
        num_fats = buf.Read<uint8_t>(0x10);
        root_dir_entries = buf.Read<uint16_t>(0x11);
        media_descriptor = buf.Read<uint8_t>(0x15);
        blocks_per_fat = buf.Read<uint16_t>(0x16);
        hidden_blocks = buf.Read<uint16_t>(0x1c);
        total_sectors = buf.Read<uint16_t>(0x20);
        delete baseSector;

        root_dir_sector = partition->startSector + reserved_blocks + blocks_per_fat * num_fats;

        auto rootdirSize = DIRENT_SIZE * root_dir_entries;
        root_directory = new uint8_t[rootdirSize];
        for (int i = 0; i < rootdirSize / 512; i++)
            blockDevice->ReadBlock(root_dir_sector + i, 1, root_directory + i * 512);

        current_cluster = new uint8_t[blocks_per_alloc * bytes_per_block];

#if FAT16_DEBUG
        printf("fat_16: Root dir at %d with %d entries\n", (int)root_dir_sector, (int)root_dir_entries);
#endif

        auto fat_base = partition->startSector + reserved_blocks;
#if FAT16_DEBUG
        printf("fat_16: reading fat table (%d blocks) from %d...\n", (int)blocks_per_fat, (int)fat_base);
#endif
        fat_cache = new uint8_t[blocks_per_fat * bytes_per_block];
        for (int i = 0; i < blocks_per_fat; i++)
            blockDevice->ReadBlock(fat_base + i, 1, fat_cache + i * 512);

        cluster_start = root_dir_sector + (root_dir_entries * DIRENT_SIZE) / bytes_per_block;
#if FAT16_DEBUG
        printf("fat_16: data region start: %d\n", (int)cluster_start);
#endif
    }

    bool Fat16::Exists(const nk::String &path)
    {
        return GetFileMeta(path).type != DirEntryType::Invalid;
    }

    DirEntry Fat16::GetFileMeta(const nk::String &path)
    {
        auto fn = path.Substring(1);

        bool eof = false;
        for (size_t i = 0; i < root_dir_entries; i++)
        {
            auto entry = ParseDirEntry(root_directory + i * DIRENT_SIZE, eof);
            if (eof)
                break;

            if (entry.name == fn)
                return entry;
        }

        return DirEntry::Invalid;
    }

    void Fat16::Read(const DirEntry &entry, size_t size, uint8_t *dst)
    {
        if (size > entry.size)
            size = entry.size;

#if FAT16_DEBUG
        printf("fat_16: reading file from %d\n", (int)entry.location);
#endif

        size_t remaining = size;

        auto clusterSize = blocks_per_alloc * bytes_per_block;
        auto cluster = entry.location;
        do
        {
            LoadCluster(cluster);
            auto offset = size - remaining;

#if FAT16_DEBUG
            printf("fat_16: writing to offset %d\n", (int)offset);
#endif

            if (remaining < clusterSize)
            {
                memcpy(dst + offset, current_cluster, remaining);
                break;
            }
            else
            {
                memcpy(dst + offset, current_cluster, clusterSize);
                remaining -= clusterSize;
            }

        } while ((cluster = ((uint16_t *)fat_cache)[cluster]) != 0xFFFF);
    }

    void Fat16::ListDirectory(const nk::String &path)
    {
        bool eof = false;
        for (size_t i = 0; i < root_dir_entries; i++)
        {
            auto entry = ParseDirEntry(root_directory + i * DIRENT_SIZE, eof);
            if (eof)
                break;

            if (entry.type == DirEntryType::File)
            {
                printf(" [f] %s\n", entry.name.CStr());
            }
            else if (entry.type == DirEntryType::Folder)
            {
                printf(" [d] %s\n", entry.name.CStr());
            }
        }
    }

    DirEntry Fat16::ParseDirEntry(uint8_t *data, bool &eof)
    {
        DirEntry result{};
        nk::Buffer dirbuf(data, 32);
        eof = false;

        char *filename = (char *)dirbuf.At(0);
        if (filename[0] == 0x00) // this is empty
        {
            eof = true;
            return result;
        }

        uint8_t attr = dirbuf.Read<uint8_t>(0x0B);
        uint16_t perms = dirbuf.Read<uint16_t>(0x14);
        uint16_t lastModifiedTime = dirbuf.Read<uint16_t>(0x16);
        uint16_t lastModifiedDate = dirbuf.Read<uint16_t>(0x18);
        uint16_t cluster = dirbuf.Read<uint16_t>(0x1A);
        uint32_t filesize = dirbuf.Read<uint32_t>(0x1C);

        if (attr & 0x02)
            result.type = DirEntryType::Hidden;
        else if (attr & 0x08)
            result.type = DirEntryType::VolumeLabel;
        else if (attr & 0x10)
            result.type = DirEntryType::Folder;
        else if (attr & 0x04)
            result.type = DirEntryType::System;
        else
            result.type = DirEntryType::File;

        result.name = filename;
        result.location = cluster;
        result.size = filesize;

        return result;
    }

    void Fat16::LoadCluster(uint32_t cluster)
    {
#if FAT16_DEBUG
        printf("fat_16: cluster_size = %d\n", blocks_per_alloc);
        printf("fat_16: loading cluster %d\n", (long)cluster);
#endif

        auto hw_cluster = cluster - 2;
        auto start_sector = cluster_start + blocks_per_alloc * hw_cluster;
        for (int i = 0; i < blocks_per_alloc; i++)
        {
#if FAT16_DEBUG
            printf("fat_16: loading %d to %x\n", (long)(start_sector + i), (long)(current_cluster + i * 512));
#endif

            blockDevice->ReadBlock(start_sector + i, 1, current_cluster + i * 512);
        }
    }

}; // namespace FS