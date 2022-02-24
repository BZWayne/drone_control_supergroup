
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

#define ACC_SIZE 10
#define PORT 7777
#define LINES 40
#define EACH_LINE 40
#define DRONE_SIZE 0.5

struct posi {
double x;
double y;
double fuel;
char online;
char group;
};

unsigned long int data_size = 1000000;
int accepted_sock[ACC_SIZE];
char accepted_flag[ACC_SIZE];
struct posi positions[ACC_SIZE];  

int sockfd, newsockfd, addr_len;
struct sockaddr_in serv_addr, cli_addr;

int biggest_fd = 1023; //I take the highest value of fd 
fd_set readfds;

void flush_show(int pos) // flush the screen by using postion stored here:  positions[ACC_SIZE];  
{
  int i,j,k;
  //transform them in integer
  int pos_x =0;
  int pos_y =0;
  char printed = 0;
  //fputs("\033[A\033[2K\033[A\033[2K",stdout);
  system("clear");
  fflush(stdout);
//printf("now");
// printing

 for(i = 0 ; i < 40 ; i ++ ){
printf("\n");
fflush(stdout);
 for(j = 0 ; j < 80 ; j ++ ){
 printed = 0;
 for(k = 0 ; k < ACC_SIZE ; k ++ ){ //check which drone here
  if(positions[k].online)
{
   pos_x = (int) positions[k].x;
   pos_y = (int) positions[k].y;
  if(pos_x==j&&pos_y==i)
{
printf("%c",positions[k].group);
printed = 1;
break;
}
}

} //for(acc_size)
if(!printed)
printf(" ");


}
}	
}
int predict_collision(int seq_num,char _group,double _x,double _y) // reuturn 1 if collision detected
{
if(_x>80||_y>40)
return 1;

int i= 0;
for(i = 0;i<ACC_SIZE;i++)
{
if(i!=seq_num &&positions[i].online==1)//Dont compute disdance with itself
{
float x_sq,y_sq;
x_sq = (positions[i].x-_x)*(positions[i].x-_x);
y_sq = (positions[i].y-_y)*(positions[i].y-_y);
  
if(sqrt(x_sq+y_sq)<=DRONE_SIZE*2)
return 1;
}

}

return 0;
}

void init() //preparation of TCP server
{
     FD_ZERO(&readfds);
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        perror("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
     serv_addr.sin_port = htons(PORT);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              perror("ERROR on binding");
     listen(sockfd,20);
     addr_len = sizeof(cli_addr);      
}

void *accepter()
{
//accept new socket and add to select group
int i =0;
while(1)
{
	usleep(1);
	for(i=0;i<ACC_SIZE;i++)
	{
	if(accepted_flag[i]) //choose available postion to store
	continue;
	
    	accepted_sock[i] = accept(sockfd, (struct sockaddr *) &cli_addr, &addr_len);
    	  if (accepted_sock[i]  < 0) 
          perror("ERROR on accept");
	accepted_flag[i] = 1; //mark accepted
	int status = fcntl(accepted_sock[i], F_SETFL, fcntl(accepted_sock[i], F_GETFL, 0)|O_NONBLOCK);
	FD_SET(accepted_flag[i], &readfds);//add select
	}
}
}

void receiver() //consumer
{
char buffer[1500] = ""; //buffer for recving data
struct timeval timeout;
timeout.tv_sec = 0;
timeout.tv_usec = 1000;
int ready = 0,i = 0, len = -1;
int tem_group_num = -1;
double tem_x = 0,tem_y = 0,tem_fuel = 0;
char tem_group;
while(1)
{


for(i = 0;i<ACC_SIZE;i++)
{

 if(accepted_flag[i])
  {
usleep(1000);
    len = read(accepted_sock[i],buffer,1400);// [goup],[x],[y],[fuel_left], --- group A->Z
    if(len>0)
    {
    sscanf(buffer,"%c,%lf,%lf,%lf,",&tem_group,&tem_x,//resolve the position
    &tem_y,
    &tem_fuel);
//printf("data recvd %s\n",buffer);
    if(predict_collision(i,tem_group,tem_x,tem_y)==1)
    {
 //printf("mov deny \n");
    int tem_sen = write(accepted_sock[i],"0",sizeof("0")); //deny movement
    }
    else //if(predict_collision(i,tem_group,tem_x,tem_y)==0)
    {
    int tem_sen = write(accepted_sock[i],"1",sizeof("1")); // confirm movement
   fsync(accepted_sock[i]);
    positions[i].group = 'A'+i ;
    positions[i].x = tem_x;
    positions[i].y = tem_y;
    positions[i].fuel = tem_fuel;
    positions[i].online = 1;
    flush_show(i);
    }
    }
    else if(len==0) // if offline remove this socket
    {
    FD_CLR(accepted_sock[i], &readfds);
    accepted_sock[i] = -1;
    accepted_flag[i] = 0;
    positions[i].online = 0;
    }


  }
}
}
}


int main(int argc, char *argv[])
{
//flush_show(1);
  init();
     int ret=0;
     pthread_t accpt;
     ret= pthread_create(&accpt,NULL,(void*)accepter,NULL ); //new thread to accept new incoming socket
     if(ret)
     {
         printf("create error\n");
         return 1;
     }
  pthread_detach(accpt); 
  receiver();
return 0;
}


