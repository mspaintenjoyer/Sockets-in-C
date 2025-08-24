# TCP Echo Server and Client (C)
A minimal example of a TCP echo server and a simple client using POSIX sockets. The server listens on an IP/port, accepts a connection, and echoes back whatever it receives. The client connects, sends a message, and prints the echoed response.

Features
- Tiny, self-contained C programs
- Blocking I/O, single-connection-at-a-time server to keep it simple

#Build 
- Server:
cc -O2 -Wall -Wextra echo_server.c -o echo_server

- Client:
cc -O2 -Wall -Wextra echo_client.c -o echo_client

- Run
Start the server in one terminal:

./echo_server 127.0.0.1 8080

- In another terminal, run the client:

./echo_client 127.0.0.1 8080 "hello world"

- Expected output from the client:

Echoed: hello world
