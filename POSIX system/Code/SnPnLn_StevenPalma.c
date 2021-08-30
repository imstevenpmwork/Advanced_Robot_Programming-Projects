// 2019 ARP Assignment V2.0
// Steven Palma Morera - S4882385
// 2019 - 02 - 24

// This script contains the development
// of the Sn, Pn and Ln process

// Includes
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<sys/wait.h>
#include <math.h>
#include <arpa/inet.h>

// Port number for the socket
//#define PORT 8086

#define _GNU_SOURCE

// Global variable for Start and Stop signal
int start_stop=1;

// Message to send to Gn+1
struct token_with_time_Pn{

	double token1;
	double time1;
	struct timeval arrival_time;
};

// Signal handler for SIGUSR1 (Start/Stop signal)
void sig_handler_sigusr1(int signo) {

	printf("(S): I received SIGUSR1\n");
	fflush(stdout);

	// Toogle the value of the variable whenever a signal arrives
	if (start_stop==0){
		start_stop=1;
	}
	else {
		start_stop=0;
	}

	// Write value in a nammed pipe communicating with Pn
	// Notifies that a Start/Stop signal has arrived
	int e;
	int fdaux;
	//fdaux = open ( "/tmp/mypipe", O_WRONLY);
	fdaux = open ( "mypipe", O_WRONLY);
	if (fdaux<0){
		perror("open");
	}
	e=write (fdaux, &start_stop, sizeof(start_stop));
	if (e<0){
		perror("write");
	}
	close(fdaux);
	if (e<0){
		perror("close");
	}
}

// Signal handler for SIGUSR2 (Dump log signal)
void sig_handler_sigusr2(int signo) {

	printf("(S): I received SIGUSR2\n");
	fflush(stdout);

	// Write in a nammed pipe communicating with Pn
	// Notifies that a Log signal has arrived
	int log=2;
	int fdaux2;
	int e;
	//fdaux2= open("/tmp/mypipe", O_WRONLY);
	fdaux2= open("mypipe", O_WRONLY);
	if (fdaux2<0){
		perror("open");
	}
	e=write (fdaux2, &log, sizeof(log));
	if (e<0){
		perror("write");
	}
	e=close(fdaux2);
	if (e<0){
		perror("close");
	}


	// File pointer to read the .log file and
	// print its content
	FILE *f;
	char c;

	f = fopen("Log_StevenPalma.log", "r");
	if (f==NULL){
		perror("fopen");
	}
	c= fgetc(f);
	printf("------ LOG FILE -----\n");
	fflush(stdout);
	while (c != EOF){
		printf("%c",c);
		fflush(stdout);
		c=fgetc(f);
	}
	fclose(f);
	printf("-------- END -------\n");
	fflush(stdout);
}

// Sn function > Declare signals
int S_task (int child_PID_P){

	// Create the signal handlers and perror
	if (signal(SIGUSR1, sig_handler_sigusr1) == SIG_ERR){
		perror("signal");
		return -1;
	}

	if (signal(SIGUSR2, sig_handler_sigusr2) == SIG_ERR){
		perror("signal");
		return -1;
	}

	return 1;
}

// Ln function > Logs data
// Takes as input the file descriptors of an unnammed pipe
int L_task(int fildes0, int fildes1){

	// Print Ln PID
	printf("(L): my process id= %d\n", (int) getpid());
	fflush(stdout);

	// Variable for reading msg from unnammed pipe
	char messagechar [100];
	int e;

	// File pointer to open .log file and write on it
	FILE *f;

	// Variables for a select()
	fd_set rfds;
	struct timeval tv;
	int retval;
	FD_ZERO(&rfds);

	// Ln is constantly waiting for a new message from the unammed pipe
	while (1){

		// Put the reading end of the unnammed pipe in the reading set
		FD_SET(fildes0, &rfds);

		// Declare the wait/timeout time of the select
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		// Using a select for performance optimization
		retval = select(20, &rfds, NULL, NULL, &tv);
		if (retval<0){
			perror("select");
			return -1;
		}

		// If there is data in the pipe
		if (retval>0){

			// Close the writing end of the unnammed pipe and read its content
			// e=close(fildes1);
			// if (e<0){
			// 	perror("close");
			// 	return -1;
			// }

			// Read the unnammed pipe
			e=read(fildes0,messagechar,100);
			if (e<0){
				perror("read");
				return -1;
			}

			printf("(L): I logged the message\n");
			fflush(stdout);

			e=gettimeofday(&tv,NULL);
			if (e<0){
				perror("gettimeofday");
				return -1;
			}

			// Open the log file and print the message in it
			f = fopen("Log_StevenPalma.log", "a+");
			if (f==NULL){
				perror("fopen");
				return -1;
			}
			fprintf(f, "Time= %ld.%06ld\n",tv.tv_sec,tv.tv_usec);
			fprintf(f, "%s\n" ,messagechar);
			fclose(f);
		}
	}

	// Never arrives here
	return 1;
}

