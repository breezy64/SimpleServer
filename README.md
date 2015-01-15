# SimpleServer

Description: A simple server written in C. There are two programs: server_f and server_p. Server f spawns a new thread when responding to a request, whilst server p spawns a new thread (server p is faster). This server can currently only handle GET requests

Installation: Simply running Make all will compile both programs. This program will compile with gcc 4.8.2

Arguments: ./server_f [port number] [log file location]
           ./server_p [port number] [log file location]
           (eg. running ./server_p 80 ~/log.txt will start server p on port 80 and server logs will be written to log.txt)
           
ToDos:-PUT, POST, and DELETE requests 
