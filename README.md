# arp_supergroup

## Contributors

|  Group name   |   Students                                                                     |
| ------------- | ------------------------------------------------------------------------------ |
| master        | Zhouyang Hong, Gesualdo Sinatra                                                |
| drone_bm3     | Bauyrzhan Zhakanov, Madi Nurmanov                                              |
| drone_awais   | Awais Tahir                                                                    |
| drone_yh11    | Ali Yousefi, Mohammad Reza Haji Hosseini                                       |
| drone_ja1     | Jabrail Chumakov, Ayan Mazhitov                                                |
| drone_ha1     | Alice Maria Catalano, Hussein Ahmed Fouad Hassan, Youssef Mohsen Mahmoud Attia |

## Description

Third assignment of the course “Advanced robot programming”. We were asked to code a program which allows the user to control the motion of several drones avoiding collisions and visualizing them printed on the screen. 
The master is opened when the “./run” command is executed. The master is the server which will hosts the drones (clients) also in running time using sockets. The drones will connect and they start sending positions, the positions are confirmed after taking care that the direction is actually free by the server.
  

## Installation
```
git clone https://github.com/BZWayne/arp_supergroup
```
```
chmod +x *
```
```
install.sh 
```

## Running
```
run.sh 
```
