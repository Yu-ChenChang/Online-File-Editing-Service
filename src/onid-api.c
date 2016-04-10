/**************************************************************************
 * CS52700 - Software Security - Spring 2016
 * Programming Project:  API for ONline fIle eDiting (ONID)
 *
 * Feel free to add any code that you want as long as you implement the
 * functions in onid-api.h.
 **************************************************************************/

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

#include "onid-datatypes.h"
#include "onid-api.h"

#define handle_error(msg) \
		   do { perror(msg); } while (0)
#define DEBUG
/* For server */
int sfd;
struct sockaddr_in serv_addr; 
char serv_data[ BLOCK_SIZE ];
/***************************************************************************
 *                                                                         *
 *                             USER FUNCTIONS                              *
 *                                                                         *
 ***************************************************************************/

/* write any other functions here. */
struct host_t *convertToHost_t(struct sockaddr_in* soaddr, int sfd, size_t bytes_rcv, size_t bytes_snd){
	struct host_t *ht = (struct host_t*)malloc(sizeof(struct host_t));
	ht->sockd = sfd;
	char ip[20] = "";
	if(inet_ntop(AF_INET,&(soaddr->sin_addr),ip,INET_ADDRSTRLEN)==NULL)
	{
		handle_error("Error: inet_ntop! ");
		return NULL;
	}
	snprintf(ht->addr,sizeof(ht->addr),"%s:%u",ip,ntohs(soaddr->sin_port));
#ifdef DEBUG
	printf("ht->addr: %s\n",ht->addr);
#endif
	ht->bytes_rcv = bytes_rcv;
	ht->bytes_snd = bytes_snd;

	return ht;
}

struct sockaddr_in *convertToSockaddr_in(struct host_t* ht){
	if(ht==NULL){
		handle_error("struct host_t* ht is NULL in convertToSockaddr_in ");
		return NULL;
	}
	struct sockaddr_in *sock_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	char ip[20] = "";
	char port_s[20] = "";
	uint16_t port = 0;
	char* e = strchr(ht->addr,':');
	int index = (int)(e-ht->addr);
	if(index<0 || index >=20){
		handle_error("Error: index is not within 0-20 ");
		return NULL;
	}
#ifdef DEBUG
	printf("index of ':' is %d\n",index);
#endif
	strncpy(ip,ht->addr,index);
#ifdef DEBUG
	printf("extracted ip is %s\n",ip);
#endif
	strncpy(port_s,ht->addr+index+1,20-index-1);
	port = (uint16_t) atoi(port_s);
#ifdef DEBUG
	printf("extracted port is %d\n",port);
#endif

	sock_addr->sin_family = AF_INET;
	sock_addr->sin_port = htons(port);

	if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
	{
		handle_error("Error: inet_pton error! ");
		return NULL;
	}
	return sock_addr;
}
/**
 * prnt_stat(): Print all information and statistics of a given host
 * Arguments: h = a host_t struct
 * Return Value: None.
 */
void prnt_stat(struct host_t *h) {
  /* print tab separated information, i.e sockd addr  bytes_rcv bytes_snd */
  /* Fill in here. */
	/* print tab separated information, i.e sockd addr  bytes_rcv bytes_snd */
	/* Fill in here. */
	if(h==NULL){
		handle_error("struct host_t is NULL in prnt_stat! ");
		return;
	}
	printf("%d\t(%.20s)\t%lu\t%lu\n",h->sockd,h->addr,h->bytes_rcv,h->bytes_snd);
}


/***************************************************************************
 *                                                                         *
 *                         CLIENT SIDE FUNCTIONS                           *
 *                                                                         *
 ***************************************************************************/

/**
 * clnt_conn(): Connect to the remote server.
 * Arguments: ip = IP address of the remote server
              port = port to connect.
 * Return Value: a host_t structure of the server.
 */
