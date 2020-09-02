#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <errno.h>
#include <stdlib.h>

#include "us_xfr.h"

int main()
{            
    int pid = fork();
    char sended[BUF_SIZE];
    char received[BUF_SIZE];
    sprintf(sended, "message from client pid=%d\n", pid);
    struct sockaddr_un addr;
    int sfd;
    ssize_t numRead;
    
    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
    {
        perror("client socket() err");
        return 1;
    }
        
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path)-1);

    if (connect(sfd, (struct sockaddr * )&addr, sizeof(struct sockaddr_un)) == -1)
    {
        perror("client connect() error");
        return 1;
    }

    printf("try to write() to socket\n");
    if (write(sfd, sended, strlen(sended)) != strlen(sended))
    {
        perror("client write() error");
        return 1;
    }
    //read
    int read_bytes_cnt = read(sfd, received, sizeof(received));
    if (read_bytes_cnt < 0)
    {
        perror("Receive from server error");
        return 1;
    }
    printf("Server response: %s",received);

    return 0;
}