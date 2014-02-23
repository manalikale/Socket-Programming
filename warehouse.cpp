#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<netinet/in.h>
#include<errno.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<iostream>
#include<fcntl.h>
#include<fstream>
#include<vector>
#include<wait.h>
#include<sstream>

#include "device.h"

using namespace std;

#define BACKLOG 5;//Number of pending connections queue will hold

int main(void)
{
	int items_aggregate[3]={0,0,0};//Store vector
	int Cpipe[2];
	int sockfd, confd, bytes, nbytes,lval,bval;
	int store_count=0;
	char addr[INET_ADDRSTRLEN];
	pid_t cpid;
	socklen_t client_length;
	
	Devices deviceDetails;
	deviceDetails.no_of_cameras=0;
	deviceDetails.no_of_laptops=0;
	deviceDetails.no_of_printers=0;

	struct sockaddr_in client_addr, server_addr;
	struct hostent *he;

	/* TCP Server socket creation for warehouse */

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd<0)// Error handling for socket()
	{
		perror("Socket error");
	}

	/* Parameter Initialization */

	he=gethostbyname("nunki.usc.edu");
	server_addr.sin_family = AF_INET;
	he = (struct hostent *)gethostbyname("nunki.usc.edu");
	server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	server_addr.sin_port = htons(21966);
	
	/*Binding the Server socket with address obtained and port number assigned*/

	bval=bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)); 
	if(bval<0)// Error handling for bind()
	{
		perror("Bind error");
	}
	
	lval=listen(sockfd, 5); //Queue will hold a max of 5 connection requests
    if(lval<0)// Handling error condition for Listen function
	{
		perror("Listen error");
	}

	/* Converting IP from numeric to presentaion format for displaying*/
	
	inet_ntop(server_addr.sin_family, &server_addr.sin_addr, addr, sizeof addr);
	int port;
	port=ntohs(server_addr.sin_port);

	cout<<"Phase 1: The central warehouse has TCP port number "<<port<<" and IP address "<<addr<<endl;

        while(true)// Loop till data from all four stores is accepted
		{ 
			client_length = sizeof(client_addr);
			confd = accept(sockfd, (struct sockaddr *) &client_addr, &client_length);// Accept the connection for particular store(client_addr
			if(confd<0)// Error handling for accept()
			{
				perror("Accept error");
				continue;
			}
			store_count++;
			pipe(Cpipe);// Creating a PIPE for communication between the warehouse(parent) and a store(child) process

			inet_ntop(client_addr.sin_family, &client_addr.sin_addr, addr, sizeof addr);// Converting IP of store(client_addr) from numeric to readable format
			int status;
			Devices detailsFromStore;
			
			/*Reused code from Prof. Bill Cheng's slide5 of lec7*/
			if((cpid=fork())==0)// Child Process code(store)
			{
				close(sockfd);// Close listening for child (store)
				close(Cpipe[0]);// Close read end of pipe for child(store)
				Devices deviceDetailsrecvd;
				bytes = recv(confd,(void *)&deviceDetailsrecvd,100, 0);// Receive data from store

				cout<<"Phase 1: The central warehouse received information from store "<<store_count<<endl;
				write(Cpipe[1],(char *)&deviceDetailsrecvd,100);// Write the data received on to the PIPE
				close(Cpipe[1]);// Close the write end of the PIPE 
				exit(0);
			}

			else
			{
				close(Cpipe[1]);// Close the write end of PIPE for parent
				Devices detailsFromStore; 
				if(wait(&status)>0)// Wait for child to finish  
				nbytes=read(Cpipe[0],(void *)&detailsFromStore,100);// Read the data from read end of PIPE
				
				// Update the total number of items_aggregate as received
				items_aggregate[0]=items_aggregate[0]+detailsFromStore.no_of_cameras;
				items_aggregate[1]=items_aggregate[1]+detailsFromStore.no_of_printers;
				items_aggregate[2]=items_aggregate[2]+detailsFromStore.no_of_laptops;
			}

			if(store_count==4)// Check whether all data is received i.e., from all stores
			break;

		} 

		for(int i=0; i<3;i++)// Update the items_aggregate i.e. truck vector is generated.
			if(items_aggregate[i]>0)
			{
				items_aggregate[i]=0;
			}
			else
		
				items_aggregate[i]=-items_aggregate[i];	 
				cout<<"End of Phase 1 for the central warehouse"<<endl;	  
				int cval=close(sockfd);// Close the TCP socket

				if(cval<0)//Handling close Error
				{
					perror("Close error");
				} 
			
	/*Sending to Store1*/

	/* Socket creation for UDP transfer*/ 
		
	 struct sockaddr_in udp_client_addr, udp_server_addr;	
	 int udpsockfd= socket(AF_INET, SOCK_DGRAM, 0);
	
	/* IP and Port declarations */ 
	 
	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_client_addr.sin_family = AF_INET;
	 udp_client_addr.sin_port = htons(5966);
	 udp_client_addr.sin_addr = *((struct in_addr *)he->h_addr);	

 	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_server_addr.sin_family = AF_INET;
	 udp_server_addr.sin_port = htons(31966);
	 udp_server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	
	
    /* Bind the socket with IP address and Port number*/

	 bval=bind(udpsockfd, (struct sockaddr *)&udp_server_addr, sizeof(udp_server_addr));
		if(bval<0)// Handling error condition for bind()
		{
		 perror("Bind error");
		} 

     /*Converting into presentation format from	numeric format*/ 

	 inet_ntop(udp_server_addr.sin_family, &udp_server_addr.sin_addr, addr, sizeof addr);
	 int port1;
	 port1=ntohs(udp_server_addr.sin_port);
	 
	 cout<<"Phase 2: The central warehouse has UDP port number "<<port1<<" and IP address "<<addr<<endl;
	 
	 client_length=sizeof(udp_client_addr);
	 Devices Store1;// The data to be sent is stored in this structure object
	 Store1.no_of_cameras=items_aggregate[0];
	 Store1.no_of_laptops=items_aggregate[1];
	 Store1.no_of_printers=items_aggregate[2];
	 
     cout<<"Phase 2: Sending the truck-vector to outlet store store_1. The truck vector value is <"<<items_aggregate[0]<<","<<items_aggregate[1]<<","<<items_aggregate[2]<<">."<<endl;	
	 
	 /* Sending the data to store1 */

	 int sval = sendto(udpsockfd,(char *)&Store1, sizeof(Store1),0, (struct sockaddr *)& udp_client_addr, client_length); 
	 if(sval<0)// Handling error in send function
	{
		perror("Error in sending");
	}
	 cval=close(udpsockfd);// Close the UDP socket created
	 if(cval<0)//Handling close Error
	{
		perror("Error in closing UDP socket");
	}

	/*Receive data from store4*/

	/* Socket Creation for UDP transfer*/

     udpsockfd = socket(AF_INET, SOCK_DGRAM, 0 );

	 /* IP and Port declarations below */   
	 
	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_client_addr.sin_family = AF_INET;
 	 udp_client_addr.sin_port = htons(20966);
	 udp_client_addr.sin_addr = *((struct in_addr *)he->h_addr);
	
 	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_server_addr.sin_family = AF_INET;
	 udp_server_addr.sin_port = htons(32966);
	 udp_server_addr.sin_addr = *((struct in_addr *)he->h_addr);
 	
	 /* Binding socket with IP address and port number*/ 

	 bval=bind(udpsockfd, (struct sockaddr *) &udp_server_addr, sizeof(udp_server_addr));
	 if(bval<0)// Handling error condition for bind function
	{
		 perror(" Bind error");
	} 

	 /*Converting into presentation format from	numeric*/ 

	 inet_ntop(udp_server_addr.sin_family, &udp_server_addr.sin_addr, addr, sizeof addr);
	 int port2;
	 port2=ntohs(udp_server_addr.sin_port);
	 
	 cout<<"Phase 2: The central warehouse has UDP port number "<<port2<<" and IP address "<<addr<<endl;
	 
	 /*Converting into presentation format from	numeric*/

	 client_length = sizeof(udp_client_addr);
	 Devices Store4;
	 
	 /*Receiving data from Store4*/
	 int rval = recvfrom(udpsockfd, (void *)&Store4 , sizeof(Store4), 0, (struct sockaddr *) &udp_client_addr, &client_length);
	 if(rval<0)// Handling error in recvfrom()
	 {
		perror("Error in receiving data");
     }

	 cout<<"Phase 2: The final truck-vector is received from the outlet store store 4, the truck-vector value is: <"<<Store4.no_of_cameras<<","<<Store4.no_of_laptops<<","<<Store4.no_of_printers<<">"<<endl;
	
	cout<<"End of Phase 2 for the central warehouse"<<endl;

	cval=close(udpsockfd);// Close the UDP socket
	if(cval<0)//Handling close Error
	 {
		perror("Error in closing socket");
	 }
	 
	 return 0;
	 
}







