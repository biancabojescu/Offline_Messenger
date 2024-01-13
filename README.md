# baza de date:
sudo service mysql start
mysql -u root -p

# server
gcc server.c user.c mesaje.c database.c -o server_executable -pthread -I/usr/include/mysql -L/usr/lib/x86_64-linux-gnu -lmysqlclient
./server_executable


# client
gcc client.c user.c mesaje.c database.c -o client_executable -I/usr/include/mysql -L/usr/lib/x86_64-linux-gnu -lmysqlclient  
./client_executable 0 2908
