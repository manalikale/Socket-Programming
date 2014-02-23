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

int item_aggregate[3]; //Store vector

/*Function for updating Truck vector and Store vector*/
Devices updation(Devices rcvdWare)
{
     int Truck[3]={0,0,0}; //Truck vector
     Truck[0]=rcvdWare.no_of_cameras;
	 Truck[1]=rcvdWare.no_of_laptops;
	 Truck[2]=rcvdWare.no_of_printers;

	 for(int i=0;i<3;i++)
     if(item_aggregate[i] + Truck[i] >= 0)
	{
	    Truck[i]=Truck[i] + item_aggregate[i];
	    item_aggregate[i]=0;
	}
	 else
	{
	   item_aggregate[i]=item_aggregate[i]+Truck[i];
       Truck[i]=0;	  
	}

	 rcvdWare.no_of_cameras=Truck[0];
	 rcvdWare.no_of_laptops=Truck[1];
	 rcvdWare.no_of_printers=Truck[2];

   return rcvdWare; 

}

int main(void)

{
    /* Variable declaration */ 

	 int sockfd,sval;
	 char addr[INET_ADDRSTRLEN];
	 int store_count=0;
	 pid_t cpid;
	 socklen_t client_length;
     string Line;
 	 ifstream input;
	 input.open("Store-1.txt");
	 
	 struct sockaddr_in client_addr, server_addr;
	 struct hostent *he;

     Devices details;

	 /*Code reused from www.stackoverflow.com*/
	 while(input>>Line)// Reading inputs from file store-1.txt
	 {   
		store_count++;
		if(store_count%2==0)
		{
	         stringstream(Line)>>item_aggregate[store_count/2 -1];
		}        
	 }
	     
	 details.no_of_cameras=item_aggregate[0];
	 details.no_of_laptops=item_aggregate[1];
	 details.no_of_printers=item_aggregate[2];

	/* Creating the TCP socket */

	 sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/* Initializing IP address and Port number parameters */

	 he = (struct hostent *)gethostbyname("nunki.usc.edu");

	 server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 server_addr.sin_family = AF_INET;
	 server_addr.sin_port = htons(21966);

	/*Connect to the server(Warehouse)*/
	
	 int cval=connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	 if(cval<0)// Handling connection error
	 {
		perror("connect error");	
	 }

	/*Send data to the Warehouse */ 

     sval = send (sockfd,(char *)&details,200,0);
	 if(sval<0)// Handling error in send function
	 {
		perror("Send error");
	 }

	 client_length= sizeof(client_addr); 

	/* Get client information(store information), connected to the socket*/ 

	 getsockname(sockfd,(struct sockaddr *)&client_addr,&client_length);

	/* Convert IP address from numeric format to readable format*/ 

	 inet_ntop(client_addr.sin_family,&client_addr.sin_addr,addr,sizeof(addr));
	 int port;
	 port=ntohs(client_addr.sin_port);
	 
	 cout<<"Phase 1: Store 1 has TCP port number "<<port<<" and IP address "<<addr<<endl;
	 cout<<"Phase 1: The outlet vector <"<<details.no_of_cameras<<","<<details.no_of_laptops<<","<<details.no_of_printers<<">"<<endl;
	 cout<<"End of Phase 1 for store 1"<<endl;

	 int ccval=close(sockfd);// Close the TCP socket created
  	 if(ccval<0)//Handling close Error
	 {
		perror("Close socket error");
	 } 

	/*Receive Data from Warehouse*/

     int udpsockfd = socket(AF_INET, SOCK_DGRAM, 0);
	/* Initializing IP address and Port number parameters */  

     struct sockaddr_in udp_client_addr, udp_server_addr;    

	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_client_addr.sin_family = AF_INET;
	 udp_client_addr.sin_port = htons(5966);
	 udp_client_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 
	 he = (struct hostent *)gethostbyname("nunki.usc.edu"); 
	 udp_server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 udp_server_addr.sin_port = htons(31966);
	 udp_server_addr.sin_family = AF_INET;
	 

	/*Binding the socket to IP address and port number  */

	 int bval=bind(udpsockfd, (struct sockaddr *) &udp_client_addr, sizeof(udp_client_addr));
	 if(bval<0)// Handling error in bind function
	 {
		perror("Bind error");
	 }

	 client_length=sizeof(udp_client_addr);

	/* Getting the port number and IP address for display below*/  

	 getsockname(udpsockfd,(struct sockaddr *)&udp_client_addr,&client_length);
	 inet_ntop(udp_client_addr.sin_family,&udp_client_addr.sin_addr,addr,sizeof(addr));
	 
	 int portt;
	 portt=ntohs(udp_client_addr.sin_port);
	 cout<<"Phase 2: Store_1 has UDP port "<<portt<<" and IP address "<<addr<<endl;

	 Devices rcvdWare;

	 socklen_t server_length = sizeof(udp_server_addr);

    /* Receiving data from Store2*/	

	 int rval = recvfrom(udpsockfd,(void *)&rcvdWare,sizeof(rcvdWare), 0,(struct sockaddr *)&udp_server_addr, &server_length);
	 if(rval<0)// Handling error condition in recvfrom
	 { 
	   perror("Error in Receiving Data");
	 } 

	 cout<<"Phase 2: Store_1 received the truck-vector <"<<rcvdWare.no_of_cameras<<","<<rcvdWare.no_of_laptops<<","<<rcvdWare.no_of_printers<<"> from the central warehouse."<<endl;

	 Devices sendToStore2=updation(rcvdWare);// Update the truck after receiving from Warehouse

	 cval=close(udpsockfd);// Close the UDP socket created
	 if(cval<0)//Handling close Error
	 {
		perror("Error in closing socket");
	 }


	/* Sending Data to Store2*/

	/* Creating UDP socket */ 

	 udpsockfd= socket(AF_INET, SOCK_DGRAM, 0);

	/* Initializing IP address and Port number parameters */    

 	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_client_addr.sin_family = AF_INET;
	 udp_client_addr.sin_port = htons(6966);
	 udp_client_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 
	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 udp_server_addr.sin_port = htons(9966);
	 udp_server_addr.sin_family = AF_INET;
	 

	/* Binding the socket to IP address and port number */

	 bval=bind(udpsockfd, (struct sockaddr *) &udp_client_addr, sizeof(udp_client_addr));
     if(bval<0)// Handling error in bind function
	 {
		perror("Bind error");
	 }

	 client_length=sizeof(udp_client_addr);

	/* Getting the port number and IP address for display below*/  

	 getsockname(udpsockfd,(struct sockaddr *)&udp_client_addr,&client_length);
	 inet_ntop(udp_client_addr.sin_family,&udp_client_addr.sin_addr,addr,sizeof(addr));
	
	int port3;
	 port3=ntohs(udp_client_addr.sin_port);

	 cout<<"Phase 2: Store_1 has UDP port "<<port3<<" and IP address "<<addr<<endl;
	 cout<<"Phase 2: The updated truck-vector <"<<sendToStore2.no_of_cameras<<","<<sendToStore2.no_of_laptops<<","<<sendToStore2.no_of_printers<<"> has been sent to store_2\nPhase 2: Store_1 updated outlet-vector is <"<<item_aggregate[0]<<","<<item_aggregate[1]<<","<<item_aggregate[2]<<">"<<endl;

	/* Sending Data to Store2 */

	 sval = sendto(udpsockfd, (char *)&sendToStore2, sizeof(sendToStore2), 0, (const struct sockaddr *) & udp_server_addr, sizeof(udp_server_addr));
	 if(sval<0)// Handling error condition in sendto
	 { 
	   perror("Error in Sending data");
	 }

	 cval=close(udpsockfd);// Close the UDP socket
	 if(cval<0)//Handling close Error
	 {
	  perror("Close socket error");
	 }

	/*Receiving Data from Store4*/

	/* Creating UDP socket */	 

     udpsockfd= socket(AF_INET, SOCK_DGRAM, 0);

	/* Initializing IP address and Port number parameters */    
	
	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_client_addr.sin_family = AF_INET;
	 udp_client_addr.sin_port = htons(7966);
	 udp_client_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 
 	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 udp_server_addr.sin_port = htons(18966);
	 udp_server_addr.sin_family = AF_INET;
	 

	/* Binding the socket to IP address and port number*/   	 

	 bval=bind(udpsockfd, (struct sockaddr *) &udp_client_addr, sizeof(udp_client_addr));
	 if(bval<0)// Handling error in bind function
	 {
		perror("Bind error");
	 } 	  

	 client_length=sizeof(udp_client_addr);

	/* Getting the port number and IP address for display below*/ 

	 getsockname(udpsockfd,(struct sockaddr *)&udp_client_addr,&client_length);
	 inet_ntop(udp_client_addr.sin_family,&udp_client_addr.sin_addr,addr,sizeof(addr));
	 
	 int port4;
	 port4=ntohs(udp_client_addr.sin_port);

	 cout<<"Phase 2: Store_1 has UDP port "<<port4<<" and IP address "<<addr<<endl;	 

	 Devices receivedFromStore4;

	/* Receiving Data from Store 4  */ 

	 rval = recvfrom(udpsockfd, (void *)&receivedFromStore4 , sizeof(receivedFromStore4), 0, (struct sockaddr *) &udp_server_addr, & client_length );
     if(rval<0)// Handling error in revfrom
	 {
		perror("Error in recvfrom");
     }  

	 cout<<"Phase 2: truck-vector <"<<receivedFromStore4.no_of_cameras<<","<<receivedFromStore4.no_of_laptops<<","<<receivedFromStore4.no_of_printers<<"> has been received from Store_4"<<endl;

	 sendToStore2=updation(receivedFromStore4);// Update the truck after receiving from store 4

     cval=close(udpsockfd);// Close the UDP socket
     if(cval<0)//Handling close Error
	 {
		perror("Error in close socket");
	 }

	/*Sending Data to Store2*/

	/* Creating UDP socket */
 	 udpsockfd= socket(AF_INET, SOCK_DGRAM, 0);

	/* Initializing IP address and Port number parameters */   
	 
	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_client_addr.sin_family = AF_INET;
	 udp_client_addr.sin_port = htons(8966);
	 udp_client_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 
	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 udp_server_addr.sin_port = htons(11966);
	 udp_server_addr.sin_family = AF_INET;
	 

	/* Binding the socket to IP address and port number */    

	 bval=bind(udpsockfd, (struct sockaddr *) &udp_client_addr, sizeof(udp_client_addr));
	 if(bval<0)// Handling error in bind function
	{
		perror("Bind error");
	}

	 /* Sending data to Store 3*/

	 sval = sendto(udpsockfd, (char *)&sendToStore2, sizeof(sendToStore2), 0, (const struct sockaddr *) & udp_server_addr, sizeof(udp_server_addr));
	 client_length=sizeof(udp_client_addr);

	/* Obtaining the port number and IP address for display below*/ 

	 getsockname(udpsockfd,(struct sockaddr *)&udp_client_addr,&client_length);
	 inet_ntop(udp_client_addr.sin_family,&udp_client_addr.sin_addr,addr,sizeof(addr));
	 int port5;
	 port5=ntohs(udp_client_addr.sin_port);

	 cout<<"Phase 2: Store_1 has UDP port "<<port5<<" and IP address "<<addr<<endl;

	 cout<<"Phase 2: The updated truck-vector <"<<sendToStore2.no_of_cameras<<","<<sendToStore2.no_of_laptops<<","<<sendToStore2.no_of_printers<<"> has been sent to store_2\nPhase 2: Store_1 updated outlet-vector is <"<<item_aggregate[0]<<","<<item_aggregate[1]<<","<<item_aggregate[2]<<">"<<endl;	

	 /* Close the UDP socket created*/
     cval=close(udpsockfd);
	 if(cval<0)//Handling close Error
	 {
		perror("Close socket error");
	 }
	 cout<<"End of Phase 2 for Store_1"<<endl;
	 
	 return 0;

}


