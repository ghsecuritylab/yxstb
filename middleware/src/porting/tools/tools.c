
#include <sys/sysinfo.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

static unsigned long GetFreeDiskBytes(const char * path)
{
        struct statfs   buf;
        if(statfs(path, &buf) != 0)
        {
                perror("statfs");
                return 0;
        }
        return (unsigned long)buf.f_bfree * 4 * 1024;
}

unsigned long GetFreeVarSize(void)
{
        return GetFreeDiskBytes("/var");
}

int IsFileExists(const char * filename)
{
    if (filename == NULL)
        return 0;
    if (access(filename, F_OK) == 0)
        return 1;
    return 0;
}
