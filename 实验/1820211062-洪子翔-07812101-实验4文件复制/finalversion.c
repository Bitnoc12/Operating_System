#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <utime.h>
#include <dirent.h>

#define READSIZE 1024
#define LinkPathLen 1024

void CopyFile(char *src, char *dest)
{
    struct stat statbuf;
    struct utimbuf uTime;

    int destFd;
    int srcFd;
    int size = 0;
    char buffer[READSIZE];

    memset(buffer, 0, sizeof(buffer));

    stat(src, &statbuf);
    destFd = creat(dest, statbuf.st_mode);

    if ((srcFd = open(src, O_RDONLY)) < 0)
    {
        printf("打开文件:%s 出现错误!\n", src);
        exit(-1);
    }

    while ((size = read(srcFd, buffer, READSIZE)) > 0)
    {
        write(destFd, buffer, size);
    }

    uTime.actime = statbuf.st_atime;
    uTime.modtime = statbuf.st_mtime;
    utime(dest, &uTime);

    close(srcFd);
    close(destFd);
}

void CopyLinkFile(char *LinkPath, char *dest)
{
    struct stat statbuf;
    struct timeval tv[2];
    char path_buf[LinkPathLen];

    memset(path_buf, 0, sizeof(path_buf));

    lstat(LinkPath, &statbuf);
    readlink(LinkPath, path_buf, LinkPathLen);

    if (symlink(path_buf, dest) == -1)
    {
        printf("建立软连接失败!\n");
        _exit(-1);
    }

    printf("软连接文件复制成功\n");

    chmod(dest, statbuf.st_mode);

    tv[0].tv_sec = statbuf.st_atime;
    tv[0].tv_usec = 0;
    tv[1].tv_sec = statbuf.st_mtime;
    tv[1].tv_usec = 0;

    lutimes(dest, tv);
}

void MyCopy(char *src, char *dest)
{
    struct stat statbuf;
    struct stat copybuf;
    struct utimbuf uTime;
    struct dirent *entry = NULL;
    DIR *dptr1 = NULL;
    char src_path[128], dest_path[128];

    strcpy(src_path, src);
    strcpy(dest_path, dest);

    dptr1 = opendir(src_path);

    if (dptr1 != NULL)
    {
        while ((entry = readdir(dptr1)) != NULL)
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            strcpy(dest_path, dest);
            strcpy(src_path, src);
            strcat(src_path, "/");
            strcat(dest_path, "/");
            strcat(src_path, entry->d_name);
            strcat(dest_path, entry->d_name);
            lstat(src_path, &statbuf);

            if (S_ISDIR(statbuf.st_mode))
            {
                printf("Directory: %s, 正在拷贝...\n", src_path);
                stat(src_path, &copybuf);
                mkdir(dest_path, copybuf.st_mode);
                MyCopy(src_path, dest_path);
                uTime.actime = copybuf.st_atime;
                uTime.modtime = copybuf.st_mtime;
                utime(dest_path, &uTime);
                printf("Directory: %s, 拷贝完成\n", src_path);
            }
            else if (S_ISLNK(statbuf.st_mode))
            {
                printf("LinkFileName: %s, 正在拷贝...\n", src_path);
                CopyLinkFile(src_path, dest_path);
            }
            else
            {
                printf("FileName: %s, 正在拷贝...\n", src_path);
                CopyFile(src_path, dest_path);
                printf("FileName: %s, 拷贝完成\n", src_path);
            }
        }

        closedir(dptr1);
    }
    else
    {
        // 如果源是文件而不是目录，则直接复制文件
        printf("FileName: %s, 正在拷贝...\n", src);
        CopyFile(src, dest);
        printf("FileName: %s, 拷贝完成\n", src);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("复制语句格式出现错误!\n");
        printf("格式为 ./mycopy3 src dest\n");
        exit(-1);
    }

    struct stat statbuf;
    struct utimbuf uTime;
    DIR *dptr2 = NULL;

    if (stat(argv[1], &statbuf) == 0)
    {
        if (S_ISREG(statbuf.st_mode))
        {
            // 如果源是文件而不是目录，则直接复制文件
            printf("FileName: %s, 正在拷贝...\n", argv[1]);
            CopyFile(argv[1], argv[2]);
            printf("FileName: %s, 拷贝完成\n", argv[1]);
        }
        else if (S_ISDIR(statbuf.st_mode))
        {
            // 如果源是目录，则创建目录，然后递归复制目录内容
            if ((dptr2 = opendir(argv[2])) == NULL)
            {
                uTime.actime = statbuf.st_atime;
                uTime.modtime = statbuf.st_mtime;

                if (mkdir(argv[2], statbuf.st_mode) < 0)
                {
                    printf("创建目录失败\n");
                    exit(-1);
                }

                MyCopy(argv[1], argv[2]);
                utime(argv[2], &uTime);
                printf("目录复制完成\n");
            }
            else
            {
                printf("目标目录已存在\n");
                exit(-1);
            }
        }
        else
        {
            printf("源既不是文件也不是目录\n");
            exit(-1);
        }
    }
    else
    {
        printf("获取文件信息失败\n");
        exit(-1);
    }

    if (dptr2 != NULL)
    {
        closedir(dptr2);
    }

    return 0;
}
