#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <arpa/inet.h>

struct registration{
    char ip_addr[ 20 ];
    char port_num[ 20 ]; 
};


int main(int argc, char *argv[]){
    int sockfd, newsockfd, dirport_num, state, i, j, count = 0, index;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clientlen = sizeof( cli_addr );
    char message[ 50 ];
    char str[ INET_ADDRSTRLEN ];   
    char temp[ 8 ];
    char reg[] = "register";
    char fail[ 50 ] = "failure\r\n"; 
    char success[ 50 ] = "success\r\n"; 
    char list_serv[] = "list-servers\r\n";
    struct registration info[ 50 ];

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
    printf( "Socket created..\n" );
     
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = dirport_num;

    // Bind socket 
    if ( bind( sockfd, (struct sockaddr *) &serv_addr, sizeof( serv_addr ) ) < 0 ){ 
        perror( "ERROR binding\n" );
        exit( 0 );
    }
    printf( "Socket bound to port %d..\n", dirport_num );
    
    // Listen
    if ( listen( sockfd, 1 ) < 0 ){
        perror( "ERROR listening\n" );
        exit( 0 ); 
    }     
    while ( 1 ){

        // Accept
        newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, &clientlen );
        if ( newsockfd < 0 ){
            perror( "ERROR accepting\n" ); 
            exit( 0 );
        }
        inet_ntop( AF_INET, &( cli_addr.sin_addr ), str, INET_ADDRSTRLEN );    
        printf( "Client %s connected..\n", str );       
         
        // Receive message
        state = recv( newsockfd, &message, sizeof( message ), MSG_WAITALL );
        if ( state < 0 ){ 
           perror( "ERROR reading from socket\n" );
           exit( 0 );
        }
         
        // Print out received message
        for( i = 0; i < strlen( message ); i++ ){
            if( message[ i ] != '\r' ){
               printf( "%c", message[ i ] );
            }
        }

        // Check if the message is register message or list-servers message by examine first 8 letter of the message
        for( i = 0; i < 8; i++ ){
            temp[ i ] = message[ i ];
        }

        // Message is register <ipaddr> <port>
        if ( strcmp( temp, reg ) == 0 ){
            char tempip[ 20 ];
            char portnum[ 20 ];

            // Storing ip into a temp char array            
	    j = 0;
            for( i = 9; i < 50; i++ ){
                if ( message[ i ] == ' ' ){
                    break; 
                } 
		tempip[ j ] = message[ i ];
                j++; 
            }
	    tempip[ j ] = '\0';
		
            // Check if the new ip is already registed. If already existed, respond with failure message
            j = 0;
            for (index = 0; index < count; index++ ){
                if ( strcmp( info[ index ].ip_addr, tempip ) == 0 ){ 
                    send( newsockfd, &fail, sizeof( fail ), 0 );
                    break; 
                }
            }    

            // Storing port number end with \r into a temp char array
            j = 0;
            for( i = i + 1; i < 50; i++ ){
                if ( message[ i ] == '\n' ){
                    break; 
                } 
                portnum[ j ] = message[ i ];
                j++; 
            }
	    portnum[ j ] = '\0';	
					
            /* Storing ip and port number */
            // count is the number of registration stored. 
            strcpy( info[ count ].ip_addr , tempip );
            strcpy( info[ count ].port_num , portnum );
            count++;
	    // Send out success message 
            send( newsockfd, &success, sizeof( success ), 0 );		 
         }
        // Message is list-server Message
        else{
            if( count <= 0 ){
                send( newsockfd, &fail, sizeof( fail ), 0 );
            }
            // Message starts with success\r, so 8 
            char res[ 100 ] = "success\r";
            for ( i = 0; i < count; i++ ){	
                // Exp: "success\r128.101.37.1"
                strcat( res, info[ i ].ip_addr );
                strcat( res, " " );
                // Exp: "success\r128.101.37.1 9123\r"
                strcat( res, info[ i ].port_num );    
            }
            // Exp: "success\r128.101.37.1 9123\r128.101.38.1 9321\r\n"
            strcat( res, "\n" );
            send( newsockfd, &res, sizeof( res ), 0 );
        }               
    }   
    return 0;         
}


