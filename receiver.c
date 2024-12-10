#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<arpa/inet.h>
#include <fcntl.h>

#define BUFFER_SIZE 2048
#define SLEEP_TIMER 10

void print(FILE *receiver_file, char *line){			//Prints the line to both receiver_file and stdout
	fprintf(receiver_file, "%s\n", line);
	printf("%s\n", line);
}

float randnum(){
	return (float)rand()/(float)(RAND_MAX);				//Random floating point number in range [0,1]
}

int main(int argc, char *argv[]){
	
	srand((unsigned int)time(NULL));					//Initializes random number generator
	
	if(argc != 4){																												//Terminate the program if no. of command line arguments aren't 4
		printf("Command does not match predefined format:\n./receiver <ReceiverPort> <SenderPort> <PacketDropProbability>\n");
		exit(1);
	}
	
	int receiver_port = atoi(argv[1]);					//atoi() converts ASCII string to integer
	int sender_port = atoi(argv[2]);					//port numbers from 49152 to 65536 can be used as they are dynamic or private
	float packet_drop_probability = atof(argv[3]);		//atof() converts ASCII string to float

	FILE *receiver_file = fopen("receiver.txt", "w");	//Opening the receiver text file (creates if doesn't exist)
	char buffer[BUFFER_SIZE];							//Creating a buffer of size BUFFER_SIZE bytes

	struct hostent *hp;
	hp = gethostbyname("localhost");					//For localhost ip should be 127.0.0.1 as per the output of ```ifconfig lo0```
	
	//Below lines were for testing whether loopback interface is obtained or not. Uncomment them if required. Their output won't be written to the text file.
	/*
	char ip[16];													//Checking the ip of localhost. Size of array is 16 because there will be 3 dots at the most 12 digits in ip address.
	struct in_addr *addr;											//ref: https://linuxhint.com/gethostbyname-function-c/
	addr = (struct in_addr *) hp->h_addr;
	strcpy(ip, inet_ntoa(*addr));
	printf("%s has been resolved to %s\n", "localhost", ip);		//Should print: localhost has been resolved to 127.0.0.1
	*/

	struct sockaddr_in sender_addr, receiver_addr;
	socklen_t sender_addr_size = sizeof(sender_addr);

	bzero((char *)&receiver_addr, sizeof(receiver_addr));				//Erases the data in the n=sizeof(receiver_addr) bytes of the memory starting at the location pointed to by &receiver_addr by writing '\0' to that area
	receiver_addr.sin_family = AF_INET;									//AF_INET means Address Family Internet Networking
	bcopy(hp->h_addr, (char *)&receiver_addr.sin_addr, hp->h_length);	//Copies n=(hp->h_length) bytes from location pointed to by src (1st argument) to location pointed to by dest (2nd argument)
	receiver_addr.sin_port = htons(receiver_port);						//Converts the integer argument from host byte order to network byte order. Network byte order is always big-endian whereas host byte order may be little-endian or big-endian based on the machine.
	//***********************htonl was not working here for some reason

	//--------------------------------------------------------------start (Feels redundant)
	bzero((char *)&sender_addr, sizeof(sender_addr));				//Erases the data in the n=sizeof(sender_addr) bytes of the memory starting at the location pointed to by &sender_addr by writing '\0' to that area
	sender_addr.sin_family = AF_INET;								//AF_INET means Address Family Internet
	bcopy(hp->h_addr, (char *)&sender_addr.sin_addr, hp->h_length);	//Copies n=(hp->h_length) bytes from location pointed to by src (1st argument) to location pointed to by dest (2nd argument)
	sender_addr.sin_port = htons(sender_port);						//Converts the integer argument from host byte order to network byte order. Network byte order is always big-endian whereas host byte order may be little-endian or big-endian based on the machine.
	//--------------------------------------------------------------end

	int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0){
		perror("Socket error");
		exit(1);
	}
	fcntl(sockfd, F_SETFL, O_NONBLOCK);										//To stop recvfrom() from blocking when packet isn't received

	int bindval = bind(sockfd, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr));		//Binds the socket using receiver port
	if(bindval < 0){
		close(sockfd);
		perror("Bind error");
		exit(1);
	}
	
	time_t t1 = time(NULL), t2 = time(NULL);						//Time in seconds from 1 Januray 1970
	int sequence_no = 1, num_packets_not_received = 0, recvval;

	while(t2-t1 < SLEEP_TIMER){
		bzero(buffer, BUFFER_SIZE);
		recvval = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&sender_addr, &sender_addr_size);					//Flags set to 0
		if(recvval != -1){
			num_packets_not_received = 0;
			t1 = t2;
			float random_number = randnum();						//Random float number in [0,1]
			if(random_number < packet_drop_probability){			//To drop packets
				print(receiver_file, "Packet dropped");
				//printf("%.2f\n", random_number);
			}
			else{
				int sequence_no_received = atoi(buffer+7);			//"Packet:" contains 7 characters
				if(sequence_no_received != sequence_no){
					sequence_no--;
				}
				//print(receiver_file, "Data received");
				sequence_no++;
				bzero(buffer, BUFFER_SIZE);
				strcpy(buffer, "Acknowledgment:");
				//itoa(sequence_no, buffer+15, 10);					//Integer sequence_no converted to ASCII string and put in buffer after "Acknowledgment:" but this isn't a standard function hence using sprintf
				sprintf(buffer+15, "%d", sequence_no);				//Integer sequence_no converted to ASCII string and put in buffer after "Acknowledgment:" as it contains 15 characters
				sendto(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&sender_addr, sizeof(sender_addr));					//Flags set to 0
				print(receiver_file, buffer);
			}
		}
		else{														//To end the program when packets don't arrive continuously for SLEEP_TIMER amount of time (in seconds)
			num_packets_not_received++;
			if(num_packets_not_received == 1){
				t1 = time(NULL);
			}
			else if(num_packets_not_received > 1){
				t2 = time(NULL);
			}
		}
	}	
	
	close(sockfd);											//Closing the socket
	fclose(receiver_file);									//Closing the receiver text file
	return 0;
}

//References:
//https://book.systemsapproach.org/foundation/software.html#application-programming-interface-sockets
//http://www2.cs.uh.edu/~gnawali/courses/cosc4377-s12/readings/beejs.pdf
//https://www.geeksforgeeks.org/time-h-header-file-in-c-with-examples/
//https://www.cs.cmu.edu/~srini/15-441/F01.full/www/assignments/P2/htmlsim_split/node12.html
//https://www.ibm.com/docs/en/zvse/6.2?topic=SSB27H_6.2.0/fa2ti_call_recvfrom.html
//https://stackoverflow.com/questions/13408990/how-to-generate-random-float-number-in-c