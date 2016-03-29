#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>

struct server_id{
    char ip_addr[ 20 ];
    char port_num[ 20 ]; 
};

int main( int argc, char *argv[] ) {
    int dirport_num, dbport_num, port_num, sockfd, sockfd1, sockfd2;
    int i, j, k, w, index, index1, count = 0, iter;
    double total_time = 0.0, sum_time = 0.0, avg_time = 0.0;
    struct timeval start, end;  		
    struct sockaddr_in dirserv_addr, dbserv_addr, serv_addr;
    struct hostent *dirserver; 
    struct hostent *dbserver;
    struct hostent *server;
    struct hostent *client;	
    char *clientip;
    char hostname[ 128 ];	
    const char *diraddr = "csel-kh4250-06.cselabs.umn.edu";            
    const char *dbaddr = "csel-kh1200-13.cselabs.umn.edu";             
    char data[ 500 ];
    char failmsg[ 50 ] = "failure\r\n";
    char finish[ 500 ] = "Finish\r\n";
	char closeM[ 500 ] = "Close\r\n";
    char getrecord[ 50 ] = "get-records\r\n";
    struct server_id id[ 50 ]; 	
    int size[ 4 ] = { 10000, 100000, 1000000, 10000000 };
    const char *sizec[ 4 ];
    sizec[ 0 ] = "10";
    sizec[ 1 ] = "100";
    sizec[ 2 ] = "1000";
    sizec[ 3 ] = "10000";
    char str[ 20 ];     
 
    //	Make message a char const is 1 byte in c, and one data message set as 500 char const,    
    //  that is, 500B in one message. 
    for ( i = 0; i < 500; i++ ){
    	data[ i ] = 'a';			
    }
	
    // Check if have enough arguments are given 
    if ( argc < 3 ){
    	printf( "Not enough argument.\n" );
		exit( 0 );
    }
    dirport_num = atoi( argv[ 1 ] );
    dbport_num = atoi( argv[ 2 ] ); 
    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( sockfd < 0 ) {
        perror( "ERROR creating socket failed.\n" );
		exit( 0 );
    }

    // Check if servername input on the command line is legal
    dirserver = gethostbyname( diraddr ); 
    if ( dirserver == NULL ) {
        printf( "Server does not exist.\n" );
		exit( 0 );
    }
  
    bzero( ( char * ) &dirserv_addr, sizeof( dirserv_addr ) );
    dirserv_addr.sin_family = AF_INET;
    bcopy(( char * )dirserver->h_addr,
          ( char * )&dirserv_addr.sin_addr.s_addr,
          dirserver->h_length );
    dirserv_addr.sin_port = dirport_num;		

    // Connect with a dir server
    if ( connect( sockfd, ( struct sockaddr * ) &dirserv_addr, sizeof( dirserv_addr ) ) < 0 ) {
        perror( "ERROR connecting fail.\n" );
		exit( 0 );
    }

    // Send out the message to request a list of servers
    char message[ 50 ] = "list-servers\r\n"; 
    send( sockfd, &message, sizeof( message ), 0 );

    // Reveive the message from dir server
    char recmsg[ 100 ];
    recv( sockfd, &recmsg, sizeof( recmsg ), 0 );

    // Print response message from dir server on screen
    for ( i = 0; i < strlen( recmsg ); i++ ) {
        if ( recmsg[ i ] == '\r' ) {
            printf( "\n" );
        }
        else {
            printf( "%c", recmsg[ i ] );
        }
    }
    close( sockfd );
    if ( strcmp( recmsg, failmsg ) != 0 ) {
        // Read in the first server's IP address and port number
        i = 8;     // the first char of the server ip address
	    j = 0;	
        // Separate different servers 128.101.37.1 9123\r128.......
		while ( 1 ) {
		    // Separate ip and port number				
			while ( 1 ) {
			    id[ count ].ip_addr[ j ] = recmsg[ i ];
			    i++;	
			    j++;
				if ( recmsg[ i ] == ' ' ) {
					id[ count ].ip_addr[ j ] = '\0';
					break;	
				}		 					    		
		    }
			j = 0;	
			i++;
			while( 1 ) {	
				id[ count ].port_num[ j ] = recmsg[ i ];
				i++;
				j++;
				if ( recmsg[ i ] == '\r' ){
					id[ count ].port_num[ j ] = '\0';
					break;
				}		
			}
			i++;
			count++;
			j = 0;
			if ( recmsg[ i ] == '\n' ) {
				break;			
			}
	    }
		i = 0;	
		char avgt[ count ];
		for ( k = 0; k < count; k++ ){
			sockfd1 = socket( AF_INET, SOCK_STREAM, 0 );
			if ( sockfd1 < 0 ) {
				perror( "ERROR creating socket failed.\n");
				exit( 0 );
			}
			server = gethostbyname( id[ k ].ip_addr );
			// If not equal to NULL then it is a valid server, else read another server's IP address and port number
		    if ( server != NULL ) {
	            //break;
		    }
			port_num = atoi( id[ k ].port_num );
	        bzero((char *) &serv_addr, sizeof(serv_addr));
	        serv_addr.sin_family = AF_INET;
	        bcopy((char *)server->h_addr,
	             (char *)&serv_addr.sin_addr.s_addr,
	             server->h_length);
	        serv_addr.sin_port = port_num;
		
	        // Connect with an app server
	        if ( connect( sockfd1, (struct sockaddr *) &serv_addr, sizeof( serv_addr ) ) < 0 ){
	            perror( "ERROR connecting fail.\n" );
	            exit( 0 );
	        }
			
			for ( i = 0; i < 4; i++ ){
				// iter is how many times need to transfer the data
				iter = ( size[ i ] / 500 ); 
			    for ( j = 0; j < 5; j++ ){  
					char recmsg[ 50 ];	
					for ( index = 0; index < iter; index++ ){
						gettimeofday( &start, NULL );
						send( sockfd1, &data, sizeof( data ), 0 );
						gettimeofday( &end, NULL );
						total_time = total_time + ( ( end.tv_sec * 1000000 + end.tv_usec ) - ( start.tv_sec * 1000000 + start.tv_usec ) );		   
					
					}
					gettimeofday( &start, NULL );
					send( sockfd1, &finish, sizeof( finish ), 0 );			
					recv( sockfd1, &recmsg, sizeof( recmsg ), 0 );
					gettimeofday( &end, NULL );
					total_time = total_time + ( ( end.tv_sec * 1000000 + end.tv_usec ) - ( start.tv_sec * 1000000 + start.tv_usec ) );		    	
					sum_time += total_time;					
					// Print out received message
					for ( index1 = 0; index1 < strlen( recmsg ); index1++ ){
						if ( recmsg[ index1 ] != '\r' ){
							printf( "%c", recmsg[ index1 ] );
						}
					}	
			    }
			 	avg_time = ( sum_time / 5.0 );
				total_time = 0.0;
				sum_time = 0.0;
			    sprintf( str, "%f", avg_time );
			 	
				// Set record	
				sockfd2 = socket(AF_INET, SOCK_STREAM, 0);
    			if( sockfd2 < 0 ){
        			perror( "ERROR creating socket failed.\n");
					exit( 0 );
    			} 
				// Check if servername input on the command line is legal
				dbserver = gethostbyname( dbaddr ); 
				if ( dbserver == NULL ){
					printf( "Server does not exist.\n" );
				    exit( 0 );
				}
		
				bzero((char *) &dbserv_addr, sizeof(dbserv_addr));
				dbserv_addr.sin_family = AF_INET;
				bcopy((char *)dbserver->h_addr,
				     (char *)&dbserv_addr.sin_addr.s_addr,
					 dbserver->h_length);
				dbserv_addr.sin_port = htons( dbport_num );        
		
				// Connect with a db server
				if ( connect( sockfd2, (struct sockaddr *) &dbserv_addr, sizeof( dbserv_addr ) ) < 0 ){
					perror( "ERROR connecting fail.\n" );
					exit( 0 );
				}

				// Send out the message to request a list of servers
				// Get the host name of the machine.
				if ( gethostname( hostname, sizeof hostname ) < 0 ){
					perror( "ERROR getting host name\n");
					exit( 0 );   
				}

				client = gethostbyname( hostname );
				if ( server == NULL ){
					perror( "Error gethostbyname\n " );
					exit( 0 );
				}
				clientip = inet_ntoa(*(struct in_addr *)client->h_addr);	
				char message[ 256 ] = "set-record ";
				strcat( message, clientip );
				strcat( message, " " );
				strcat( message, id[ k ].ip_addr );
				strcat( message, " " );
				strcat( message, id[ k ].port_num );
				strcat( message, " " );	
				strcat( message, sizec[ i ] );
				strcat( message, " ");
				strcat( message, str );
				strcat( message, "\r\n" );
				send( sockfd2, &message, sizeof( message ), 0 );
				// Reveive the message from db server
				char recmsg[ 256 ];
				recv( sockfd2, &recmsg, sizeof( recmsg ), 0 );	
				// Print response message from db server on screen
				for ( w = 0; w < strlen( recmsg ); w++ ){
					if ( recmsg[ w ] != '\r' ){
						printf( "%c", recmsg[ w ] );
					}
				}
				shutdown(sockfd2,2);	
		    }	 
		send( sockfd1, &closeM, sizeof( closeM ), 0 );
		} 			    	    
        // Get record 
		sockfd2 = socket(AF_INET, SOCK_STREAM, 0);
		if( sockfd2 < 0 ){
			perror( "ERROR creating socket failed.\n");
			exit( 0 );
		} 
		// Check if servername input on the command line is legal
		dbserver = gethostbyname( dbaddr ); 
		if ( dbserver == NULL ){
			printf( "Server does not exist.\n" );
		    exit( 0 );
		}

		bzero((char *) &dbserv_addr, sizeof(dbserv_addr));
		dbserv_addr.sin_family = AF_INET;
		bcopy((char *)dbserver->h_addr,
		     (char *)&dbserv_addr.sin_addr.s_addr,
			 dbserver->h_length);
		dbserv_addr.sin_port = htons( dbport_num );        

		// Connect with a db server
		if ( connect( sockfd2, (struct sockaddr *) &dbserv_addr, sizeof( dbserv_addr ) ) < 0 ){
			perror( "ERROR connecting fail.\n" );
			exit( 0 );
		}
        send( sockfd2, &getrecord, sizeof( getrecord ), 0 );
        // Reveive the message from dir server
        char recmsg[ 1000 ];
        recv( sockfd2, &recmsg, sizeof( recmsg ), 0 );
        // Print response message from dir server on screen
        for ( i = 0; i < ( strlen( recmsg ) - 1 ); i++ ){
            if ( recmsg[ i ] == '\r' ){
                printf( "\n" );
            }
            else{
                printf( "%c", recmsg[ i ] );
            }
        }
		printf("\n");
			
    }        
    else{
        printf("Retrieve server failure");
        exit( 0 );
    }    
    return 0;
}
 





