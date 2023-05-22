#include "linklayer.h"
#include "frame.h"

int fd, count = 0, alarm_tries, alarm_timeOut;
struct termios oldtio,newtio;


void alarme()
{
    int res;

    printf("\nDidn´t receive UA\n");
    send_SET(fd);
    
    count ++;
  
    if (count == alarm_tries){
        printf("Tou farto de esperar, ate logo\n"); 
        exit(1);
    }
        
    alarm(alarm_timeOut);
    
}


// Opens a connection using the "port" parameters defined in struct linkLayer, returns "-1" on error and "1" on sucess
int llopen(linkLayer connectionParameters)
{
    int res_write, res_read;
    char buf[buf_size]={0};
    
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    if (fd < 0) { perror(connectionParameters.serialPort); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE_DEFAULT | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    (void) signal(SIGALRM, alarme);

    alarm_tries=connectionParameters.numTries;
    alarm_timeOut=connectionParameters.timeOut;

    if (connectionParameters.role == TRANSMITTER)
    {
        printf("\nSend SET\n");
        res_write = send_SET(fd);
        if (res_write != 0){
            printf("Error sending SET\n");
            return -1;
        }

        alarm(alarm_timeOut);

        res_read = read_frame(buf, fd);
        alarm(0);

        if(res_read == -1)
            printf("Connection established: Read UA\n");
        else{
            printf("Error: Connection\n");
            return -1;
        }
    }
 

    if(connectionParameters.role == RECEIVER)
    {
        printf("\nWriting back SET\n");
        res_read = read_frame(buf, fd);
        if(res_read == -1)
            printf("Connection established: Read SET\n");
        
        else{
            printf("Error: Connection %d\n", res_read);
            return -1;
        } 

        printf("\nSend UA\n");
        res_write = send_UA(fd);
        if (res_write != 0){
            printf("Error sending SET\n");
            return -1;
        }
    }

    return 1;

}

//Sends data in buf with size bufSize«
int llwrite(char* buf, int bufSize)
{
   int res;
   printf("\nSend Frame\n");
   res = write_frame(buf, bufSize, fd);
   if(res != 0){
       printf("Error writing Frame\n");
       return -1;
   }
   
   return 0;
   //return -1;
}

// Receive data in packet
int llread(char* packet)
{
    int res;

   printf("\nWriting back Frame\n");
    res = read_frame(packet,fd);
    if (res > 0){
        printf("Read frame correctly\n");
        return res;
    }
    else{
        printf("Error reading frame\n");
        return -1;
    }
}
// Closes previously opened connection; if showStatistics==TRUE, link layer should print statistics in the console on close
int llclose(linkLayer connectionParameters, int showStatistics)
{
    if(showStatistics==TRUE)
    {
        printf("Statistics: %d\n", showStatistics);
    }
    
    //if(connectionParameters.role == RECEIVER)
    //{
//
    //}

    sleep(1);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);

    return 0;
}