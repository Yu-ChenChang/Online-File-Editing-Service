/**************************************************************************
 * CS52700 - Software Security - Spring 2016
 * Programming Project:  API for ONline fIle eDiting (ONID)
 *
 * Feel free to edit any struct defnition or add any required structs as
 * long as you keep the names of the existing structs.
 **************************************************************************/

#define BLOCK_SIZE  	512   	/* block size of the files */
#define MAX_FILE_SIZE 	1048576 /* maximum possible file size */
#define FILENAME_SIZE	20		/* maximum possible filename size */
#define MAX_CONNECTION  100		/* maximum possible connection from client */

enum clnt_cmd {OPEN=1, READ, WRITE, CLOSE, EDIT, QUIT};
enum serv_cmd {YOURTPES=1 /* fill in here. */ };
enum proc_status {FAILURE=-1,SUCCESS=0 /* fill in here. */ };
enum close_type {DONTSAVE=1, SAVE};

struct file_content{
	char* content;
	unsigned int size;			/* size of used content */
	int flag;					/* check whether this entry is initialized */
	int fd;						/* file descriptor of this entry */
};

struct host_t {
  int sockd;                    /* socket descriptor */
  char addr[20];                /* ip address and port of client */
                                /* e.g. "11.22.33.44:9999" */
  size_t bytes_rcv, bytes_snd;  /* number of send and received bytes */

  /* feel free to add more fields here */
};

struct cmd_t {
  int type;                     /* command type */
  int res;                      /* result (successful or not) */

  /* feel free to add more fields here */
};

struct blk_t {                  /* blocks to send to server */
  int  meta1;                   /* you can store any metadata here */
  int  meta2;
  char data[ BLOCK_SIZE ];      /* actual data */
};

struct file_t {                 /* open file information */
  int fd;                       /* file descriptor */
  int sockd;					/* socket descriptor for connection */
  /* Fill in here. */
};

