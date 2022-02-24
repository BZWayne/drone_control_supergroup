#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <sys/select.h>
#include <math.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define PORT 7777

//int randomizer;

void error(char *msg)
{
    perror(msg);
    exit(0);
}
/*
Function to move the drone, there are 8 directions to travel: North, East, West, South, North-West, North-East, South-West, South-East
This function takes pointers to the x and y variables and updates them corresponding to the direction chosen
*/
void move(float *x, float *y, float step, int *direction){
  /*
  The drone is capable of moving only 0.5m per step, thus to calculate the distance it can travel in a diagonal direction, we use Pythagorean theorem
  */
  float diag = roundf(step / sqrt(2) * 100) / 100;
  switch (*direction){
    case 0: // going south
          *y = *y + step;
          break;
    case 1: // going S-W
          *x = *x - diag;
          *y = *y + diag;
          break;
    case 2: // going West
          *x = *x - step;
          break;
    case 3: // goint N-W
          *x = *x - diag;
          *y = *y - diag;
          break;
    case 4: // going North
          *y = *y - step;
          break;
    case 5: // going N-E
          *x = *x + diag;
          *y = *y - diag;
          break;
    case 6: // going East
          *x = *x + step;
          break;
    case 7: // going S-E
          *x = *x + diag;
          *y = *y + diag;
          break;
    default:
          break;
  }
  // Boundaries
  if ( *y <= 0){
    *y = 0;
    *direction = 0; // If we are in the border, we go the other way
  }else if (*y >= 40){ // y = 40 is in the south
    *y = 40;
    *direction = 4; // If we are in the border, we go the other way
  }
  if (*x <= 0){ // x = 0 is on the west
    *x = 0;
    *direction = 6; // If we are in the border, we go the other way
  }else if( *x>=80 ){
    *x = 80;
    *direction = 2; // If we are in the border, we go the other way
  }

}
/*
Function that simulates the wind. The wind blows the drone and changes the position of the drone by a random number between [0;1]
*/
void wind(float *x, float*y){
  int wind = rand() % 5;
  float shift = (double)rand() / (double)RAND_MAX;
  shift = roundf(shift * 100) / 100;
  switch (wind){
    case 0: // Wind from North, blows the drone to the South
          *y = *y + shift;
          printf("---------\n");
          printf("Danger, wind from North...\n");
          printf("---------\n");
          break;
    case 1: // Wind from South, blows the drone to the North
          *y = *y - shift;
          printf("---------\n");
          printf("Danger, wind from South...\n");
          printf("---------\n");
          break;
    case 2: // Wind from East, blows the drone to the West
          *x = *x - shift;
          printf("---------\n");
          printf("Danger, wind from East...\n");
          printf("---------\n");
          break;
    case 3: // Wind from West, blows the drone to the East
          *x = *x + shift;
          printf("---------\n");
          printf("Danger, wind from West...\n");
          printf("---------\n");
          break;
    default:
          break;
  }
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int sockfd, n;
    float x, y, step, fuel, x_tmp, y_tmp, fuel_tmp;
    fuel = 100;
    char group = 'J';
    int response;
    x = 70; // Starting position of the drone in x coordinates; x belongs [0;80]
    y = 30; // Starting position of the drone in y coordinates; y belongs [0;40]
    step = 0.5; // Maximim travel distance in one time step
    /*
    Establishing socket connection
    */
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    serv_addr.sin_port = htons(PORT);

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    int n_steps = 0;
    int direction = 0;
    float p = 0;
    while (1) {
      bzero(buffer,256);
      x_tmp = x;
      y_tmp = y;
      if (fuel >= 3.0) {
        if (n_steps % 10 == 0){ // Each 10 steps we change the direction
          direction = rand() % 8; // Random integer number between 0-7
        }
        move(&x_tmp, &y_tmp, step, &direction); // Update the x and y position as we travel
        fuel_tmp = fuel - 1; // Each step takes 1% of fuel
        p = (double)rand() / (double)RAND_MAX; // Calculate the probability
        if (p < 0.3){ // With 30% probability, the wind occurs
          wind(&x_tmp, &y_tmp);
        }
      }else if (fuel > 0){
        fuel_tmp = fuel - 1;
        printf("Running out of fuel, landing ...\n");
      }else if (fuel == 0){
        printf("No fuel...\n");
      }
      // Send our intended coordinates to the master
      sprintf(buffer, "%c, %lf, %lf, %lf", group, x_tmp, y_tmp, fuel_tmp);
      printf("Sending: %s\n", buffer);
      n = write(sockfd,buffer,strlen(buffer));
      
      if (n < 0)
           error("ERROR writing to socket");
      // Read the response
      bzero(buffer,256);
      n = read(sockfd,buffer,255);
      if (n < 0)
           error("ERROR reading from socket");
      response = atoi(buffer);
      // If 1, then we can travel, and thus we update our real x,y coordinates
      if (response == 1){
        printf("Movement has been accepted\n");
        x = x_tmp;
        y = y_tmp;
        fuel = fuel_tmp;
        n_steps++;
      }else{
        direction = rand() % 8;
      }
      sleep(1);
    }
    // Closing the socket file descriptor
    close(sockfd);
    return 0;
}
