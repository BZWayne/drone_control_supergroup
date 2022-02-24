// necessary libs for project
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <ctype.h>
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <netdb.h> 

// functions
void error_msg(const char *msg);

// max values for (x,y)
float x_max = 80.0;
float y_max = 40.0;

// server parameters
char server_address[] = "127.0.0.1";

// denoting the error function
void error_msg(const char *msg) {
    perror(msg);
}

// coorfinate function for any edges
float coord(float coord, float max_coord, float step) {
    float new_coord = coord - step;

    // if the new coord < 0
    if (new_coord <= 0) {
        new_coord = 0;
    }

    // if the new coord > 0
    else if (new_coord >= max_coord) {
        new_coord = max_coord;
    }
    return new_coord;
}

// main 
int main(int argc, char *argv[]) { 

    //initialize socket  
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    // setting buffer 
    char buffer[256];
    float curr_x;
    float curr_y;

    // setting port number
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // Issue during opening socket
 	if (sockfd < 0) {
 		error_msg("Error, Openning Socket is failed");
    }

    // setting the connection 
	bzero((char *) &serv_addr, sizeof(serv_addr));
 	serv_addr.sin_family = AF_INET;

    // to convert a text host address to a numeric address
    if (inet_pton(AF_INET, server_address, &serv_addr.sin_addr.s_addr) <= 0){
        return -1;
    }

    // port 777
 	serv_addr.sin_port = htons(7777); 

    // error during connection with server
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error_msg("Connection using Socket failed\n");
    }
    else {
        printf("Connected \n");
    }

    // defining [group],[x],[y],[fuel_left], --- group A->Z
    // initial parameters for group, x, ,y, fuel (full)
    char group = 'A';
    float x;
    float y;
    float step = 1.0;
    float fuel_full = 100.0;
    float usage = 2.0;
    float fuel_new;
    int server_msg;
    int steps = 1.0;

    //reinitializing (x, y) coords
    curr_x = 45;
    curr_y = 20;
    x = curr_x;
    y = curr_y;
	int num;

    float x_step, y_step;
    float fuel = fuel_full;
    
    while (1) {
        
        bzero(buffer, 256);

        // random movement
	    int num = rand()%((3+1)-1) + 1;
        int num_dir = rand()%((8+1)-1) + 1;
         
        //check there is a fuel 
        if (fuel > 5.0) {
            x_step = coord(x, x_max, step);
            y_step = coord(y, y_max, step);

            /// all conditions for drone movement
            // if drone touches edges 80 and 40
            if (x_step >= x_max){
                x_step = x_step - num;
            } 
            if (y_step >= y_max){
                y_step = y_step - num;
            }

            // if drone is not close to land
            if (x_step >= 10){
                x_step = x_step - num_dir;
            } 
            if (y_step >= 10){
                y_step = y_step - num_dir;
            }
            
            // if drone is too close to land
            if (x_step < 10){
                x_step = x_step + num;
            } 

            if (y_step < 10){
                y_step = y_step + num;
            }

            // extracting fuel
            fuel_new = fuel - usage;
        }
    
    // land the drone by decreasing values 
	else if (fuel > 0) {
		fuel_new = fuel - 1;
		printf("Fuel is finishing\n");
		printf("Drone is landing\n");
		x_step = x - 2;
		y_step = x - 2;
	}

    // drone landed
	else {	
		x_step = 0;
		y_step = 0;
		printf("Fuel finished\n");
	}

        //wrtiting the values to buffer
        sprintf(buffer, "%c, %lf, %lf, %lf", group, x_step, y_step, fuel_new);
        printf("Client -> Server: %s\n", buffer);

        // printf("I am here");
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0){
            error_msg("Error reading from socket\n");}
        // printf("Now, I am here");
        
        // reading the values from buffer
        bzero(buffer, 256);
        n = read(sockfd, buffer, 255);
        if (n < 0){
            error_msg("Error reading from socket\n");
        }
        // printf("%s\n", buffer);
        // printf("Look, I am here");
        
        // recognising the message from server
        // if 1, the message accepted , drone can move
        // if 0, the message declined, drone cannot move
        server_msg = atoi(buffer);
        printf("Server -> Client: %d\n", server_msg);
        
        if (server_msg == 1) {
            printf("Movement Accepted\n");
            fuel = fuel_new;
            x = x_step;
            y = y_step;
            steps++;
        }

        else if (server_msg == 0)  {
            printf("Movement Rejected\n");
        }

        // sleep time
        usleep(2000000);
    }
	close(sockfd);
    return 0;
}
