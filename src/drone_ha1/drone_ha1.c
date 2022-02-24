/*
code for connecting and sending and recving using sockts TCP
IP addr is 127.0.0.1
port: 7777
should send: a,1.222,2.333,99.00,


compile with: gcc drone.c -lpthread -o drone -lm
run with: ./drone

*/

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

char reply[256];
int replyint=0;
double x=40;
double y=20;
double fuel_left=100;
char msg[50];
int XYarr[40][80] ={{0}};
int zx=1;
int zy=40;

void error(char *msg, int fd){
    //error checking function
    perror(msg);
    shutdown(fd, SHUT_RDWR);
    close(fd);
    exit(0);
}

float genno(int a){
    //function to get random values to assign at the coordinates
  float new;
  srand((unsigned int)time(NULL));

  if(a==80){
    //x condtion
    new=(((float)rand()/(float)(RAND_MAX)) * zx);
    zx++;
    if (zx==80){
      zx=0;
    } 
  }
  else if(a==40){
    //y condtion
    new=(((float)rand()/(float)(RAND_MAX)) * zy);
    zy--;
    if (zy==0){
      zy=40;
    }
  }
  return new;

}

void printingscannedarea(){
  //function to calculate the area scanned by the drone in position x,y in a 2D array
  printf("\n");
  printf("\n");
  printf("Fuel left is:%d, The scanned area is: \n",(int)fuel_left);
  if(XYarr[(int)y][(int)x]==0){
    //mark the scaned area.
    if ((int)y-1>=0 && (int)x-1>=0)
      XYarr[(int)y-1][(int)x-1]=1;

    if((int)y-1>=0&&(int)x>=0)
      XYarr[(int)y-1][(int)x]=1;

    if((int)y-1>=0&&(int)x+1<=80)
      XYarr[(int)y-1][(int)x+1]=1;



    if((int)y>=0 && (int)x-1>=0)
      XYarr[(int)y][(int)x-1]=1;

    if((int)y>=0 && (int)x>=0)
      XYarr[(int)y][(int)x]=1;

    if((int)y>=0 && (int)x+1<=80)
      XYarr[(int)y][(int)x+1]=1;



    if((int)y+1<=40 && (int)x-1>=0)
      XYarr[(int)y+1][(int)x-1]=1;

    if((int)y+1<=40 && (int)x>=0)
      XYarr[(int)y+1][(int)x]=1;

    if((int)y+1<=40 && (int)x+1<=80)
      XYarr[(int)y+1][(int)x+1]=1;
  }
  for(int i=0; i<40; i++) {
    for(int j=0; j<80; j++) {
      printf("%d ", XYarr[i][j]);
    }
    printf("\n");
  }
}

void generate_msg(){
     //function to generate the message sent to the master code.
    char str_x[50];
    char str_y[50];
    char str_fuel_left[50];
    char comma[]= ",";
    char groupname[]="h,";
    char endofstr[]="\0";
    sprintf(str_x, "%.3lf", x);
    sprintf(str_y, "%.3lf", y);
    sprintf(str_fuel_left, "%.3lf", fuel_left);
    
    strcpy(msg, groupname);
    strcat(msg, str_x);
    strcat(msg, comma);
    strcat(msg, str_y);
    strcat(msg, comma);
    strcat(msg, str_fuel_left);
    strcat(msg, endofstr);
    printf("what will be sent is: %s\n", msg);
}

void updateXY(){
  //function to get new coordinates from genno() to explore the space
  int flag=0;
  double x_old=x;
	double y_old=y;
	
	x_old=x;
	y_old=y;
	double y_new =genno(40);
	double x_new= genno(80);
	double dif_x=abs(x_old-x_new);
	double dif_y=abs(y_old-y_new);
  //constraint on the new generated coordinates and checking on the drone's fuel.
  if (fuel_left <= ((sqrt(pow(x, 2) + pow(y, 2))))/4){
    printf("Fuel is about to end i will go back to refuel\n");
    x=0;
    y=0;
    fuel_left=100;
    flag=1;
  }

	if (flag==0 && fuel_left>0 && XYarr[(int)y_new][(int)x_new]!=1){
    x=x_new;
    y=y_new;
    fuel_left = fuel_left -((sqrt(pow(dif_x, 2) + pow(dif_y, 2)))/20);
    if(fuel_left<0)
      fuel_left=0;
	}
  flag=0;
  
}
int main(){

    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    portno = 7777;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
      error("ERROR opening socket",sockfd);
    }
    //getting information by the host
    char ipadd[]={'1','2','7','.','0','.','0','.','1','\0'};
    server = gethostbyname(ipadd);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
      error("ERROR connecting",sockfd);
    }
    while(1){
    sleep(1);
    generate_msg();
    n = write(sockfd,msg,strlen(msg));
    if (n < 0){
      error("ERROR writing to socket",sockfd);
    }
    bzero(reply,256);
    n = read(sockfd,reply,255);
    sscanf(reply, "%d",&replyint);
    if (replyint == 0){
      printf("motion was rejected\n");
    }
    else if (replyint==1)
    {
      printingscannedarea();
      updateXY();
    }
    else{
      error("Wrong reply from the master",sockfd);
    }
    
    if (n < 0){
      error("ERROR reading from socket",sockfd);
    }
    }
}
