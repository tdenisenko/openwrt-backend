#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

void do_exit(PGconn *conn) {
    
    PQfinish(conn);
    exit(1);
}

int main(int argc, char *argv[]) {
	
	PGconn *conn = PQconnectdb("user=tim password=ncc2017 dbname=wrt host=localhost");

    if (PQstatus(conn) == CONNECTION_BAD) {
        
        fprintf(stderr, "Connection to database failed: %s\n",
            PQerrorMessage(conn));
        do_exit(conn);
    }

	const int LEN = 11;
    const char *paramValues[3];
	int ret;
	int routerNo;
	int rssi;
	char mac_id[18];
	char *line = NULL;
	size_t size;
	while(getline(&line, &size, stdin) != -1) {
		ret = sscanf(line, "%d %*s %s %d", &routerNo, mac_id, &rssi);
		if (ret != 3) {
			fprintf(stderr, "Wrong input, only %d words found.\n", ret);
			exit(1);
		}
		//printf("Your command: %d %s %d\n", routerNo, mac_id, rssi);
		char str[LEN];
		char str2[LEN];
		snprintf(str, LEN, "%d", routerNo);  
		paramValues[0] = str;
		paramValues[1] = mac_id;
		snprintf(str2, LEN, "%d", rssi);  
		paramValues[2] = str2;
		char *stm = "INSERT INTO connections (mac_id, router_id, rssi) VALUES ($2, $1, $3) ON CONFLICT (mac_id, router_id) DO UPDATE SET rssi = excluded.rssi;";
		PGresult *res = PQexecParams(conn, stm, 3, NULL, paramValues, NULL, NULL, 0);
		
		if (PQresultStatus(res) != PGRES_COMMAND_OK) {

			char *errmsg = PQresultErrorMessage(res);
			printf("Error: %s\n", errmsg);
			PQclear(res);
			do_exit(conn);
		} 
		
		PQclear(res);
	}
	
	fprintf(stderr, "Error reading line\n");
	do_exit(conn);
	
	return 0;
}