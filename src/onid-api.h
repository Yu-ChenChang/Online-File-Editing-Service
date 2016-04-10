/**************************************************************************
 * CS52700 - Software Security - Spring 2016
 * Programming Project:  API for ONline fIle eDiting (ONID)
 *
 * This file contains the necessary definitions for the ONID API. These
 * functions are used by our checker, so do not modify them.
 **************************************************************************/

/***************************************************************************
 *                                                                         *
 *                             USER FUNCTIONS                              *
 *                                                                         *
 ***************************************************************************/

struct host_t *convertToHost_t(struct sockaddr_in* soaddr, int sfd, size_t bytes_rcv, size_t bytes_snd);

struct sockaddr_in *convertToSockaddr_in(struct host_t* ht);
/**
 * prnt_stat(): Print all information and statistics of a given host
 * Arguments: h = a host_t struct
 * Return Value: None.
 */
void prnt_stat(struct host_t *h);


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
struct host_t *clnt_conn(const char *ip, uint16_t port);

/**
 * clnt_open(): Open a remote file.
 * Arguments: s = host_t struct of the server.
 * Return Value: a file_t struct of the open file, or NULL.
 */
struct file_t *clnt_open(struct host_t *s, char *filename);

/**
 * clnt_close(): Close  file, either saving or discarding changes.
 * Arguments: f = file_t struct of the open file,
              s = host_t struct of the server.
			        close_type = DONTSAVE/SAVE
 * Return Value: None.
 */
void clnt_close(struct host_t *s, struct file_t *f, int close_type);

/**
 * clnt_term(): Teardown connection with the server.
 * Arguments: s = host_t struct of the server.
 * Return Value: None.
 */
void clnt_term(struct host_t *s);

/**
 * clnt_waitcmd(): Wait for a command from user.
 * Arguments: s = host_t struct of the server.
 * Return Value: The command from client. If an error occurs, return NULL.
 */
struct cmd_t *clnt_waitcmd(struct host_t *s);

/**
 * clnt_proccmd(): Process a command and send the data to the server.
 * Arguments: s = host_t server's information, cmd = the command to process.
 * Return Value: 0 on success, -1 on failure.
 */
int clnt_proccmd(struct host_t *s, struct cmd_t* cmd);

/**
 * clnt_write(): Write a character to the open file
 * Arguments: f = file_t struct of the opened file
 *            off = offset within file, 
 *            ch = character to write
 * Return Value: 0 on success, -1 on failure.
 */
int clnt_write(struct file_t *f, unsigned int off, char ch);

/**
 * clnt_read(): Read block from an opened file.
 * Arguments: f = file_t struct of the opened file,
              buf = buffer to place data in,
              off = offset within file,
              sz = size of buf.
 * Return Value: Number of bytes read into buf.
 */
size_t clnt_read(struct file_t *f, char *buf, unsigned int off, size_t sz);


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
int serv_bind(const char *ip, uint16_t port);

/**
 * serv_wait(): wait for a client to connect.
 * Arguments: None.
 * Return Value: A host_t struct of the remote client. If an error occurs, return NULL.
 */
struct host_t *serv_wait( void );

/**
 * serv_waitcmd(): wait for a command from client.
 * Arguments: cl = host_t struct of the server.
 * Return Value: The command from client. If an error occurs, return NULL.
 */
struct cmd_t *serv_waitcmd(struct host_t *cl);

/**
 * serv_proccmd(): Process a command and send the results back to client.
 * Arguments: cl = host_t struct of the server, cmd = the command to process.
 * Return Value: 0 on success, -1 on failure.
 */
int serv_proccmd(struct host_t *cl, struct cmd_t* cmd);
