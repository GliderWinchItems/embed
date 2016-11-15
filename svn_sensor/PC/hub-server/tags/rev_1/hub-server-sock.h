/******************************************************************************/
/* hub-server-sock.h -- Interface -- Socket stuff for hub-server.
 */
#ifndef __HUB_SERVER_SOCK_H_
#define __HUB_SERVER_SOCK_H_
#ifdef _CPLUSPLUS
  extern "C" {
#endif

/* Dependencies 
 */

/******************************************************************************/

/* Interface
 */
void hs_sock_init(void);

void hs_sock_new_client(int newsock);

#endif
