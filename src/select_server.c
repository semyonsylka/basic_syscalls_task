#include <unistd.h>
#include <fcntl.h>
// #include <sys/time.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <err.h>
#include <errno.h>
#include <stdlib.h>

#include "us_xfr.h"

#define BACKLOG 5

int main()
{    
    //server
    struct sockaddr_un addr;
    int sfd, cfd;
    ssize_t numRead;
    char buf[BUF_SIZE];

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
    {
        perror("socket()");
        return 1;
    }

    // set socket to be non-blocking
    int non_blocking_enable = 1;
    if (ioctl(sfd, FIONBIO, (char*)&non_blocking_enable) < 0)
        perror("ioctl() call failed");

    if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT)
    {
        perror("remove - %s",SV_SOCK_PATH);
        return 1;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path)-1);

    if (bind(sfd, (struct sockaddr * )&addr, sizeof(struct sockaddr_un)) == -1)
        perror("bind()");

    if (listen(sfd, BACKLOG) == -1)
        perror("listen()");

    // select syscall fds
    fd_set read_fds, master_fds;
    FD_ZERO(&master_fds);
    FD_ZERO(&read_fds);
    int fd_max = sfd;
    FD_SET(fd_max, &master_fds);
    int nready;

    while(1)
    {
        memcpy(&read_fds, &master_fds, sizeof(master_fds));
        nready = select(fd_max+1, &read_fds, NULL, NULL, NULL);
        if (nready == -1)
        {
            perror("select()");
            return 1;
        }
            
        for (int i = 0; i <= fd_max && nready > 0; i++)
        {
            if (FD_ISSET(i, &read_fds))
            {
                // nready--;
                // new connection
                if (i == sfd)
                {
                    printf("try to accept() new connection\n");
                    
                    cfd = accept(sfd, NULL, NULL);
                    if (cfd == -1)
                    {
                        perror("accept()");
                        return 1;
                    }
                    FD_SET(cfd, &master_fds);
                    if (cfd > fd_max)
                        fd_max = cfd;
                }
                else
                {
                    printf("try to read connected\n");
                    while ((numRead = read(i, buf, BUF_SIZE)) > 0)
                    {
                        if (write(STDOUT_FILENO, buf, numRead) != numRead)
                        {
                            perror("write from client");                        
                        }
                        if (write(i, buf, numRead) != numRead)
                        {
                            perror("write write to client");                            
                        }
                    }
                    if (numRead == -1)
                    {
                        perror("read client data error");                    
                    }
                    //close
                    if (close(i) == -1)
                    {
                        perror("close client connection");                    
                    }
                    FD_CLR(i, &master_fds);
                    if (i == fd_max)
                    {
                        while (!FD_ISSET(fd_max, &master_fds))
                            fd_max--;
                    }
                }
                
            }
        }
        // insert conditional return?        
    }
        
    return 0;
}