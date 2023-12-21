#include "database.h"

MYSQL* connDatabase(){
    MYSQL *conn;
	
	char *server = "localhost";
	char *user = "root";
	char *password = "parola"; 
	char *database = "offlineMessenger";
	
	conn = mysql_init(NULL);
	
	/* Conexiunea la baza de date (va arunca o eroare in caz contrar): */
	if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}
	
	return conn;
}



