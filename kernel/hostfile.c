#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <myst/buf.h>
#include <myst/eraise.h>
#include <myst/file.h>
#include <myst/hostfile.h>
#include <myst/paths.h>
#include <myst/printf.h>
#include <myst/syscall.h>
#include <myst/tcall.h>
#include <myst/trace.h>
#include <myst/uid_gid.h>

struct linux_dirent64
{
    unsigned long d_ino;     /* 64-bit inode number */
    unsigned long d_off;     /* 64-bit offset to next structure */
    unsigned short d_reclen; /* Size of this dirent */
    unsigned char d_type;    /* File type */
    char d_name[];           /* Filename (null-terminated) */
};

static int _get_host_uid_gid(uid_t* host_uid, gid_t* host_gid)
{
    int ret = 0;

    ECHECK(myst_enc_uid_to_host(myst_syscall_geteuid(), host_uid));
    ECHECK(myst_enc_gid_to_host(myst_syscall_getegid(), host_gid));

done:
    return ret;
}

static int _host_open(const char* pathname, int flags, mode_t mode)
{
    int ret = 0;
    uid_t uid;
    gid_t gid;

    ECHECK(_get_host_uid_gid(&uid, &gid));

    long params[6] = {
        (long)pathname, (long)flags, (long)mode, (long)uid, (long)gid};

    ECHECK(ret = myst_tcall(SYS_open, params));

done:
    return ret;
}

static int _host_close(int fd)
{
    long params[6] = {(long)fd};
    return (int)myst_tcall(SYS_close, params);
}

static ssize_t _host_read(int fd, void* buf, size_t count)
{
    long params[6] = {(long)fd, (long)buf, (long)count};
    return (ssize_t)myst_tcall(SYS_read, params);
}

static ssize_t _host_readlink(const char* pathname, void* buf, size_t bufsiz)
{
    long params[6] = {(long)pathname, (long)buf, (long)bufsiz};
    return (ssize_t)myst_tcall(SYS_readlink, params);
}

static ssize_t _host_getdents64(int fd, void* dirp, size_t count)
{
    long params[6] = {(long)fd, (long)dirp, (long)count};
    return (long)myst_tcall(SYS_getdents64, params);
}

int myst_load_host_file(const char* path, void** data_out, size_t* size_out)
{
    int ret = 0;
    int fd = -1;
    myst_buf_t buf = MYST_BUF_INITIALIZER;
    const size_t min_buf_size = 4096;
    struct locals
    {
        char buf[BUFSIZ];
    };
    struct locals* locals = NULL;

    if (data_out)
        *data_out = NULL;

    if (size_out)
        *size_out = 0;

    if (!path || !data_out || !size_out)
        ERAISE(-EINVAL);

    if (!(locals = calloc(1, sizeof(struct locals))))
        ERAISE(-ENOMEM);

    if (myst_buf_reserve(&buf, min_buf_size) != 0)
        ERAISE(-ENOMEM);

    ECHECK(fd = _host_open(path, O_RDONLY, 0));

    for (;;)
    {
        ssize_t n = _host_read(fd, locals->buf, sizeof(locals->buf));
        ECHECK(n);

        if (n == 0)
            break;

        if (myst_buf_append(&buf, locals->buf, n) != 0)
            ERAISE(-ENOMEM);
    }

    /* append a zero-terminator character */
    {
        char c = '\0';

        if (myst_buf_append(&buf, &c, 1) != 0)
            ERAISE(-ENOMEM);
    }

    *data_out = buf.data;
    buf.data = NULL;
    *size_out = buf.size - 1; /* don't count the zero terminator */

done:

    if (buf.data)
        free(buf.data);

    if (fd >= 0)
        _host_close(fd);

    if (locals)
        free(locals);

    return ret;
}

int myst_copy_host_directory_recursively(
    const char* src_dir,
    const char* dst_dir,
    bool ignore_errors)
{
    int ret = 0;
    int fd = -1;
    int new_fd = -1;
    long nread;
    char d_type;
    void* buf = NULL;
    size_t buf_size;
    struct linux_dirent64* d;
    struct locals
    {
        char buf[BUFSIZ];
        char symlink_target[BUFSIZ];
        char src_path[PATH_MAX];
        char dst_path[PATH_MAX];
    };
    struct locals* locals = NULL;

    if (!src_dir || !dst_dir)
        ERAISE(-EINVAL);

    ECHECK(fd = _host_open(src_dir, O_RDONLY | O_DIRECTORY, 0));

    if (!(locals = calloc(1, sizeof(struct locals))))
        ERAISE(-ENOMEM);

    ECHECK(myst_mkdirhier(dst_dir, 0755));

    for (;;)
    {
        ECHECK(nread = _host_getdents64(fd, locals->buf, BUFSIZ));

        if (nread == 0)
            break;

        for (long bpos = 0; bpos < nread;)
        {
            d = (struct linux_dirent64*)(locals->buf + bpos);
            bpos += d->d_reclen;

            if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
                continue;

            d_type = d->d_type;
            if (d_type != DT_DIR && d_type != DT_REG && d_type != DT_LNK)
                continue;

            ECHECK(myst_make_path(
                locals->src_path,
                sizeof(locals->src_path),
                src_dir,
                d->d_name));
            ECHECK(myst_make_path(
                locals->dst_path,
                sizeof(locals->dst_path),
                dst_dir,
                d->d_name));
            if (d_type == DT_DIR)
            {
                ECHECK(myst_copy_host_directory_recursively(
                    locals->src_path, locals->dst_path, ignore_errors));
            }
            else if (d_type == DT_REG)
            {
                if (myst_load_host_file(locals->src_path, &buf, &buf_size) < 0)
                {
                    if (ignore_errors)
                        continue;
                    ERAISE(-EINVAL);
                }
                ECHECK(new_fd = creat(locals->dst_path, 0644));
                ECHECK(myst_write_file_fd(new_fd, buf, buf_size));

                close(new_fd);
                new_fd = -1;

                if (buf)
                {
                    free(buf);
                    buf = NULL;
                }
            }
            else if (d_type == DT_LNK)
            {
                ssize_t n = _host_readlink(
                    locals->src_path,
                    locals->symlink_target,
                    sizeof(locals->symlink_target));
                ECHECK(n);
                if (n == sizeof(locals->symlink_target))
                {
                    myst_eprintf(
                        "host symlink is too long (truncated): %s\n",
                        locals->src_path);
                    ERAISE(-EINVAL);
                }

                locals->symlink_target[n] = '\0';
                ECHECK(symlink(locals->symlink_target, locals->dst_path));
            }
        }
    }

done:

    if (fd != -1)
        close(fd);

    if (locals)
        free(locals);

    if (new_fd != -1)
        close(new_fd);

    if (buf)
        free(buf);

    return ret;
}
