/**************************************************************************
 * CS52700 - Software Security - Spring 2016
 * Programming Project:  API for ONline fIle eDiting (ONID)
 *
 * This file contains a simple server that uses the ONID library.
 **************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "onid-api.h"

#define SERVER "127.0.0.1"
#define PORT   0x4e14  /* 0xN14 */

int main(int argc, char *argv[]) {

  struct host_t *client;
  struct cmd_t  *cmd;  
  pid_t pid;
  int r;


  if( serv_bind(SERVER, PORT) < 0 ) {
    printf("Error! Cannot bind to address/port.\n");
    return -1;
  }

  while( 1 ) {

    if( !(client = serv_wait()) ) {
      printf("Error! Cannot accept client.\n");
      return -1;
    }

    if( (pid = fork()) < 0){
      printf("Error! Fork failed.\n");
      return -1;
    }
    
    if( pid == 0 ) {
      printf("Child Process:\n");

      while( 1 ) {
        if( !(cmd = serv_waitcmd(client)) ) {
          printf("Error! Cannot get command from client.\n");
          return -1;
        }
        
        r = serv_proccmd(client, cmd);

        if( r == 1 ) break;
        else if ( r < 0 ) {
          printf("Error! Cannot process command.\n");
          return -1;
        }
      }
    }
  }
}