// Pn function > Reads and writes pipes, receives, computes and sends tokens
// Takes as input the file descriptors of an unnammed pipe
int P_task(int child_PID_L, int fildes0, int fildes1){

	// File descriptors for nammed pipes
	int fd1;
	int fd2;

	// Variables for error handling and logic
	int e;
	int onoff=1;

	// Creating nammed pipes
	//char *myfifo= "/tmp/mypipe";
	//char *myfifo2= "/tmp/mypipe2";

	char *myfifo= "mypipe";
	char *myfifo2= "mypipe2";

	e=mkfifo( myfifo , S_IRUSR | S_IWUSR);
	// if (e<0){
	// 	perror("mkfifo");
	// 	return -1;
	// }
	e=mkfifo( myfifo2 , S_IRUSR | S_IWUSR);
	// if (e<0){
	// 	perror("mkfifo");
	// 	return -1;
	// }

	// Variables for messages of the nammed pipes
	int messageint=0;
	char messagechar2 [100];

	// Variables for the socket connection
	int sock = 0;
	struct sockaddr_in serv_addr;

	// Variables for a select()
	fd_set rfds;
	struct timeval tv;
	int retval;
	FD_ZERO(&rfds);

	// Read values from config file
	// Variables for reading
	FILE * fp;
  char * line = NULL;
  size_t len = 0;

	// Variables read
	char *ipuser= NULL;
	int port2 = 0;
	float refv = 0;
	unsigned int usecs=0;

	// Open config file
	// Directory must be changed!
	fp = fopen("Config_StevenPalma.config", "r");
	if (fp==NULL){
		 perror("open");
		 return -1;
	 }

	 // Read first line (port2)
	 if ((e=getline(&line,&len,fp))!=-1){
			 port2=atoi(line);
	 }

	 if ((e=getline(&line,&len,fp))!=-1){
		 // Read second line (refv)
			 refv=atof(line);
	 }

	 // Read third line (ipuser ADDRESS)
	 if ((e=getline(&line,&len,fp))!=-1){
			 ipuser=line;
	 }

	 // Read fourth line (delay)
	 if ((e=getline(&line,&len,fp))!=-1){
			 usecs=atoi(line);
	 }
	 else{
		 perror("readconfig");
	 }

	 // Closes config file

	 fclose(fp);

	// P is constantly waiting for new messages from the nammed pipes
	while (1){

		// Open nammed pipes in non blocking mode
		//fd1 = open ( "/tmp/mypipe", O_RDONLY | O_NONBLOCK);
		fd1 = open ( "mypipe", O_RDONLY | O_NONBLOCK);
		if (fd1<0){
			perror("open");
			return -1;
		}
		//fd2 = open ( "/tmp/mypipe2", O_RDONLY | O_NONBLOCK);
		fd2 = open ( "mypipe2", O_RDONLY | O_NONBLOCK);
		if (fd2<0){
			perror("open");
			return -1;
		}

		// Add the file descriptors of the nammed pipes to the reading set of the select
		FD_SET(fd1, &rfds);
		FD_SET(fd2, &rfds);

		// Declare the wait/timeout time of the select
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		// Select between the two nammed pipes
		retval = select(20, &rfds, NULL, NULL, &tv);

		if (retval == -1){
			perror("select");
			e=close(fd1);
			if (e<0){
				perror("close");
				return -1;
			}
			e= close(fd2);
			if (e<0){
				perror("close");
				return -1;
			}
			return -1;
		}

		// If there isnt any data in either pipes
		else if (retval==0){

			printf("(P): No data available.\n");
			fflush(stdout);

			e=close(fd1);
			if (e<0){
				perror("close");
				return -1;
			}
			e=close(fd2);
			if (e<0){
				perror("close");
				return -1;
			}
		}

		// If there is new data available in either of the pipes
		else {

			// If the data comes from the nammed pipe "mypipe"
			// "mypipe" has priority over "mypipe2"
			if (FD_ISSET(fd1, &rfds)){

				// Read the data
				e=read (fd1, &messageint, sizeof(messageint));
				if (e<0){
					perror("read");
					return -1;
				}

				// Close both nammed pipes
				e=close(fd1);
				if (e<0){
					perror("close");
					return -1;
				}
				e=close(fd2);
				if (e<0){
					perror("close");
					return -1;
				}

				printf("(P): I received> %d\n",messageint);
				fflush(stdout);

				// If the data= 1, start the receiving, computing and sending of the token
				if (messageint==1){

					printf("(P): Start\n");
					fflush(stdout);
					onoff=messageint;

					// Event to be written in the log
					char * phrase = "From S> SIGUSR1 on\n";

					// Close the reading end of the unnammed pipe
					// e=close(fildes0);
					// if (e<0){
					// 	perror("close");
					// 	return -1;
					// }

					// Write event in the unnammed pipe
					e=write(fildes1,phrase,strlen(phrase)+1);
					if (e<0){
						perror("write");
						return -1;
					}
				}

				// If the data= 2, write in the log that a log display has been solicited
				else if (messageint==2){

					printf("(P): Log\n");
					fflush(stdout);

					// Event to be written in the log
					char * phrase = "From S> SIGUSR2 log\n";

					// Close the reading end of the unnammed pipe
					// e=close(fildes0);
					// if (e<0){
					// 	perror("close");
					// 	return -1;
					// }

					// Write event in the unnammed pipe
					e=write(fildes1,phrase,strlen(phrase)+1);
					if (e<0){
						perror("write");
						return -1;
					}

				}

				// If the data= 0, stop the receiving, computing and sending of the token
				else{

					printf("(P): Stop\n");
					fflush(stdout);

					// Event to be written in the log
					char * phrase = "From S> SIGUSR1 off\n";
					onoff=messageint;

					// Close the reading end of the unnammed pipe
					// e=close(fildes0);
					// if (e<0){
					// 	perror("close");
					// 	return -1;
					// }

					// Write event in the unnammed pipe
					e=write(fildes1,phrase,strlen(phrase)+1);
					if (e<0){
						perror("write");
						return -1;
					}
				}
			}

			// If the data comes from the nammed pipe "mypipe2"
			else if (FD_ISSET(fd2, &rfds)){

				// If receiving, computing and sending on
				if (onoff==1){

					// Create a variable type token_with_time_Pn to manipulate tokens
					struct token_with_time_Pn Pn_token;

					// Read from nammed pipe mypipe2
					e=read(fd2, messagechar2, 100);
					if (e<0){
						perror("read");
						return -1;
					}

					// Close both nammed pipes
					e=close (fd2);
					if (e<0){
						perror("close");
						return -1;
					}
					e=close(fd1);
					if (e<0){
						perror("close");
						return -1;
					}

					// The data read from mypipe2 is a char with a form
					// float token
					// float seconds since EPOCH

					// Splitting the char in order to save both numbers in different variables
  				char *p;
  				p = strtok (messagechar2,"\n");

					// Saving the first number from the data message (token) into the token1 attribute of the struct
					Pn_token.token1=atof(p);
  				while (p!= NULL){

						// Saving the second number from the data message (token) into the token1 attribute of the struct
						Pn_token.time1=atof(p);
    				p = strtok (NULL, "\n");
  				}

					printf("(P): I received> Token= %f Timestamp= %f\n",Pn_token.token1,Pn_token.time1);
					fflush(stdout);

					char phrase[100];

					// Event to be written in the log file
					sprintf(phrase,"From G> Received> Token= %f Timestamp= %f\n",Pn_token.token1,Pn_token.time1);

					// Close the reading end of the unammed pipe
					// e=close(fildes0);
					// if (e<0){
					// 	perror("close");
					// 	return -1;
					// }

					// Write in the unanammed pipe
					e=write(fildes1,&phrase,strlen(phrase)+1);
					if (e<0){
						perror("write");
						return -1;
					}

					// Once that Pn has received a new token, it must compute the new one and send it through the socket


					// Creating socket file descriptor
			 	 	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
						perror("socket");
						return -1;
					}

			 	 	// Set address of the socket
			 	 	serv_addr.sin_family = AF_INET;
					serv_addr.sin_addr.s_addr = INADDR_ANY;
			 	 	serv_addr.sin_port = htons(port2);

			 	 	// Convert IPv4 and IPv6 addresses from text to binary form
			 	 	// This can be configured for another ip cpu
			 	 	// e=inet_pton(AF_INET, ipuser, &serv_addr.sin_addr);
			 	 	e=inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
			 	 	if(e<=0){
						perror("inet_pton");
						return -1;
					}

				 	// Waits delay
				 	printf("(P): Waiting the delay\n");
				 	fflush(stdout);
				 	usleep(usecs);

				 	// Waits the connection of the other socket
				 	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
						perror("connect");
						return -1;
					}

					// Once they are connected
					// Take current time
					e=gettimeofday( &Pn_token.arrival_time,NULL);
					if (e<0){
						perror("gettimeofday");
						return -1;
					}

					// Computes new token
					// new token = received token + DT x (1. - (received token)^2/2) x 2 pi x refv
					//Pn_token.token1=Pn_token.token1*10000000 + (Pn_token.arrival_time.tv_sec*1000000+Pn_token.arrival_time.tv_usec-Pn_token.time1*1000000)*(1-((Pn_token.token1*Pn_token.token1*1000000000000)/2))*2*M_PI*refv;
					// Since the provided formula doesn't seem to work, a new formula is used
					// New_token= Past_token + DT x refv
					// Which represents the total distance traveled of a body moving at refv speed for DT microseconds
					Pn_token.token1= ((Pn_token.token1 + (Pn_token.arrival_time.tv_sec*1000000+Pn_token.arrival_time.tv_usec-Pn_token.time1*1000000)*refv));

					// Save new token and timestamp in the phrase variable with the same format as received
					sprintf(phrase,"%f\n%ld.%06ld\n",Pn_token.token1,Pn_token.arrival_time.tv_sec,Pn_token.arrival_time.tv_usec);

					// Send token and timestamp through the socket
					e=send(sock,&phrase,strlen(phrase),0);
					if (e<0){
						perror("send");
						return -1;
					}

					// Closes the socket
					e=close(sock);
					if(e<0){
						perror("close");
					}

					printf("(P): I sent> Token= %f Timestamp= %ld.%06ld\n",Pn_token.token1,Pn_token.arrival_time.tv_sec,Pn_token.arrival_time.tv_usec);
					fflush(stdout);

					// Event to be written in the log file
					sprintf(phrase,"From P> Sent-> Token= %f Timestamp= %ld.%06ld\n",Pn_token.token1,Pn_token.arrival_time.tv_sec,Pn_token.arrival_time.tv_usec);

					// Close the reading end of the unammed pipe
					// e=close(fildes0);
					// if (e<0){
					// 	perror("close");
					// 	return -1;
					// }

					// Write in the unammed pipe
					e=write(fildes1,&phrase,strlen(phrase)+1);
					if (e<0){
						perror("write");
						return -1;
					}
				}

				// If receiving, computing and sending off
				else{
					printf("(P): Process is locked.\n");
					fflush(stdout);

					// Close both nammed pipes
					e=close (fd2);
					if (e<0){
						perror("close");
						return -1;
					}
					e=close(fd1);
					if (e<0){
						perror("close");
						return -1;
					}
				}
			}

			// If the data comes neither from mypipe1 nor mypipe2
			// Something strange happened
			else{

				printf("An unknown error ocurred, please try again");
				fflush(stdout);
				e=close(fd2);
				if (e<0){
					perror("close");
					return -1;
				}
				e=close(fd1);
				if (e<0){
					perror("close");
					return -1;
				}
			}

		}

	}

	return 1;
}

