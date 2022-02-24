#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h> 

// Defining the port number
#define PORTNO 7777

// Some global variables of double and int type
double x, y;
double x_incmt, y_incmt;
double fuel = 100;

//pointer to log file
FILE *logfile;

//This function checks if something failed, exits the program and prints an error in the logfile
int check(int retval)
{
    if (retval == -1)
    {
        fprintf(logfile, "\nClient - ERROR (" __FILE__ ":%d) -- %s\n", __LINE__, strerror(errno));
        fflush(logfile);
        fclose(logfile);
        printf("\tAn error has been reported on log file.\n");
        fflush(stdout);
        exit(-1);
    }
    return retval;
}

// Choosing movement of the drone along different dimentions. It return the random string which has been choosen by the function randomly
const char * movement()
{
    const char *string_table[] = {
        "UP",
        "RIGHT"
        "DOWN",
        "LEFT",
        "UP-RIGHT",
        "UP-LEFT",
        "DOWN-RIGHT",
        "DOWN-LEFT",
    };

    // Randomize the direction of the drone 
    // Randomize with the size of the table which is 4
    const char *rand_string = string_table[rand() % 4];
    return rand_string; // returning the random string
}

// The main process is starting here
int main (int argc, char *argv[]) 
{
    //open Log file
    logfile = fopen("Drone.txt", "a");
    if (logfile == NULL)
    {
        printf("An error occured while creating Drone's log file\n");
        return 0;
    }
    fprintf(logfile, "******log file created******\n");
    fflush(logfile);
    
    
    char sendBuffer[1000];
    char receiveBuffer[1000];
    char hostname[1024];

    // Getting theh initial state of the drone
    x = atoi(argv[1]);
    y = atoi(argv[2]);

    // performing socket communication with the server
    int sockfd, n, response;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    sockfd = check(socket(AF_INET, SOCK_STREAM, 0));
    gethostname(hostname, 1024);
 
    //write on log file
    fprintf(logfile, "c - socket created\n");
    fflush(logfile);

    server = gethostbyname(hostname);
    if (server == NULL)
    {
        check(-1);
    }
    //write on log file
    fprintf(logfile, "c - correct server\n");
    fflush(logfile);

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(PORTNO);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //open new connection
    if (check(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))) < 0) 
        {
            check(-1);
        }
    //write on log file
    fprintf(logfile, "c - connected to socket\n");
    fflush(logfile);

    // randomize the start value
    srand(time(NULL)); 
    //Writing in log file
    fprintf(logfile, "c - randomizing seed for random error generator\n");

    while(fuel > 0) 
    {
        const char *dir = movement(); // change direction

        // loop for avoid vibrating
        for (int i = 0; i < 5; i++)
        {
            // Operations performed in order to detect next state of the drone
            x_incmt = x;
            y_incmt = y;
            if(dir == "UP") 
            {
                y_incmt++;
            }
            if(dir == "RIGHT") 
            {
                x_incmt++;
            }
            if(dir == "DOWN") 
            {
                y_incmt--;
            }
            if(dir == "LEFT") 
            {
                x_incmt--;
            }
            if(dir == "UP-RIGHT") 
            {
                y_incmt++; 
                x_incmt++;
            }
            if(dir == "UP-LEFT") 
            {
                y_incmt++; 
                x_incmt--;
            }
            if(dir == "DOWN-RIGHT") 
            {
                y_incmt--; 
                x_incmt++;
            }
            if(dir == "DOWN-LEFT") 
            {
                y_incmt--; 
                x_incmt--;
            }

            // Composing the message 
            bzero(sendBuffer,4);
            sprintf(sendBuffer,"%c,%lf,%lf,%lf,", 'a', x_incmt, y_incmt, fuel);

            // Sending drone's next state to get permission             
            check(write(sockfd,sendBuffer,strlen(sendBuffer)));   
            fsync(sockfd);                  
            sleep(1);
            fprintf(logfile, "c - Drone's next state has been sent\n");
            fflush(logfile);
  

            // Fetting response from server             
            n = check(read(sockfd, receiveBuffer, sizeof(receiveBuffer))); 

            // Shwoing the confirmation on the screen which has got by the server
            printf("Received Confirmation %d\n",n);   
            fsync(stdout);             

            sscanf(receiveBuffer,"%d,",&response);             
            if (response == 1)                 
                continue;             
            else                 
                break;
            
            // Drone is moving
            x = x_incmt;
            y = y_incmt;

            // Consuming the fuel
            fuel--;
        }
    }
    printf("Fuel is about to end, landing...\n");
    fprintf(logfile, "c - Fuel is about to end, landing...\n");
    fflush(logfile);


    //close log file
    fclose(logfile);

    return 0; 
} 
