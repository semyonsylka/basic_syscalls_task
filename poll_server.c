#include <unistd.h>
#include <fcntl.h>
// #include <sys/time.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <err.h>
#include <errno.h>
#include <stdlib.h>

#include "us_xfr.h"

#define BACKLOG 5

#define SERVER_BREAK -1
#define SERVER_ENABLE 0

int main()
{
    struct sockaddr_un addr;
    int sfd, cfd;
    ssize_t numRead;
    char buf[BUF_SIZE];

    // poll descriptors array 
    int fds_count = 10;
    struct pollfd fds[fds_count];
    int nfds = 1, curent_size = 0;

    int retval;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
    {
        perror("socket()");
        return 1;
    }

    // set socket to be non-blocking
    int non_blocking_enable = 1;
    retval = ioctl(sfd, FIONBIO, (char*)&non_blocking_enable);
    if (retval < 0)
        perror("ioctl() call failed");

    if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT)
    {
        perror("remove - %s",SV_SOCK_PATH);
        return 1;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path)-1);

    retval = bind(sfd, (struct sockaddr * )&addr, sizeof(struct sockaddr_un));
    if (retval == -1)
        perror("bind()");

    retval = listen(sfd, BACKLOG);     
    if (retval == -1)
        perror("listen()");
    
    // init pollfd structs array
    memset(fds, 0, sizeof(fds));

    // set up initial socket
    fds[0].fd = sfd;
    fds[0].events = POLLIN;
    
    //set up the timeout for 1 minute
    int timeout = 1 * 60 * 1000;

    int status = SERVER_ENABLE;
    // fds iterator, close connection flag
    int i, close_connection = 0, conn_closed = 0;
    while(status != SERVER_BREAK)
    {
        printf("waiting on poll()\n");   
        retval =  poll(fds, nfds, timeout);    
        if (retval == -1)
        {
            perror("poll()");
            return 1;
        }
            
        if (retval == 0)
        {
            printf("poll() timed out\n");
            break;
        }

        curent_size = nfds;
        for (i = 0; i < curent_size; i++)
        {
            if (fds[i].revents == 0)
                continue;
            
            if (fds[i].revents != POLLIN)
            {
                printf("error! revents = %d\n", fds[i].revents);
                status = SERVER_BREAK;
                break;
            }    

            if (fds[i].fd == sfd)
            {
                // accept new connection
                do
                {
                    cfd = accept(sfd, NULL, NULL);
                    if (cfd < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            perror("accept() failed");
                            status = SERVER_BREAK;
                        }
                        break;
                    }

                    printf("new connection %d\n", cfd);
                    fds[nfds].fd = cfd;
                    fds[nfds].events = POLLIN;
                    nfds++;

                } while (cfd != -1);                
            }
            else
            {
                printf("read from %d\n",fds[i].fd);
                while ((numRead = read(fds[i].fd, buf, BUF_SIZE)) > 0)
                {
                    if (write(STDOUT_FILENO, buf, numRead) != numRead)
                    {
                        perror("write from client");
                        return -1;
                    }
                    if (write(fds[i].fd, buf, numRead) != numRead)
                    {
                        perror("write to client");
                        return -1;
                    }
                }
                if (numRead == -1)
                {
                    perror("fatal read client data error");
                    return -1;
                }
                if (numRead == 0)
                {
                    printf("connection closed\n");                    
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    conn_closed = 1;                
                }
            }                    
        }
        if (conn_closed)
        {
            conn_closed = 0;
            for (int ii = 0; ii < nfds; ii++)
            {
                if (fds[ii].fd == -1)
                {
                    for (int jj = ii; jj <= nfds; jj++)
                        fds[jj] = fds[jj+1];
                    ii--;
                    nfds--;    
                }
            }
        }
    }        
    return 0;
}