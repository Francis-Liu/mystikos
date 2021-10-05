// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <myst/eraise.h>
#include <myst/file.h>
#include <myst/fs.h>
#include <myst/hostfile.h>
#include <myst/mount.h>
#include <myst/printf.h>
#include <myst/ramfs.h>

#define SYS_DEVICE_CPU_DIR "/sys/devices/system/cpu"
#define SYS_DEVICE_NODE_DIR "/sys/devices/system/node"

static myst_fs_t* _sysfs;

int sysfs_setup()
{
    int ret = 0;

    if (myst_init_ramfs(myst_mount_resolve, &_sysfs) != 0)
    {
        myst_eprintf("failed initialize the sys file system\n");
        ERAISE(-EINVAL);
    }

    if (myst_mkdirhier("/sys", 777) != 0)
    {
        myst_eprintf("cannot create mount point for sysfs\n");
        ERAISE(-EINVAL);
    }

    if (myst_mount(_sysfs, "/", "/sys", false) != 0)
    {
        myst_eprintf("cannot mount sys file system\n");
        ERAISE(-EINVAL);
    }

    ECHECK(myst_copy_host_directory_recursively(
        SYS_DEVICE_CPU_DIR, SYS_DEVICE_CPU_DIR, true));
    ECHECK(myst_copy_host_directory_recursively(
        SYS_DEVICE_NODE_DIR, SYS_DEVICE_NODE_DIR, true));

done:

    return ret;
}

int sysfs_teardown()
{
    if ((*_sysfs->fs_release)(_sysfs) != 0)
    {
        myst_eprintf("failed to release sysfs\n");
        return -1;
    }

    return 0;
}