struct host_t *clnt_conn(const char *ip, uint16_t port) {
	/* Fill in here. */
	int sfd=0;
	struct sockaddr_in serv_addr; 
	/* For Connection */
	if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		handle_error("Error: socket error! ");
		return NULL;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
	{
		handle_error("Error: inet_pton error! ");
		return NULL;
	}

	if(connect(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		handle_error("Error: connect fail! ");
		return NULL;
	}

	/* Prepare return structure */
	struct host_t *ht = convertToHost_t(&serv_addr,sfd,0,0);
	return ht;
}

/**
 * clnt_open(): Open a remote file.
 * Arguments: s = host_t struct of the server.
 * Return Value: a file_t struct of the open file, or NULL.
 */
struct file_t *clnt_open(struct host_t *s, char *filename) {
  	/* Fill in here. */
	if(s == NULL || filename == NULL)
	{
		handle_error("struct host_t* is NULL or filename is NULL in clnt_open! ");
		return NULL;
	}
	int sfd = s->sockd;
	struct blk_t blk;
	memset(&blk,0,sizeof(blk));
	blk.meta1 = OPEN;
	strncpy(blk.data, filename, FILENAME_SIZE);
#ifdef DEBUG
	printf("The file going to be opened: %s\n",filename);
	printf("The file going to be opened in blk: %s\n",blk.data);
	int i=0;
	for(i=0;i<FILENAME_SIZE;i++)
		printf("%c ",filename[i]);
	printf("\n");
#endif
	if(write(sfd,&blk,sizeof(struct blk_t))<0)
	{
		handle_error("Error: write in clnt_open! ");
		return NULL;
	}

	/* read the server returned struct file_t */
	char recvBuff[1024];
	memset(recvBuff, '\0',sizeof(recvBuff));
	int n=0;
	struct blk_t* recv_blk = NULL;
	if((n = read(sfd,recvBuff,sizeof(recvBuff)-1))>0)
	{
		recvBuff[n] = 0;
		recv_blk = (struct blk_t*)recvBuff;
#ifdef DEBUG
		printf("recvBuff: %s\n",recvBuff);
		printf("received blk: meta1: %d, meta2: %d, data: %s\n",recv_blk->meta1,recv_blk->meta2,recv_blk->data);
		printf("number of received bytes: %d\n",n);
#endif
	}
	if(n<0)
	{
		handle_error("Error: read error! ");
	}
	if(recv_blk ==NULL)
	{
		handle_error("Error: only read 0! ");
		return NULL;
	}
	if(recv_blk->meta2 == -1)
	{
		printf("The open commmand is not successful\n");
		return NULL;
	}

	struct file_t* ft = (struct file_t*)malloc(sizeof(struct file_t));
	ft->fd = recv_blk->meta1;
#ifdef DEBUG
	if(ft!=NULL) printf("received fd: %d\n",ft->fd);
#endif
  	return ft;
}

/**
 * clnt_close(): Close (save) a file.
 * Arguments: f = file_t struct of the open file,
              s = host_t struct of the server.
			        close_type = DONTSAVE/SAVE
 * Return Value: None.
 */
void clnt_close(struct host_t *s, struct file_t *f, int close_type) {
  /* Fill in here. */
}

/**
 * clnt_term(): Teardown connection with the server.
 * Arguments: s = host_t struct of the server.
 * Return Value: None.
 */
void clnt_term(struct host_t *s) {
  /* Fill in here. */
}

/**
 * clnt_waitcmd(): Wait for a command from user.
 * Arguments: s = host_t struct of the server.
 * Return Value: The command from client. If an error occurs, return NULL.
 */
struct cmd_t *clnt_waitcmd(struct host_t *s) {
	/* Fill in here. */
	if(s==NULL) return NULL;
	char cmd[20] = "";
	int res = 0;
	int type = 0;
	printf("Waiting for user command: (open, read, write, close, quit)\n");
	if(fgets(cmd,20,stdin) != NULL)
	{
		if(strncmp(cmd, "open", 4)==0)
		{
			type = OPEN;
		}
		else if(strncmp(cmd, "read", 4)==0)
		{
			type = READ;
		}
		else if(strncmp(cmd, "write", 5)==0)
		{
			type = WRITE;
		}
		else if(strncmp(cmd, "close", 5)==0)
		{
			type = CLOSE;
		}
		else if(strncmp(cmd, "quit", 4)==0)
		{
			type = QUIT;
		}
		else
		{
			return NULL;
		}
		struct cmd_t* cmdst = (struct cmd_t*)malloc(sizeof(struct cmd_t));
		cmdst->type = type;
		cmdst->res = res;
		return cmdst;
	}
	
