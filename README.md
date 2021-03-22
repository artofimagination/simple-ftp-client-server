# simple-ftp-client-server

# How it works?
Both client and Server consists of a UI and a Network compponents. Also Server has a ThreadController member.
  - UI implements a simple command line interface
  - Network is responsible of sending and receiving pakcets and connect, disconnect
  - Thread controller maintains the threadpool responsible of handling each client connecting to te server. This happens through ClientHandler objects.
  
The Client Handler has the following:
  - UI
  - Network
  - FileProcessor, that is rsponsible to split up the files into chunks, maintain transfer progress of larger files and read/write to/from files

On Server start
  1. Creates a socket
  2. Waits for client connections
  3. Starts periodic client poll

On client connection
  1. Threadcontroller creates a Client handler object and a thread that is using the resources of this object
  2. Starts listening to client packets
  
Transfering files:
  1. User defines the files and folder to be copied.
  2. Traverses the the folder hierarchy and files defined by the user input and creates info header for each object found
    - infoheader stores name length, file content length, seek position, file type
  3. Starts to fill up a FsBuffer (file system buffer) based on the info headers generated previously.
  4. The Buffer has a limited size and is referred as a chunk.
  5. It puts as many file content in the buffer as it fits, 
    - if there is a file that is larger than the buffer or there are too many files it will repeat step 4, after the complete chunk has been processed on the other side
  6. Send the chunk to the other side wrapped into a packet
  
Writing after transfer:
  1. When the chunk arrives to the other side the packet will be unwrapped, than the FsBuffer will be written to the files described in the info headers
    - note that the info headers are transferred as well to the otherside alongisde with the content
  2. When the chunk is written in files a confirmation is sent back to the source
  3. Source keeps creating and sending the next chunks until the whole selection is transferred
  
 This works on client-server, server-client direction alike
 
Threading:
  - UI interface is not handling in a trhedsafe way in the implementation
  - The number of shared resources on the server side are minimized. Each client handler has its own network, UI, and file processor
  - They only share the file access table

File access table:
  - It contains the list of files available in the server storage folder
  - Each file has an access object that tells if there is a writer, who is that writer or the number of readers.
  - When a client handler accesses the table mutex is applied
  
  1. When a reading happens on a file that is not accessed, a reader reference count is increased and the access is set to read.
  2. If there is at least on read happening on the file no write is permitted
  3. When a writing happens on a file that is not accessed at the moment, the writer socket will be stored  and the qaccess mode is set to write
  4. If there is a write on the file, no other write or no read can happen.
  
Features not implemented but would be important:
  1. Network packet timeout. If there is a multi chunk transfer each send has to be checked for timeout. If no ack arrives, the transfer has to be interrupted. Both sides needs the timeout check and on timeout clear the file processing related components.
  2. Access table reset on client interrupt. At the moment of the transfer is interrupted the server will not set the file accesses to none, server has to be restarted
  3. Client is not disconnecting on server termination. Need a client side polling.
  4. CRC check on each chunk and at the and when awhole file is transferred. If that is mismatching, the chunk has to be resent.

# Setup
Assuming that the host system is Ubuntu 18.04+
  - Run ```./installDependencies.sh```

This will install build-essential, cmake, ninja-build

# Compile and Run
  - Run server ```./runServer.sh <file storage folder absolute path>```
  - Run client ```./runClient.sh```
  
Note: The transfer happens slow for testing. To increase speed reduce the delays in the listenToPAckets functions on both sides.

# Tested cases
  - Send single file to server - Passed
  - Send multiple files to server - Passed
  - Send folder to server - Passed
  - Send nested folder and files to server - Passed
  - Send large file to server - Passed
  - Same as above but getting from server - Passed
  - Interrupt writing on server then retry - fails. Access map is not resolved if the client is suddenly disconnected. If any operation is interrupted server needs to be restarted before the next test.
  
  - Connect/disconnect with single client - Passed
  - Connect/disconnect multiple clients - Passed
  - Overwrite file on server while at least one client reads the same file - Passed
  - Try to overwrite file on server while other client is writing it - Passed
  - Read file from server while another client writes it - Passed
  - Read file by multiple clients - Passed
