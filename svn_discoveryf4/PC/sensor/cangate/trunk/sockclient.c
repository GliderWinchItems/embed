/******************************************************************************
* File Name          : sockclient.c
* Date First Issued  : 12-18-2013
* Board              : 
* Description        : socket client
*******************************************************************************/
#include "sockclient.h"

/* **************************************************************************************
 * int sockclient_connect(char *ip, int port);
 * @brief	: Generate a CAN test msg that goes to the CAN bus
 * @return	: Greater or equal zero is success--
 *		:    file descriptor
 *		: Negative--error
 * ************************************************************************************** */
int sockclient_connect(char *ip, int port)
{
    int sockfd = 0;

    struct sockaddr_in serv_addr; 

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return -1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); 

    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return -1;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return -1;
    } 

    return sockfd;
}
/*
    while ( (n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
    {
        recvBuff[n] = 0;
        if(fputs(recvBuff, stdout) == EOF)
        {
            printf("\n Error : Fputs error\n");
        }
    } 

    if(n < 0)
    {
        printf("\n Read error \n");
    } 
*/