	return NULL;
}

/**
 * clnt_proccmd(): Process a command and send the data to the server.
 * Arguments: s = host_t server's information, cmd = the command to process.
 * Return Value: 0 on success, -1 on failure.
 */
int clnt_proccmd(struct host_t *s, struct cmd_t* cmd) {
	/* feel free to implement read/write in any way that you want */
	if(cmd == NULL || s == NULL) return -1;
	int type = cmd->type;

	int sfd = s->sockd;
	char* msg = (char*)malloc(sizeof(char)*20);
	memset(msg,'\0',20);

	struct blk_t blk;
	blk.meta1 = type;
	memset(blk.data,'\0',BLOCK_SIZE);
	/* Fill in here. */
	switch(type){
		case OPEN:
			msg = "OPEN";
			break;
		case READ:
			msg = "READ";
			break;
		case WRITE:
			msg = "WRITE";
			break;
		case CLOSE:
			msg = "CLOSE";
			break;
		case EDIT:
			msg = "EDIT";
			break;
		case QUIT:
			msg = "QUIT";
			break;
		default:
			return -1;
	}
#ifdef DEBUG
	printf("msg: %s\n",msg);
#endif
	blk.meta2 = 20;
	strncpy(blk.data,msg,5);
	if(write(sfd,&blk,sizeof(struct blk_t))<0)
	{
		handle_error("Error: write in clnt_proccmd! ");
		return -1;
	}
	return 0;
}

/**
 * clnt_write(): Write a character to the open file
 * Arguments: f = file_t struct of the opened file
 *            offset = offset within file, 
 *            ch = character to write
 * Return Value: 0 on success, -1 on failure.
 */
int clnt_write(struct file_t *f, unsigned int off, char ch) {
  /* Fill in here. */
  return -1;
}

/**
 * clnt_read(): Read block from an opened file.
 * Arguments: f = file_t struct of the opened file
 *            p = pointer to buffer
 *            off = offset within file
 *            sz = max size
 * Return Value: Number of bytes read.
 */
size_t clnt_read(struct file_t *f, char *p, unsigned int off, size_t sz) {
  /* Fill in here. */
  return 0;
}



/***************************************************************************
 *                                                                         *
 *                         SERVER SIDE FUNCTIONS                           *
 *                                                                         *
 ***************************************************************************/

/**
 * serv_bind(): bind the server at a specific address and port .
 * Arguments: ip = IP address to bind, port = port to bind to.
 * Return Value: 0 on success, -1 on failure.
 */
int serv_bind(const char *ip, uint16_t port) {
	/* Fill in here. */
	/* For Connection */
	if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		handle_error("Error: socket error! ");
		return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
	{
		handle_error("Error: inet_pton error! ");
		return -1;
	}

	if(bind(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
	{
		handle_error("Error: bind error! ");
		return -1;
	}

	return 0;
}

/**
 * serv_wait(): wait for a client to connect.
 * Arguments: None.
 * Return Value: A host_t struct of the remote client. If an error occurs, return NULL.
 */
struct host_t *serv_wait( void ) {
	/* Fill in here. */
	int connfd=0;
	if(listen(sfd, 10)<0)
	{
		handle_error("Error: listen! ");
		return NULL;
	}
	if((connfd = accept(sfd, (struct sockaddr*)NULL, NULL))<0)
	{
		handle_error("Error: accept! ");
		return NULL;
	}
	/* Prepare return structure */
	struct host_t *ht = convertToHost_t(&serv_addr,connfd,0,0);
	return ht;
}

/**
 * serv_waitcmd(): wait for a command from client.
 * Arguments: cl = host_t struct of the server.
 * Return Value: The command from client. If an error occurs, return NULL.
 */
struct cmd_t *serv_waitcmd(struct host_t *cl) {
	/* Fill in here. */
	if(cl==NULL){
		handle_error("Error: cl is NULL in serv_waitcmd! ");
		return NULL;
	}
	int sfd = cl->sockd;
	int n=0;
	char recvBuff[1024];

	memset(recvBuff, '\0',sizeof(recvBuff));
	struct blk_t* blk = NULL;
	if((n = read(sfd,recvBuff,sizeof(recvBuff)-1))>0)
	{
		recvBuff[n] = 0;
		blk = (struct blk_t*)recvBuff;
#ifdef DEBUG
		printf("recvBuff: %s\n",recvBuff);
		printf("received blk: meta1: %d, meta2: %d, data: %s\n",blk->meta1,blk->meta2,blk->data);
		printf("number of received bytes: %d\n",n);
#endif
	}
	if(n<0){
		handle_error("Error: read error! ");
		return NULL;
	}
	if(blk==NULL){
		handle_error("Error: only read 0! ");
		return NULL;
	}
	
	/* Prepare for struct cmd_t */
	struct cmd_t* ct = (struct cmd_t*)malloc(sizeof(struct cmd_t));
	ct->type = blk->meta1;
	ct->res = blk->meta2;
	strncpy(serv_data, blk->data, BLOCK_SIZE);
#ifdef DEBUG
	printf("The file going to be opened in blk: %s\n",serv_data);
	printf("The file going to be opened in blk: %s\n",blk->data);
			int i=0;
			for(i=0;i<520;i++)
				printf("%c ",blk->data[i]);
			printf("\n");
#endif
	return ct;
}

/**
 * serv_proccmd(): Process a command and send the results back to client.
 * Arguments: cl = host_t struct of the server, cmd = the command to process.
 * Return Value: 0 on success, -1 on failure. If command closes connection with the 
 *  server, function returns 1.
 */
int serv_proccmd(struct host_t *cl, struct cmd_t* cmd) {
	/* Fill in here. */
	if(cmd == NULL || cl == NULL) return -1;
	int type = cmd->type;
	int sfd = cl->sockd;

	struct blk_t blk;
	int fd;
	switch(type){
		case OPEN:
			blk.meta2 = 0;
#ifdef DEBUG
			char tmp[6] = {0};
			strncpy(tmp, serv_data, 6);
			printf("The file going to be opened: %s\n",serv_data);
			int i=0;
			for(i=0;i<520;i++)
				printf("%c ",serv_data[i]);
			printf("\n");
#endif
			if((fd = open(serv_data, O_RDWR))<0)
			{
				handle_error("Error: open file in serv_proccmd! ");
				blk.meta2 = -1;
			}
			blk.meta1 = fd;
#ifdef DEBUG
			printf("The file opened in server side: %d\n",fd);
#endif
			break;
		case READ:
			break;
		case WRITE:
			break;
		case CLOSE:
			break;
		case EDIT:
			break;
		case QUIT:
			break;
		default:
			return -1;
	}

	/* Send msg back to client */
	if(type == OPEN)
	{
#ifdef DEBUG
		printf("sending blk: meta1: %d, meta2: %d, data: %s\n",blk.meta1,blk.meta2,blk.data);
#endif
		if(write(sfd,&blk,sizeof(struct blk_t))<0)
		{
			handle_error("Error: write in clnt_open! ");
			return -1;
		}
	}
	return 0;
}
