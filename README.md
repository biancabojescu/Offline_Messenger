# Project Description:

The project involves developing a client/server application that allows the exchange of messages between two or more logged-in users. Additionally, the software has implemented the functionality to send messages to offline users, which they can read later upon connection. The application also offers the ability to register in the database, change passwords, and view the conversation history with each user individually.


## Compilation Commands:

### Database:

sudo service mysql start

mysql -u root -p

### Server:

gcc server.c user.c mesaje.c database.c -o server_executable -pthread -I/usr/include/mysql -L/usr/lib/x86_64-linux-gnu -lmysqlclient

./server_executable


### Client:

gcc client.c user.c mesaje.c database.c -o client_executable -I/usr/include/mysql -L/usr/lib/x86_64-linux-gnu -lmysqlclient  

./client_executable 0 2908
