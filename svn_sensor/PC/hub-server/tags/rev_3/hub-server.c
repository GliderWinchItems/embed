/* 
 *  A threaded server
 *  by Martin Broadhurst (www.martinbroadhurst.com)
 *  Compile with -pthread
 *
 * This is the main entry point for the hub-server.  It accomplishes the following:
 * -- Initialize the listening socket,
 * -- Call the hs_sock_init() function in file hub-server-sock.c.
 * -- Loop forever accepting new connections and calling hs_sock_new_client(int socket)
 *    in file hub-server-sock.c. 
 *
 * Note that this server hasn't been "serverized" so it's not a proper daemon.
 * For instance, it logs informational & error messages on (unbuffered) stdout.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memset() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

#define PORT    "32123" /* Port to listen on */
#define BACKLOG      5  /* Passed to listen() */

int main(int argc, char *argv[])
{
	char *me = "main";
    int sock;
    pthread_t uart_thread;
    pthread_t sock_thread;
    struct addrinfo hints, *res;
    int reuseaddr = 1; /* True */
    
    setbuf(stdout, NULL);				/* Make stdout unbuffered */

    /* Get the address info */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(NULL, PORT, &hints, &res) != 0)
        hs_fatal(me, "getaddrinfo");

    /* Create the socket */
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1)
        hs_fatal(me, "socket");

    /* Enable the socket to reuse the address */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1)
        hs_fatal(me, "setsockopt");

    /* Bind to the address */
    if (bind(sock, res->ai_addr, res->ai_addrlen) == -1)
        hs_fatal(me, "bind");

    freeaddrinfo(res);

    /* Set listen mode */
    if (listen(sock, BACKLOG) == -1)
        hs_fatal(me, "listen");

    /* Initialize the client input service thread */
    hs_sock_init();

    /* Main loop.  We monitor the listen socket for connections and 
     * create new clients as connections come in.
     */
    while (1) 
    {
        size_t size = sizeof(struct sockaddr_in);
        struct sockaddr_in their_addr;
        int newsock;
        
        newsock = accept(sock, (struct sockaddr*)&their_addr, (socklen_t *)&size);
        if (newsock == -1)
            perror("main -- accept returns -1");
        else
        	hs_sock_new_client(newsock);
    }

    close(sock);

    return 0;
}
