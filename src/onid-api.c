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
#define size_align_with_block(x) (x/BLOCK_SIZE+1)*BLOCK_SIZE; 
#define DEBUG
/* For server */
int sfd;
struct sockaddr_in serv_addr; 
char serv_data[ BLOCK_SIZE ];
struct file_content fileContent[ MAX_CONNECTION];
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
	if((n = read(sfd,recvBuff,sizeof(struct blk_t)))>0)
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
	ft->sockd = sfd;
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
	if(s == NULL || f == NULL)
	{
		handle_error("struct host_t* is NULL or struct file_t* is NULL in clnt_close! ");
	}
	int sfd = s->sockd;
	struct blk_t blk;
	memset(&blk,0,sizeof(blk));
	blk.meta1 = CLOSE;
	blk.meta2 = close_type;
#ifdef DEBUG
	printf("The close_type: %d\n",blk.meta2);
#endif
	if(write(sfd,&blk,sizeof(struct blk_t))<0)
	{
		handle_error("Error: write in clnt_close! ");
	}
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
	if(cmd == NULL || s == NULL) return FAILURE;
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
			return FAILURE;
	}
#ifdef DEBUG
	printf("msg: %s\n",msg);
#endif
	blk.meta2 = 20;
	strncpy(blk.data,msg,5);
	if(write(sfd,&blk,sizeof(struct blk_t))<0)
	{
		handle_error("Error: write in clnt_proccmd! ");
		return FAILURE;
	}
	return SUCCESS;
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
	if(f == NULL)
	{
		handle_error("struct file_t *f is NULL in clnt_write! ");
		return FAILURE;
	}
	int sfd = f->sockd;
	struct blk_t blk;
	memset(&blk,0,sizeof(blk));
	blk.meta1 = WRITE;
	blk.meta2 = (int)off;
	blk.data[0] = ch;
#ifdef DEBUG
	printf("The ch going to be written: %c\n",ch);
	printf("The ch going to be written in blk: %s\n",blk.data);
#endif
	if(write(sfd,&blk,sizeof(struct blk_t))<0)
	{
		handle_error("Error: write in clnt_write! ");
		return FAILURE;
	}
	
	return SUCCESS;
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
	if(f==NULL || p==NULL)
	{
		handle_error("struct file_t *f or char*p is NULL in clnt_read! ");
		return 0;
	}
	int sfd = f->sockd;
	struct blk_t blk;
	memset(&blk,0,sizeof(blk));
	blk.meta1 = READ;
	blk.meta2 = (int)off;
	blk.data[0] = (char)sz;
#ifdef DEBUG
	printf("Going to read %zu chars from offset:%u\n",sz,off);
#endif
	if(write(sfd,&blk,sizeof(struct blk_t))<0)
	{
		handle_error("Error: write in clnt_read! ");
		return 0;
	}

	/* read the server returned struct blk_t */
	char recvBuff[1024];
	memset(recvBuff, '\0',sizeof(recvBuff));
	int n=0;
	struct blk_t* recv_blk = NULL;
	int end = 0;
	int pos = 0;
	while(!end)
	{
		if((n = read(sfd,recvBuff,sizeof(struct blk_t)))>0)
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
			return 0;
		}
		end = recv_blk->meta2; //The flag that whether the server ends sending data
		strncpy(p+pos*sizeof(char),recv_blk->data,recv_blk->meta1); //meta1 is for char size
		pos += recv_blk->meta1;
		p[pos] = '\0';
#ifdef DEBUG
		printf("content of p:%s, pos: %d\n",p,pos);
