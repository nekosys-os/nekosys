#include <kernel/fs/vfs.h>
#include <kernel/kdebug.h>

namespace FS
{

    DEFINE_SINGLETON(VirtualFileSystem)

    VirtualFileSystem::VirtualFileSystem()
    {
    }

    void VirtualFileSystem::Mount(const nk::String &path, FileSystem *fs)
    {
        mounts.Add(new MountPoint(path, fs));
    }

    void VirtualFileSystem::Unmount(const nk::String &path)
    {
        for (size_t i = 0; i < mounts.Size(); i++)
        {
            if (mounts.At(i)->path == path)
            {
                auto mount = mounts.At(i);
                mounts.Remove(i);
                delete mount;
                break;
            }
        }
    }

    bool VirtualFileSystem::Exists(const nk::String &path)
    {
        auto mountPoint = FindMountPoint(path);
        auto subPath = path.Substring(mountPoint->path.Length());
        return mountPoint->fs->Exists(path);
    }

    DirEntry VirtualFileSystem::GetFileMeta(const nk::String &path)
    {
        if (path == "/")
        {
            return {"/", DirEntryType::Folder, 0, 0};
        }

        auto mountPoint = FindMountPoint(path);
        if (mountPoint == nullptr)
        {
            kdbg("vfs: Mount point not found for %s\n", path.CStr());
            return DirEntry::Invalid;
        }
#if VFS_DEBUG
        kdbg("vfs: get_file_meta %s\n", mountPoint->path.CStr());
#endif
        return mountPoint->fs->GetFileMeta(GetRelativePath(mountPoint, path));
    }

    uint32_t VirtualFileSystem::Open(const nk::String &path)
    {
        auto mountPoint = FindMountPoint(path);
        if (mountPoint == nullptr)
        {
            kdbg("vfs: Mount point not found for %s\n", path.CStr());
            return 0;
        }

        auto entry = mountPoint->fs->GetFileMeta(GetRelativePath(mountPoint, path));
        if (entry.type == DirEntryType::Invalid)
        {
            return 0;
        }

        FileHandle *handle = new FileHandle(++idCounter, mountPoint->fs, entry);
        fileHandles.Add(handle);
        return handle->id;
    }

    void VirtualFileSystem::Read(uint32_t fileHandle, size_t offset, size_t size, uint8_t *dst)
    {
        if (fileHandle == 0)
            return;

        for (size_t i = 0; i < fileHandles.Size(); i++)
        {
            auto handle = fileHandles.At(i);
            if (handle->id == fileHandle)
            {
                handle->fs->Read(handle->entry, offset, size, dst);
                break;
            }
        }
    }

    void VirtualFileSystem::Close(uint32_t fileHandle)
    {
        if (fileHandle == 0)
            return;

        for (size_t i = 0; i < fileHandles.Size(); i++)
        {
            auto handle = fileHandles.At(i);
            if (handle->id == fileHandle)
            {
                fileHandles.Remove(i);
                delete handle;
                return;
            }
        }
    }

    nk::Vector<DirEntry> VirtualFileSystem::ListDirectory(const nk::String &path)
    {
        auto mountPoint = FindMountPoint(path);
        if (mountPoint == nullptr)
        {
            kdbg("vfs: Mount point not found for %s\n", path.CStr());
            return {};
        }
        return mountPoint->fs->ListDirectory(GetRelativePath(mountPoint, path));
    }

    nk::String VirtualFileSystem::GetRelativePath(MountPoint *mountPoint, const nk::String &absolutePath)
    {
        nk::String base = "/";
        nk::String relative = absolutePath.Substring(mountPoint->path.Length());
        return base.Append(relative);
    }

    MountPoint *VirtualFileSystem::FindMountPoint(const nk::String &path)
    {
        for (size_t i = 0; i < mounts.Size(); i++)
        {
            auto mount = mounts.At(i);
            if (path.StartsWith(mount->path))
                return mount;
        }
        return nullptr;
    }

} // namespace FS