// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <myst/eraise.h>
#include <myst/hostfile.h>

#define SYS_DEVICE_CPU_DIR "/sys/devices/system/cpu"
#define SYS_DEVICE_NODE_DIR "/sys/devices/system/node"

int sysfs_setup()
{
    int ret = 0;
    ECHECK(myst_copy_host_directory_recursively(
        SYS_DEVICE_CPU_DIR, SYS_DEVICE_CPU_DIR, true));
    ECHECK(myst_copy_host_directory_recursively(
        SYS_DEVICE_NODE_DIR, SYS_DEVICE_NODE_DIR, true));

done:

    return ret;
}
