#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char *argv[])
{
    int sockfd, sockfd1, newsockfd, dirport_num, state,i, port_num;
    struct sockaddr_in dir_addr, serv_addr, cli_addr;
    struct hostent *dirserver;
    struct hostent *server;
    const char *diraddr = "csel-kh4250-06.cselabs.umn.edu";
    socklen_t serverlen = sizeof( serv_addr );
    socklen_t clientlen = sizeof( cli_addr );
    char hostname[ 128 ];
    char registration[ 50 ] = "register ";
    char resp[ 50 ];
    char str[ INET_ADDRSTRLEN ];
    char *serverip;
    char serverport_num[ 30 ];

    // Check if have enough arguments are given 
    if ( argc < 2 ){
        printf( "Not enough argument." );
        exit( 0 );
    }
    dirport_num = atoi( argv[ 1 ] );
    
    // Creat socket		
    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    if (sockfd < 0){ 
        perror( "ERROR creating a socket" );
        exit( 0 ); 
    }
    printf( "Socket created for app_client..\n" );
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons( 0 );
     
    // Bind socket	
    if ( bind( sockfd, (struct sockaddr *) &serv_addr, sizeof( serv_addr ) ) < 0 ){ 
        perror( "ERROR binding\n" );
	    exit( 0 );
    }
 
    // Get the host name of the machine.
    if ( gethostname( hostname, sizeof hostname ) < 0 ){
        perror( "ERROR getting host name\n");
        exit( 0 );   
    }

    server = gethostbyname( hostname );
    if ( server == NULL ){
        perror( "Error gethostbyname\n " );
        exit( 0 );
    }
    serverip = inet_ntoa(*(struct in_addr *)server->h_addr);
	
    // Get the port number of the machine
    if ( getsockname( sockfd, (struct sockaddr *) &serv_addr, &serverlen ) < 0 ){
        perror( "ERROR getting sock information\n");
        exit( 0 );   
    }
    port_num = serv_addr.sin_port;
    sprintf( serverport_num, "%d", port_num );
    printf( "Socket bound to port %s..\n", serverport_num );
     	
    // Print <ip_address>, <port>
    printf( "ip address: %s, port number: %s\n",serverip, serverport_num ); 
    
    
    // Creat a new socket for dir_server		
    sockfd1 = socket( AF_INET, SOCK_STREAM, 0 );
    if (sockfd < 0){ 
        perror( "ERROR creating a socket" );
        exit( 0 ); 
    }
    printf( "Socket created for dir server..\n" );
    	    
     	
    // Try to make a TCP connection with dir_server
    dirserver = gethostbyname( diraddr );
    if ( dirserver == NULL ){
       printf( "Server does not exist.\n" );
       exit( 0 );
    }

    bzero((char *) &dir_addr, sizeof(dir_addr));
    dir_addr.sin_family = AF_INET;
    bcopy((char *)dirserver->h_addr,
         (char *)&dir_addr.sin_addr.s_addr,
         dirserver->h_length);
    dir_addr.sin_port = dirport_num;

    // Connect with dir server
    if ( connect( sockfd1, (struct sockaddr *) &dir_addr, sizeof( dir_addr ) ) < 0 ){
        perror( "ERROR connecting fail.\n" );
        exit( 0 );
    }
        		
    // Make and send message
    strcat( registration, serverip );
    strcat( registration, " " );	
    strcat( registration, serverport_num );
    strcat( registration, "\r\n" );		
    send( sockfd1, &registration, sizeof( registration ), MSG_WAITALL );
    	
    // Receive message
    state = recv( sockfd1, &resp, sizeof( resp ), MSG_WAITALL );
    if ( state < 0 ){ 
        perror( "ERROR reading from socket\n" );
        exit( 0 );
    }
    
    // Print out received message
    for( i = 0; i < strlen( resp ); i++ ){
        if( resp[ i ] != '\r' ){
            printf( "%c", resp[ i ] );
        }
    }
    close(sockfd1); 	
    	
    // Listen
    if ( listen( sockfd, 1 ) < 0 ){
        perror( "ERROR listening\n" );
        exit( 0 ); 
    }

    // Accept
    newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, &clientlen );
    if ( newsockfd < 0 ){
        perror( "ERROR accepting\n" ); 
        exit( 0 );
    }
    inet_ntop( AF_INET, &( cli_addr.sin_addr ), str, INET_ADDRSTRLEN ); 	
    printf( "Client %s connected..\n", str );	

    // Receive the message from the client.
    char temp[] = "Finish\r\n"; 
	char temp1[] = "Close\r\n";  
    while( 1 ){
        char data[ 500 ];
        // Receive message from client
        state = recv( newsockfd, &data, sizeof( data ), MSG_WAITALL );
        if ( state < 0 ){ 
            perror( "ERROR reading from socket\n" );
            exit( 0 );
        }
        if ( strcmp( temp, data ) == 0 ){
            char res[ 50 ] = "Ack\r\n";
            send( newsockfd, &res, sizeof( res ), MSG_WAITALL );
			for ( i = 0; i < strlen(data); i++ ){
				if ( data[ i ] != '\r' ){
					printf( "%c", data[ i ] );
				}		
			}	
        }
		else if ( strcmp( temp1, data ) == 0 ){
			for ( i = 0; i < strlen(data); i++ ){
				if ( data[ i ] != '\r' ){
					printf( "%c", data[ i ] );
				}		
			}
			close( newsockfd );
			break;
		}	 
    } 	 	  		
    return 0; 
}


