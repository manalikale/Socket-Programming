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

     struct hostent *he; 
	 int sockfd, rval;
	 char addr[INET_ADDRSTRLEN];
	 pid_t cpid;
	 socklen_t client_length;
	 struct sockaddr_in client_addr, server_addr, udp_server_addr, udp_client_addr;
	 string Line;
	 ifstream input;
	 input.open("Store-4.txt");
	 int store_count=0;
	 Devices details;

     int sval;
	
	/*Code reused from www.stackoverflow.com*/	 
	 while(input>>Line)// Reading inputs from file store-4.txt
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

	 he=gethostbyname("localhost");
	 server_addr.sin_family = AF_INET;
  	 he = (struct hostent *)gethostbyname("localhost");
	 server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 server_addr.sin_port = htons(21966);
    
	/*Connect to the server(Warehouse) */

	 int connValue=connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	 if(connValue<0)// Handling error for connect function
	 {
		perror("connect error");	
	 }

	 /*Send data to the Warehouse*/

     sval=send(sockfd,(char *)&details,100,0);
	 if(sval<0)// Handling error in send function
	 {
		perror("Error in sending data");
	 }

	 client_length= sizeof(client_addr); 

	 /* Get client information(store information), connected to the socket*/ 
	 getsockname(sockfd,(struct sockaddr *)&client_addr,&client_length);
	 
	/* Convert IP address from numeric format to readable format*/ 

	 inet_ntop(client_addr.sin_family,&client_addr.sin_addr,addr,sizeof(addr));
	
	int port16;
	 port16=ntohs(client_addr.sin_port);
	 cout<<"Phase 1: store_4 has TCP port number "<<port16<<" and IP address "<<addr<<endl;

	 cout<<"Phase 1: The outlet vector <"<<item_aggregate[0]<<","<<item_aggregate[1]<<","<<item_aggregate[2]<<"> for the store_4 has been sent to the central warehouse"<<endl;

	 cout<<"End of Phase 1 for store 4"<<endl; 

	 int cval=close(sockfd);// Close the TCP socket created
	 if(cval<0)//Handling close Error
	 {
	  perror("Closing error");
	 }

	/*Receive From Store3*/ 

	/* Creating UDP socket */

	 int udpsockfd= socket(AF_INET, SOCK_DGRAM, 0);

	/* Initializing IP address and Port number parameters */ 

  	 he = (struct hostent *)gethostbyname("localhost");
	 udp_client_addr.sin_family = AF_INET;
	 udp_client_addr.sin_port = htons(17966);
	 udp_client_addr.sin_addr = *((struct in_addr *)he->h_addr);
	  
	 he = (struct hostent *)gethostbyname("localhost");
	 udp_server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 udp_server_addr.sin_port = htons(14966);
	 udp_server_addr.sin_family = AF_INET;
	 

	/* Binding UDP socket to IP address and port number */

	 int bval=bind(udpsockfd, (struct sockaddr *) &udp_client_addr, sizeof(udp_client_addr));
	 if(bval<0)// Handling error in bind function
	 {
		perror("Bind error");
	 }

	 client_length= sizeof(udp_client_addr); 

	 /* Obtaining the port number and IP address for display */

	 getsockname(udpsockfd,(struct sockaddr *)&udp_client_addr,&client_length);
	 inet_ntop(udp_client_addr.sin_family,&udp_client_addr.sin_addr,addr,sizeof(addr));
	 client_length=sizeof(udp_server_addr);
	 
	 int port17;
	 port17=ntohs(udp_client_addr.sin_port);

	 cout<<"Phase 2: Store_4 has UDP port "<<port17<<" and IP address "<<addr<<endl;

	 Devices receivedFromStore3;

	 /* Receiving data from Store 3*/

	 rval = recvfrom(udpsockfd, (void *)&receivedFromStore3 , sizeof(receivedFromStore3), 0, (struct sockaddr *) &udp_server_addr, & client_length );
	 if(rval<0)// Handling error condition in recvfrom
	 { 
	  perror("Error in Receiving data");
	 }

     cout<<"Phase 2: Store_4 received the truck-vector <"<<receivedFromStore3.no_of_cameras<<","<<receivedFromStore3.no_of_laptops<<","<<receivedFromStore3.no_of_printers<<"> from store 3"<<endl;    

	 Devices sendToStore1=updation(receivedFromStore3);// Update the truck after receiving from store 3

	 cval=close(udpsockfd);// Close the UDP socket created
	 if(cval<0)//Handling close Error
	 {
	  perror("Close socket error");
	 }

	/*Send to Store1*/

	/* Creating UDP socket */

	 udpsockfd= socket(AF_INET, SOCK_DGRAM, 0);

	/* Initializing IP address and Port number parameters */ 

  	 he = (struct hostent *)gethostbyname("localhost");
	 udp_client_addr.sin_family = AF_INET;
	 udp_client_addr.sin_port = htons(7966);
	 udp_client_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 
	 he = (struct hostent *)gethostbyname("localhost");
	 udp_server_addr.sin_family = AF_INET;
	 udp_server_addr.sin_port = htons(18966);
	 udp_server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	
	/* Binding the socket to IP address and port number*/ 

	 bval=bind(udpsockfd, (struct sockaddr *) &udp_server_addr, sizeof(udp_server_addr));
	 if(bval<0)// Handling error in bind function
	 {
		perror("Bind Error");
	 }

     client_length= sizeof(udp_server_addr); 

	/* Getting the port number and IP address for display below*/

	 getsockname(udpsockfd,(struct sockaddr *)&udp_server_addr,&client_length);
	 inet_ntop(udp_server_addr.sin_family,&udp_server_addr.sin_addr,addr,sizeof(addr));
	 
	 int port18;
	 port18=ntohs(udp_server_addr.sin_port);

	 cout<<"Phase 2: Store_4 has UDP port "<<port18<<" and IP address "<<addr<<endl;

	 cout<<"Phase 2: The updated truck-vector <"<<sendToStore1.no_of_cameras<<","<<sendToStore1.no_of_laptops<<","<<sendToStore1.no_of_printers<<"> has been sent to store_1\nPhase 2: Store_4 updated outlet-vector is <"<<item_aggregate[0]<<","<<item_aggregate[1]<<","<<item_aggregate[2]<<">"<<endl;

	/* Sending Data to Store 1*/

	 sval = sendto(udpsockfd, (void *)&sendToStore1, sizeof(sendToStore1), 0, (const struct sockaddr *) & udp_client_addr, sizeof(udp_client_addr));

	 if(sval<0)// Handling error condition in sendto
	 { 
	   perror("Error in sending data");
	 }

	 cval=close(udpsockfd);// Close the UDP socket
     if(cval<0)//Handling close Error
	 {
		perror("Close socket error");
	 }

	 
	/* Receive from Store3*/

    /* Creating UDP socket */	

	 udpsockfd= socket(AF_INET, SOCK_DGRAM, 0);

	/* Initializing IP address and Port number parameters */  

 	 he = (struct hostent *)gethostbyname("localhost");
	 udp_client_addr.sin_family = AF_INET;
	 udp_client_addr.sin_port = htons(16966);
	 udp_client_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 
	 he = (struct hostent *)gethostbyname("localhost");
	 udp_server_addr.sin_family = AF_INET;
	 udp_server_addr.sin_port = htons(19966);
	 udp_server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 
	/*Binding the socket to IP address and port number  */

	 bval=bind(udpsockfd, (struct sockaddr *) &udp_server_addr, sizeof(udp_server_addr));
	 if(bval<0)// Handling error in bind function
	 {
	  perror("Bind error");
	 }

	 client_length= sizeof(udp_server_addr); 

	 
	/* Getting the port number and IP address for display below*/

	 getsockname(udpsockfd,(struct sockaddr *)&udp_server_addr,&client_length);

	 inet_ntop(udp_server_addr.sin_family,&udp_server_addr.sin_addr,addr,sizeof(addr));
	
	int port19;
	 port19=ntohs(udp_server_addr.sin_port);

	 cout<<"Phase 2: Store_4 has UDP port "<<port19<<" and IP address "<<addr<<endl;
	 client_length=sizeof(udp_client_addr);

	/* Receiving Data from Store 3*/

	 rval = recvfrom(udpsockfd, (void *)&receivedFromStore3 , sizeof(receivedFromStore3), 0, (struct sockaddr *) &udp_client_addr, &client_length);
	 if(rval<0)// Handling error in revfrom
	 {
	  perror("Error in receiving data");
     }

	 cout<<"Phase 2: truck-vector <"<<receivedFromStore3.no_of_cameras<<","<<receivedFromStore3.no_of_laptops<<","<<receivedFromStore3.no_of_printers<<"> has been received from Store_3"<<endl;

	 Devices sendWare=updation(receivedFromStore3); // Update the truck after receiving from store 3	 

     cval=close(udpsockfd);// Close the UDP socket
	 if(cval<0)//Handling close Error
	 {
	  perror("Close socket error");
	 }

	
	/*Sending Data to Warehouse*/

	/* Creating UDP socket */	

	 udpsockfd= socket(AF_INET, SOCK_DGRAM, 0);
	/* Initializing IP address and Port number parameters */   

	 he = (struct hostent *)gethostbyname("localhost");
	 udp_client_addr.sin_family = AF_INET;
	 udp_client_addr.sin_port = htons(32966);
	 udp_client_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 
  	 he = (struct hostent *)gethostbyname("localhost");
	 udp_server_addr.sin_port = htons(20966);
	 udp_server_addr.sin_family = AF_INET;
	 udp_server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	 
	/* Binding the socket to IP address and port number*/   

	 bval=bind(udpsockfd, (struct sockaddr *) &udp_server_addr, sizeof(udp_server_addr));
	 if(bval<0)// Handling error in bind function
	 {
	  perror("Bind error");
	 }

	 client_length= sizeof(udp_server_addr); 

	/* Getting the port number and IP address for display below*/ 

	 getsockname(udpsockfd,(struct sockaddr *)&udp_server_addr,&client_length);
	 inet_ntop(udp_server_addr.sin_family,&udp_server_addr.sin_addr,addr,sizeof(addr)); 	
	 
	 int port20;
	 port20=ntohs(udp_server_addr.sin_port);	 

	 cout<<"Phase 2: Store_4 has UDP port "<<port20<<" and IP address "<<addr<<endl;

	/* Sending data to Warehouse*/

	 sval = sendto(udpsockfd, (void *)&sendWare, sizeof(sendWare), 0, (const struct sockaddr *) & udp_client_addr, sizeof(udp_client_addr));
	 if(sval<0)// Handling error condition in sendto
	 { 
	   perror("Error in sending data");
	 }

	 cout<<"Phase 2: The updated truck-vector <"<<sendWare.no_of_cameras<<","<<sendWare.no_of_laptops<<","<<sendWare.no_of_printers<<"> has been sent to central warehouse\nPhase 2: Store_4 updated outlet-vector is <"<<item_aggregate[0]<<","<<item_aggregate[1]<<","<<item_aggregate[2]<<">"<<endl;

	 cout<<"End of Phase 2 for store_4"<<endl;	

	  /* Close the UDP socket created*/
	 cval=close(udpsockfd);
     if(cval<0)//Handling close Error
	 {
		perror("Close socket error");
	 }
	return 0;

}


