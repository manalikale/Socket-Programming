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

int item_aggregate[3];//Store vector

/*Function for updating Truck vector and Store vector*/
Devices updation(Devices rcvdStore1)
{
      int Truck[3]={0,0,0};
      Truck[0]=rcvdStore1.no_of_cameras;
	  Truck[1]=rcvdStore1.no_of_laptops;
	  Truck[2]=rcvdStore1.no_of_printers;

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

	 rcvdStore1.no_of_cameras=Truck[0];
	 rcvdStore1.no_of_laptops=Truck[1];
	 rcvdStore1.no_of_printers=Truck[2];
	 
   return rcvdStore1; 
}

int main(void)
{
 /* Variable declaration */
 
     
	 int sockfd;
	 char addr[INET_ADDRSTRLEN];
	 int store_count=0;
	 pid_t cpid;
	 socklen_t client_length;
	 struct sockaddr_in client_addr, server_addr;
	 struct hostent *he;
     string Line;
	 ifstream input;
	 input.open("Store-3.txt");
	
     Devices details;

	 /*Code reused from www.stackoverflow.com*/
	 while(input>>Line)// Reading inputs from file store-3.txt
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

	/* Creating TCP socket */

	 sockfd = socket(AF_INET, SOCK_STREAM, 0);
	 
	/* Initializing IP address and Port number parameters */

	 he=gethostbyname("nunki.usc.edu");
	 server_addr.sin_family = AF_INET;
	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 server_addr.sin_port = htons(21966);

	 /*Connect to the server(Warehouse) */ 

	 int connValue=connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	 if(connValue<0)// Handling error for connect function
	 {
		perror("Connection error");
	 }

    //Send data to the Warehouse	
     int sval=send (sockfd,(char *)&details,100,0);
	 if(sval<0)// Handling error in send function
	 {
		perror("Error in sending data");
	 }

	 client_length= sizeof(client_addr); 
	 
	/* Get client information(store information), connected to the socket*/ 
	 getsockname(sockfd,(struct sockaddr *)&client_addr,&client_length);
	 
	/* Convert IP address from numeric format to readable format*/ 

	 inet_ntop(client_addr.sin_family,&client_addr.sin_addr,addr,sizeof(addr));
	 
	 int port11;
	 port11=ntohs(client_addr.sin_port);

	 cout<<"Phase 1: store_3 has TCP port number "<<port11<<" and IP address "<<addr<<endl;

	 cout<<"Phase 1: The outlet vector <"<<item_aggregate[0]<<","<<item_aggregate[1]<<","<<item_aggregate[2]<<"> for the store_3 has been sent to the central warehouse"<<endl;

	 cout<<"End of Phase 1 for store 3"<<endl; 

	 int cval=close(sockfd);// Close the TCP socket created
	 if(cval<0)//Handling close Error
	 {
		perror("Error in closing socket");
	 }

	/*Receive Data from Store2*/

	/* Creating UDP socket */

	 int udpsockfd= socket(AF_INET, SOCK_DGRAM, 0);

	/* Initializing IP address and Port number parameters */  

	 struct sockaddr_in udp_server_addr,udp_client_addr;
	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_client_addr.sin_family = AF_INET;
	 udp_client_addr.sin_port = htons(13966);
	 udp_client_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 
	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_server_addr.sin_family = AF_INET;
	 udp_server_addr.sin_port = htons(10966);
	 udp_server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 
	 

	 Devices receivedFromStore2;

	/* Binding the socket to IP address and port number*/

	 int bindValue=bind(udpsockfd, (struct sockaddr *) &udp_client_addr, sizeof(udp_client_addr));
	 if(bindValue<0)// Handling error in bind function
	 {
		perror("Bind error");
	 }

	 client_length= sizeof(udp_client_addr); 

	/* Getting the port number and IP address for display below*/

	 getsockname(udpsockfd,(struct sockaddr *)&udp_client_addr,&client_length);
	 inet_ntop(udp_client_addr.sin_family,&udp_client_addr.sin_addr,addr,sizeof(addr));
	 client_length=sizeof(udp_server_addr);
	 int port12;
	 port12=ntohs(udp_client_addr.sin_port);
	 cout<<"Phase 2: Store_3 has UDP port "<<port12<<" and IP address "<<addr<<endl;

	/* Receiving data from Store 2*/

	 int rval = recvfrom(udpsockfd, (void *)&receivedFromStore2 , sizeof(receivedFromStore2), 0, (struct sockaddr *) &udp_server_addr, &client_length);
	 if(rval<0)// Handling error condition in recvfrom
	 { 
	   perror("Error in Receiving data");
	 }

	 cout<<"Phase 2: Store_3 received the truck-vector <"<<receivedFromStore2.no_of_cameras<<","<<receivedFromStore2.no_of_laptops<<","<<receivedFromStore2.no_of_printers<<"> from store 2"<<endl; 
	 cval=close(udpsockfd);// Close the UDP socket created

	 if(cval<0)//Handling close Error
	 {
	   perror("Closing socket error");
	 }

	/* Send Data to Store4*/

	/* Creating UDP socket */

	 udpsockfd= socket(AF_INET, SOCK_DGRAM, 0);
	 Devices sendToStore4;

	/* Initializing IP address and Port number parameters */  

 	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_client_addr.sin_family = AF_INET;
	 udp_client_addr.sin_port = htons(14966);
	 udp_client_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 
 	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 udp_server_addr.sin_port = htons(17966);
	 udp_server_addr.sin_family = AF_INET;
	 

	/* Binding the socket to IP address and port number */

	 bindValue=bind(udpsockfd, (struct sockaddr *) &udp_client_addr, sizeof(udp_client_addr));
	 if(bindValue<0)// Handling error in bind function
	 {
		perror("Bind error");
	 }

	 client_length= sizeof(udp_client_addr); 
	 sendToStore4=updation(receivedFromStore2);// Update the truck after receiving from store 2

	/* Getting the port number and IP address for display below*/ 

	 getsockname(udpsockfd,(struct sockaddr *)&udp_client_addr,&client_length);
	 inet_ntop(udp_client_addr.sin_family,&udp_client_addr.sin_addr,addr,sizeof(addr));
	 
	 int port13;
	 port13=ntohs(udp_client_addr.sin_port);
	 cout<<"Phase 2: Store_3 has UDP port "<<port13<<" and IP address "<<addr<<endl;
	 cout<<"Phase 2: The updated truck-vector <"<<sendToStore4.no_of_cameras<<","<<sendToStore4.no_of_laptops<<","<<sendToStore4.no_of_printers<<"> has been sent to store_4\nPhase 2: Store_3 updated outlet-vector is <"<<item_aggregate[0]<<","<<item_aggregate[1]<<","<<item_aggregate[2]<<">"<<endl;

	/* Sending Data to Store4*/

	 sval = sendto(udpsockfd, (char *)&sendToStore4, sizeof(sendToStore4), 0, (const struct sockaddr *) & udp_server_addr, sizeof(udp_server_addr));
	 if(sval<0)// Handling error condition in sendto
	 { 
	  perror("Error in sending Data");
	 } 

	 cval=close(udpsockfd);// Close the UDP socket
	 if(cval<0)//Handling close Error
	 {
		perror("Close error");
	 }


	/*Receive From Store2*/

    /* Creating UDP socket */	 

	 udpsockfd= socket(AF_INET, SOCK_DGRAM, 0);

	/* Initializing IP address and Port number parameters */   
	
	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_client_addr.sin_family = AF_INET;
	 udp_client_addr.sin_port = htons(15966);
	 udp_client_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 
	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_server_addr.sin_family = AF_INET;
	 udp_server_addr.sin_port = htons(11966);
	 udp_server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	
	
	/* Binding the socket to IP address and port number  */ 

	 bindValue=bind(udpsockfd, (struct sockaddr *) &udp_client_addr, sizeof(udp_client_addr));
	 if(bindValue<0)// Handling error in bind function
	 {
		perror("Bind Error");
	 } 

	 client_length= sizeof(udp_client_addr); 

	/* Obtaining the port number and IP address for display below*/ 

	 getsockname(udpsockfd,(struct sockaddr *)&udp_client_addr,&client_length);
	 inet_ntop(udp_client_addr.sin_family,&udp_client_addr.sin_addr,addr,sizeof(addr));
	 client_length = sizeof(udp_server_addr);
	 
	 int port14;
	 port14=ntohs(udp_client_addr.sin_port);
	 cout<<"Phase 2: Store_3 has UDP port "<<port14<<" and IP address "<<addr<<endl; 

	/* Receiving Data from Store 2 */

	 rval = recvfrom(udpsockfd, (void *)&receivedFromStore2 , sizeof(receivedFromStore2), 0, (struct sockaddr *) &udp_server_addr, &client_length);
	 if(rval<0)// Handling error in revfrom
	 {
		perror("Error in Receiving data");
     }

     cout<<"Phase 2: truck-vector <"<<receivedFromStore2.no_of_cameras<<","<<receivedFromStore2.no_of_laptops<<","<<receivedFromStore2.no_of_printers<<"> has been received from Store_2"<<endl;
	 sendToStore4=updation(receivedFromStore2);// Update the truck after receiving from store 2	 

	 cval=close(udpsockfd);// Close the UDP socket
     if(cval<0)//Handling close Error
	{
	  perror("Error in closing socket");
	}

	/*Sending Data to Store4*/

	/* Creating UDP socket */
	 udpsockfd= socket(AF_INET, SOCK_DGRAM, 0);
	 
	/* Initializing IP address and Port number parameters */  

 	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_client_addr.sin_family = AF_INET;
	 udp_client_addr.sin_port = htons(16966);
	 udp_client_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 
	 he = (struct hostent *)gethostbyname("nunki.usc.edu");
	 udp_server_addr.sin_family = AF_INET;
	 udp_server_addr.sin_port = htons(19966);
	 udp_server_addr.sin_addr = *((struct in_addr *)he->h_addr);
 	 
	/*Binding the socket to IP address and port number   */

	 bindValue=bind(udpsockfd, (struct sockaddr *) &udp_client_addr, sizeof(udp_client_addr));
	 if(bindValue<0)// Handling error in bind function
	 {
		perror("Bind error");
	 }

	 client_length= sizeof(udp_client_addr); 

	/* Obtaining the port number and IP address for display below*/ 

	 getsockname(udpsockfd,(struct sockaddr *)&udp_client_addr,&client_length);
	 inet_ntop(udp_client_addr.sin_family,&udp_client_addr.sin_addr,addr,sizeof(addr));
	 
	 int port15;
	 port15=ntohs(udp_client_addr.sin_port);
	 cout<<"Phase 2: Store_3 has UDP port "<<port15<<" and IP address "<<addr<<endl; 

	/* Sending data to Store 4*/

	 sval = sendto(udpsockfd, (char *)&sendToStore4, sizeof(sendToStore4), 0, (const struct sockaddr *) & udp_server_addr, sizeof(udp_server_addr));
	 if(sval<0)// Handling error condition in sendto
	 { 
	    perror("Error in Sending data");
	 }

	 cout<<"Phase 2: The updated truck-vector <"<<sendToStore4.no_of_cameras<<","<<sendToStore4.no_of_laptops<<","<<sendToStore4.no_of_printers<<"> has been sent to store_4\nPhase 2: Store_3 updated outlet-vector is <"<<item_aggregate[0]<<","<<item_aggregate[1]<<","<<item_aggregate[2]<<">"<<endl;
	 cout<<"End of Phase 2 for store_3"<<endl;

	 
	 /* Close the UDP socket created*/
	 cval=close(udpsockfd);
	 if(cval<0)//Handling close Error
	 {
		perror("Closing error");
	 }

	return 0;

}


