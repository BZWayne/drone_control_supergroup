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
#include <time.h> 

double x, x_hat, y, y_hat, fuel = 100;
int maxStep = 5, permissionFlag = 1;

void error (char *msg)
{
    perror(msg);
    exit(0);
}

// function for chosing random direction for drone
const char * direction()
{
    const char *string_table[] = { // array of pointers to constant strings
        "north",
        "east",
        "south",
        "west",
        "north-east",
        "north-west",
        "south-east",
        "south-west",
    };
    int table_size = 8; 
    const char *rand_string = string_table[rand() % table_size];
    return rand_string;
}

int main (int argc, char *argv[]) 
{
    char sendBuffer[1000];
    char receiveBuffer[1000];
    char hostname[1024];

    // drone's initial state
    x = atoi(argv[1]);
    y = atoi(argv[2]);

    // performing socket communication with the server
    int sockfd, n, response;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    gethostname(hostname, 1024);
    server = gethostbyname(hostname);
    if (server == NULL) 
    {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(7777);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    srand(time(NULL)); // randomize the start value

    while(fuel > 0) 
    {
        permissionFlag = 1;
        while((permissionFlag == 1) && (fuel > 0))
        {
            const char *dir = direction(); // change direction
            printf("\nchanging direction to %s...\n",dir);
            fflush(stdout);

            // loop for avoid vibrating
            for (int j = 0; j < maxStep; j++)
            {
                sleep(1);
                // detecting drone's next state
                x_hat = x;
                y_hat = y;
                if(dir == "north") y_hat--; 
                else if(dir == "east") x_hat++; 
                else if(dir == "south") y_hat++; 
                else if(dir == "west") x_hat--; 
                else if(dir == "north-east") {y_hat--; x_hat++;}
                else if(dir == "north-west") {y_hat--; x_hat--;}
                else if(dir == "south-east") {y_hat++; x_hat++;}
                else if(dir == "south-west") {y_hat++; x_hat--;}

                // compusing the message 
                bzero(sendBuffer,1000);
                sprintf(sendBuffer,"%c,%lf,%lf,%lf,", 'e', x_hat, y_hat, fuel); 

                // sending drone's next state to get permission             
                n = write(sockfd,sendBuffer,strlen(sendBuffer)); 
		sleep(1);  
                fsync(sockfd);             
                if (n < 0)                  
                    error("ERROR writing to socket");
                printf("requesting for next state: %f , %f, \n", x_hat, y_hat);
                fflush(stdout); 

                // getting response from server             
                n = read(sockfd, receiveBuffer, sizeof(receiveBuffer));                  
                sscanf(receiveBuffer,"%d,",&response);             
                if (response == 1)
                { 
                    printf("request accepted...\n");
                    fflush(stdout);
                    // moving the drone
                    x = x_hat;
                    y = y_hat;

                    // consuming fuel
                    fuel-=0.1;
                    printf("remaining fuel: %f\n\n", fuel);
                    fflush(stdout);
                }             
                else if (response == 0)  
                {
                    printf("request rejected...\n\n");
                    fflush(stdout);
                    permissionFlag = 0; 
                }    
                else
                {
                    perror("invalid response!");
                    exit(EXIT_FAILURE);
                }    
            }
        }
    }
    printf("fuel is over, landing...\n");
    return 0; 
} 
