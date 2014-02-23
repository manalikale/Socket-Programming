Socket-Programming
==================

This assignment involves the simulation of a logistics system primarily consisting of a central warehouse and four outlet stores. There are two main phases of the project namely inventory reporting and stock replenishment.

	Phase 1:
	In this phase, every store opens its corresponding store file, reads it and stores data into the Devices structure. Simulteneously, the central warehouse will creates a TCP socket,connect and listen for incoming inventory from each store.
	Every store reports its device inventory read from the file to the central warehouse on a static TCP port.	 
	
	Phase 2:	 
	In this phase, the central warehouse sends the truck-vector over a dynamic UDP connection to Store 1. Store 1 updates both the truck vector and and outlet vector and sends these vectors to Store 2. Store2 sends messages to Store3, Store3 sends messages to Store4 and Store4 forwards the vectors to Store1 again. This loops for two times and then Store4 returns truck vector to the central warehouse. Positive values in the truck-vector at the end of the loop running twice indicates that the store requirement has been satisfied.	 
	 	
Code Files:
    
	device.h :
	It contains the Devices structure that is required by four store codes and warehouse code for storing the data.
	
	store1.cc :
	It contains the code for functioning of Store1 which includes creation of TCP socket, reads Devices information from Store-1.txt file, sending inventory information to Store 2 and sending truck vector and outlet vector received from warehouse over UDP conection.
	
	store2.cc :
	It contains the code for functioning of Store2 which includes creation of TCP sockets, reads Devices information from Store-2.txt file, sending inventory information to Store 3 and sending truck vector and outlet vector received from Store 1 over UDP conection.
	
	store3.cc :
	It contains the code for functioning of Store3 which includes creation of TCP sockets, reads Devices information from Store-3.txt file, sending inventory to Store 4 and sending truck vector and outlet vector received from Store 2 over UDP conection.
	
	store4.cc :
	It contains the code for functioning of Store4 which includes creation of TCP sockets, reads Devices information from Store-4.txt file, sending inventory to Store 5 and sending truck vector and outlet vector received from Store 3 over UDP conection.
	
	warehouse.cc - It receives Devices information from all the Stores mentioned above,sends the truck vector to Store 1 and receives it after 2 rounds after traversing Store 1 to Store 4. 		

Steps for running the program:
	1. Open a putty session. 
	2. Go to the directory where source code files are saved.
	3. Run makefile to compile source code and create executables.
	   run makefile as: gmake all
	3. Then open different simulteneous putty connections and run codes as below in the order-
		./warehouse.o
		./store1.o
		./store2.o
		./store3.o
		./store4.o
		
	4.Terminate the warehouse process using Ctrl+C when it shows "End of Phase 2 for Warehouse" on the window where you run "./warehouse.o".
	
The format of the messages exchanged are as per the specification mentioned in the EE450_Socket_Programming_Fall12.pdf file. 

The project works fine once the connection with the warehouse is establihed. The store codes should then be executed in the following sequence: store1, store2, store3, store4 for complete execution of both Phase1 and Phase2. However, if Ctrl+C is pressed during execution of any store code, the next store will give a pipe fail error since fork() has been used in the warehouse code for creating children processes(stores) and pipe is used for inter-process communication. 

h.  Reused Code:  The following functions have been used from Beej's manual:
	getsockname(),ntohs(),listen(),recvfrom(),receive(),send(),sendto(),bind(),connect(),accept(),inet_ntop(),close().
	
	Code for reading a file is reused from www.stackoverflow.com and code for fork() in warehouse.cc is reused from Prof. Bill Cheng's slides(Lec 7:slide5)
