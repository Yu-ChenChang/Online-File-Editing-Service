/**************************************************************************
 * CS52700 - Software Security - Spring 2016
 * Programming Project:  API for ONline fIle eDiting (ONID)
 *
 * This file contains a simple client that uses the ONID library.
 **************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>

#include "onid-datatypes.h"
#include "onid-api.h"

#define SERVER "127.0.0.1"
#define PORT   0x4e14  /* 0xN14 */

int main(int argc, char *argv[]) {
  
  if (argc != 2) {
    printf("Usage: %s <1|2|3> (for test case 1, 2, or 3)\n", argv[0]);
    return -1;
  }

  int testcase = atoi(argv[1]);
  int i, j;
  struct host_t *server = clnt_conn(SERVER, PORT);
  struct file_t *file;
  char *p;


  if( !server ) {
    printf("Error! Cannot connect to server.\n");
    return -1;
  }

  switch (testcase) {
    case 1:
      /* Open file A, write static string, close file. */
      /* Open file A, read data into 2nd buffer and verify correctness. */
      
      /* Assume A.txt contains 1024 A's */
      if( !(file = clnt_open(server, "A.txt")) ) {
        printf("Error! Cannot open file!\n");
        return -1;
      }

      if( clnt_write(file, 0, 'c') < 0 ||
          clnt_write(file, 1, 's') < 0 ||
          clnt_write(file, 2, '5') < 0 ||
          clnt_write(file, 3, '2') < 0 ||
          clnt_write(file, 4, '7') < 0) {
        printf("Error writing data to file!\n");
        return -1;
      }

      clnt_close(server, file, SAVE);
         
      if( !(file = clnt_open(server, "A.txt")) ) {
        printf("Error! Cannot open file!\n");
        return -1;
      }
 
      p = (char*)malloc(16*sizeof(char));
      if( (clnt_read(file, p, 0, 16) != 4) ) {
        printf("Error! Null pointer from read block!\n");
        return -1;
      }
      
      if( p[0] == 'c' && p[1] == 's' && p[2] == '5' && p[3] == '2' && 
          p[4] == '7' && p[5] == 'A') {
        for(i=6; i<16; ++i) {
          if(p[i] != 'A') {
            printf("Error reading expected 'A's in #%d\n", testcase);
            break;
          }
        }
        if( i == BLOCK_SIZE )
          printf("Testcase #%d correct!\n", testcase);
      }
      else printf("Testcase #%d Wrong.\n", testcase);
      
      clnt_close(server, file, DONTSAVE);   
      clnt_term(server);
      free(p);
      break;


    case 2:
      /* Open file A, write data to it, close it. */
      /* Open file A, read data into buffer, close it. */
      /* Create new file B, write buffer to file B, close it. */
      /* Open file B, verify correctness of written data. */
    
      /* Assume B.txt contain 20 B's */      
      if( !(file = clnt_open(server, "B.txt")) ) {
        printf("Error! Cannot open file!\n");
        return -1;
      }

      p = (char*)malloc(20*sizeof(char));
      if( (clnt_read(file, p, 3, 20) != 17) ) {
        printf("Error: wrong size of B!\n");
        return -1;
      }
         
      for(i=0; i<17; ++i) {
        if(p[i] != 'B') {
          printf("Testcase #%d wrong.\n", testcase);
          break;
        }
      }

      clnt_close(server, file, DONTSAVE);   
      free(p);
 
      /* Assume C.txt doesn't exists */
      if( !(file = clnt_open(server, "C.txt")) ) {
        printf("Error! Cannot open file!\n");
        return -1;
      }
      
      p = (char*)malloc(256*sizeof(char));
      if( (clnt_read(file, p, 0, 10) != 0) ) {
        printf("Error read unexpected data!\n");
        return -1;
      }
      
      for(i=0; i<1337; ++i) {
        if( clnt_write(file, i, 'K') < 0 ) {
          printf("Error! Cannot write to file!\n");
          return -1;
        }
      }

      clnt_close(server, file, SAVE);   

      if( !(file = clnt_open(server, "C.txt")) ) {
        printf("Error! Cannot open file!\n");
        return -1;
      }

      if( (clnt_read(file, p, 0, 256) != 256) ) {
        printf("Error unepxected read!\n");
        return -1;
      }
      
      for(i=0; i<256; ++i) {
        if(p[i] != 'K') {
          printf("Testcase #%d Wrong.\n", testcase);
          break;
        }   
      }
 
      if( (clnt_read(file, p, 512, 256) != 256) ) {
        printf("Error unexpected read!\n");
        return -1;
      }
      
      for(i=0; i<256; ++i) {
        if(p[i] != 'K') {
          printf("Testcase #%d Wrong.\n", testcase);
          break;
        }   
      }
  
      clnt_close(server, file, DONTSAVE);
      clnt_term(server);
      free(p);
      printf("Testcase #%d Correct!\n", testcase);
      break;


    case 3:
      /* Longer testcase that switches between OPEN/READ/WRITE/EDIT */
    
      /* D contains 60 D's */    
      if( !(file = clnt_open(server, "D.txt")) ) {
          printf("Error! Cannot open file!\n");
          return -1;
        }

      /* should return 1st block */
      p = (char*)malloc(200*sizeof(char));
      if( (clnt_read(file, p, 10, 200) != 50) ) {
        printf("Error, unexpected read!\n");
        return -1;
      }
      
      
      for(i=0; i<999999; ++i) {
        if( clnt_write(file, i, 'X') < 0 ) {
          printf("Error! Cannot write to file!\n");
          return -1;
        }
      }

      clnt_close(server, file, DONTSAVE);  

      /* D still contains 60 D's */
      if( !(file = clnt_open(server, "D.txt")) ) {
          printf("Error! Cannot open file!\n");
          return -1;
      }

      /* should return 1st block */
      if( (clnt_read(file, p, 50, 200) != 10) ) {
        printf("Error, unexpected read!\n");
        return -1;
      }
      
      for(i=0; i<10; ++i)
        if(p[i] != 'D') {
          printf("Testcase #%d Wrong.\n", testcase);
          break;
        }   
      
      clnt_close(server, file, DONTSAVE);

      if( !(file = clnt_open(server, "D.txt")) ) {
        printf("Error! Cannot open file!\n");
        return -1;
      }

      /* should return 1st block */
      if( clnt_read(file, p, 0, 200) != 60 ) {
        printf("Error, unexpected read!\n");
        return -1;
      }
      
      
      for(i=0; i<999999; ++i)
        if( clnt_write(file, i, 'X') < 0 )  {
          printf("Error! Cannot open file!\n");
          return -1;
        }
      
      clnt_close(server, file, SAVE);  
      
      
      /* D contains Xs */
      if( !(file = clnt_open(server, "D.txt")) ) {
        printf("Error! Cannot open file!\n");
        return -1;
      }
      
      for(j=0; j<20; j++) {
        if( clnt_read(file, p, j*200, 200) != 200) {
          printf("Error, unexpected read!\n");
          return -1;
        }
        for(i=0; i<200; ++i) {
          if(p[i] != 'X') {
            printf("Testcase #%d Wrong.\n", testcase);
            return -1;
          } 
        }  
      }
      clnt_close(server, file, DONTSAVE);  
      clnt_term(server);
      free(p);
      printf("Testcase #%d Correct!\n", testcase);
      
      break;
    
    default:
      printf("Illegal test case.\n");
  }
}
