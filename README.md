Readme for the dehapa project.
==============================

The target program implements an online, shared file editing service (we will
call it ONID -- ONline fIle eDiting service). Your task is to implement the
functions to a pre-defined API. You are given all function and structure
declarations as a basis. This API is then used both in a server and a client
(and our test driver to test functionality). You are free to add any additional
helper functions or data structures but you must stay binary compatible with
whatever goes over the wire. The development platform is 64-bit x86 Linux and
your program must compile with both LLVM and GCC on Ubuntu 15.10.

Feel free to modify any data that goes over the wire or change any definitions
as long as you (i) implement the functions defined in onid-api.h (which will
then be called from our testing framework) and (ii) keep the names of the
structures in onid-datatypes.h. You are free to change anything in onid-api.c.
Please do not add any additional files.

Changelog:
04/01/16:
* Updated source code to make testing easier.
* Change calling conventions to make clnt_open, clnt_read, clnt_write, 
  clnt_close explicit to work on files
* Update header files prototypes for library
* Add a sample client with a set of test cases that we expect to run.

03/17/16:
* Updated source code release to clarify some questions
* Split structure definitions to onid-datatypes.h and moved function
  declarations to onit-api.h
* Added read to struct clnt_cmd according to project specification.
* Linted file
* Minor typo fixes

03/11/16:
* Initial release of source code