#endif
	}
	return pos;
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
		return FAILURE;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
	{
		handle_error("Error: inet_pton error! ");
		return FAILURE;
	}

	if(bind(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
	{
		handle_error("Error: bind error! ");
		return FAILURE;
	}

	return SUCCESS;
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
	/* Initialize fileContent */
	memset(fileContent,0,MAX_CONNECTION*sizeof(struct file_content));
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
	if((n = read(sfd,recvBuff,sizeof(struct blk_t)))>0)
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
	if(cmd == NULL || cl == NULL) return FAILURE;
	int type = cmd->type;
	int sfd = cl->sockd;

	struct blk_t blk;
	switch(type){
		case OPEN:
		{
			blk.meta2 = 0;
#ifdef DEBUG
			char tmp[6] = {0};
			strncpy(tmp, serv_data, 6);
			printf("The file going to be opened: %s\n",serv_data);
			int i=0;
			for(i=0;i<BLOCK_SIZE;i++) printf("%c",serv_data[i]);
			printf("\n");
#endif
			int fd;
			if((fd = open(serv_data, O_RDWR))<0)
			{
				handle_error("Error: open file in serv_proccmd! ");
				blk.meta2 = -1;
			}
			blk.meta1 = fd;
#ifdef DEBUG
			printf("The file opened in server side: %d\n",fd);
#endif
			if(sfd<0 || sfd>=MAX_CONNECTION)
			{
				handle_error("Error: sfd is not within 0-100! ");
			}
			/* Get the size of the file */
			int size = lseek(fd, 0L, SEEK_END);
			lseek(fd, 0L, SEEK_SET);
			int aligned_size = size_align_with_block(size);
#ifdef DEBUG
			printf("file size:%d, aligend size:%d (block_size:%d)\n",size,aligned_size,BLOCK_SIZE);
#endif

			fileContent[sfd].content = (char*)malloc(aligned_size*sizeof(char));
			fileContent[sfd].size = size;
			fileContent[sfd].flag = 1;
			fileContent[sfd].fd = fd;
			/* Get the content from file */
			ssize_t read_c_size;
			if((read_c_size = read(fd, fileContent[sfd].content, size))<0)
			{
				handle_error("Error: read from file error! ");
			}
#ifdef DEBUG
			printf("Number of char read from file:%zd\n",read_c_size);
			printf("file content:%s\n",fileContent[sfd].content);
#endif

			/* Send msg back to client */
#ifdef DEBUG
			printf("sending blk: meta1: %d, meta2: %d, data: %s\n",blk.meta1,blk.meta2,blk.data);
#endif
			if(write(sfd,&blk,sizeof(struct blk_t))<0)
			{
				handle_error("Error: write in clnt_open! ");
				return FAILURE;
			}
			break;
		}
		case READ:
		{
			unsigned int off = (unsigned int)cmd->res; //blk->meta2
			size_t sz = (char)serv_data[0];
#ifdef DEBUG
			printf("Offset:%u, size:%zu\n",off,sz);
#endif

			/* Send msg back to client */
			int pos=0;
			while(sz > BLOCK_SIZE)
			{
				strncpy(blk.data,fileContent[sfd].content+pos*sizeof(char),BLOCK_SIZE);
				blk.meta1 = BLOCK_SIZE; // number of sent characters
				blk.meta2 = 0; //not end
				sz-=BLOCK_SIZE;
				pos+=BLOCK_SIZE;
#ifdef DEBUG
				printf("sending blk: meta1: %d, meta2: %d, data: %s\n",blk.meta1,blk.meta2,blk.data);
#endif
				if(write(sfd,&blk,sizeof(struct blk_t))<0)
				{
					handle_error("Error: write when sending data to client! ");
					return FAILURE;
				}
			}
			strncpy(blk.data,fileContent[sfd].content+pos*sizeof(char),sz);
			blk.meta1 = sz;
			blk.meta2 = 1; //end for sending
#ifdef DEBUG
				printf("sending blk: meta1: %d, meta2: %d, data: %s\n",blk.meta1,blk.meta2,blk.data);
#endif
			if(write(sfd,&blk,sizeof(struct blk_t))<0)
			{
				handle_error("Error: write when sending data to client! ");
				return FAILURE;
			}
			break;
		}
		case WRITE:
		{
			unsigned int off = (unsigned int)cmd->res; //blk->meta2
			char ch = serv_data[0];
#ifdef DEBUG
			printf("The ch:%c, offset:%u in sfd:%d\n",ch,off,sfd);
#endif
			if(fileContent[sfd].flag <= 0)
			{
				handle_error("The file content entry is not initialized! ");
			}
			/* realloc if size is not enough*/
			unsigned int aligned_size = size_align_with_block(off);
			unsigned int used_aligned_size = size_align_with_block(fileContent[sfd].size);
			if(aligned_size > used_aligned_size)
			{
				char* tmp = realloc(fileContent[sfd].content,aligned_size*sizeof(char));
				if(tmp == NULL)
				{
					handle_error("Realloc Fail! ");
				}
				else
				{
					fileContent[sfd].content = tmp;
				}
				fileContent[sfd].size = off;
#ifdef DEBUG
				printf("offset:%d, aligend size:%d (block_size:%d)\n",off,aligned_size,BLOCK_SIZE);
#endif
			}
			/* update the content */
			fileContent[sfd].content[off] = ch;
#ifdef DEBUG
			printf("file content:%s\n",fileContent[sfd].content);
#endif
			break;
		}
		case CLOSE:
		{
			int close_type = cmd->res; //blk->meta2
			int fd = fileContent[sfd].fd;
#ifdef DEBUG
			printf("The close_type: %d\n",blk.meta2);
#endif
			if(fileContent[sfd].flag <= 0)
			{
				handle_error("The file content entry is not initialized! ");
			}

			/* Record Data */
			if(close_type == SAVE)
			{
#ifdef DEBUG
				printf("Close type:SAVE, written size:%d)\n",fileContent[sfd].size);
#endif
				lseek(fd, 0L, SEEK_SET);
				if(write(fd,fileContent[sfd].content,fileContent[sfd].size)<0)
				{
					handle_error("Error: write error in close command! ");
				}
			}
			//need to free filecontent
			free(fileContent[sfd].content);
			memset(&fileContent[sfd],0,sizeof(struct file_content));

			//need to close the open file
			if(close(fd)<0)
			{
				handle_error("Error: close file error in close command! ");
			}
			break;
		}
		case EDIT:
			break;
		case QUIT:
		{

			break;
		}
		default:
			return FAILURE;
	}

	return SUCCESS;
}