// Main of the program
int main (int argc, char const *argv[]) {


	int retval;

	// PID for P
	pid_t child_PID_P;
	pid_t child_PID_G;

	printf("(S): my process id= %d (parent PID= %d)\n", (int) getpid(), (int) getppid());
	fflush(stdout);

	// Fork P
	child_PID_P = fork();
	if (child_PID_P <0){
		perror("fork");
  	return -1;
	}

	// S process executes S function
	else if (child_PID_P > 0){

		// Fork G
		child_PID_G=fork();
		if (child_PID_P <0){
			perror("fork");
	  	return -1;
		}

		// S process executes S function
		else if (child_PID_G>0){

			retval=S_task(child_PID_P);
		}

		// G executes
		else{

			printf("(G): my process id= %d\n", (int) getpid());
			fflush(stdout);

			// Using execvp system call
			char *arg_G[3];
			arg_G[0] = (char *)"./Gn";
      arg_G[1] = NULL;
			arg_G[2] = NULL;

			retval=execvp(arg_G[0],arg_G);
			if (retval<0){
				perror("execvp");
				return -1;
			}
		}
	}

	// P
	else {

		printf("(P): my process id= %d\n", (int) getpid());
		fflush(stdout);

		// PID for L
		pid_t child_PID_L;

		// File descriptors for the unammed pipe
		int fildes[2];

		// Create unammed pipe
		if (pipe(fildes) != 0){
	  		perror("Pipe");
	  		return -1;
	 	}

		// Fork L
		child_PID_L = fork();
		if (child_PID_L <0){
			perror("Fork");
  		return -1;
		}

		// P executes P function
		else if (child_PID_L > 0){

			// Pass as arguments the file descriptors of the unnammed pipe
			retval=P_task(child_PID_L, fildes[0],fildes[1]);
			return retval;

		}

		// L executes L funtion
		else {

			// Pass as arguments the file descriptors of the unnammed pipe
			retval=L_task(fildes[0],fildes[1]);
			return retval;
		}
	}

	// Only Sn process arrives here, since Pn and Ln are in an while(1) loop
	// Sn stays in standby waiting for Pn to end or for a signal to arrive
	// This should be done for performance optimization
	//wait(NULL);
	int status=1;
	waitpid(child_PID_P,&status,0);

	return 1;
}
