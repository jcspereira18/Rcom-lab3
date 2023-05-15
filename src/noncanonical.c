/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int STOP=FALSE;

#define F 0x5C
#define A 0x01
#define C 0x03
#define BCC A^C
#define ESC 0x5D


typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_RCV,
    ESC_RCV,
    BCC2,
    END
} FRAME_State;

int received_message(int fd)
{
    int i=0, res;
    char msg[255], buf[255];

    while (STOP==FALSE) {       
        res = read(fd,buf,1);   
                     
        if (buf[0]=='\0') STOP=TRUE;
        msg[i] = buf[0];
        i++;
    }

    printf("Writing back\n"); 
    res = write(fd, msg, i);
    printf("\tMessage: %s :%d\n", msg, res);
    
    return 0;
}

int send_UA(int fd)
{
    int res;
    char buf[255]={0};

    buf[0]=0x5C;
    buf[1]=0x01;
    buf[2]=0x07;
    buf[3]=0x01^0x07;
    buf[4]=0x5C;

    res = write(fd,buf,strlen(buf));
    printf("%d bytes written UA\n", res);

    return 0;
}

int SET_read(int fd)
{
    uint8_t temp;
    int res;

    FRAME_State currentState = START;
   
    while(currentState != END){

        res = read(fd, &temp, 1);

        if (res < 0) 
            return NULL;  // erro
 
        if (res == 0) {
            if (currentState != END) 
                return NULL;  // erro
            else 
                break;  // sucesso
        }

        switch(currentState){
            case START:

                if(temp == F){
                    printf("\tReceived: 0x%x\n", temp);
                    currentState = FLAG_RCV;
                }
            break;
            
            case FLAG_RCV:

                if(temp == F){
                     printf("\tReceived: 0x%x\n", temp);
                    currentState = FLAG_RCV;
                }
                else if(temp == A){
                     printf("\tReceived: 0x0%x\n", temp);
                    currentState = A_RCV;
                }
                
               else
                    currentState = START;
               
            break; 

            case A_RCV:
                
                if(temp == F){
                     printf("\tReceived: 0x%x\n", temp);
                    currentState = FLAG_RCV;
                }
                else if(temp == C){
                    printf("\tReceived: 0x0%x\n", temp);
                    currentState = C_RCV;
                }
                else 
                    currentState = START;
                
            break;

            case C_RCV:
                
                if(temp == F){
                     printf("\tReceived: 0x%x\n", temp);
                    currentState = FLAG_RCV;
                }
                else if(temp == BCC){
                    printf("\tReceived: 0x0%x\n", temp);
                    currentState = BCC_RCV;
                }
                else 
                    currentState = START;

            break;

            case BCC_RCV:

                if(temp == F){
                    printf("\tReceived: 0x%x\n", temp);
                    currentState = END;
                }
                else
                    currentState = START;

            break;

            case END:
            break; 
        }
    }

    return 0;
}



int main(int argc, char** argv)
{
    int fd, c, res, res_read, res_write, res_frame;
    struct termios oldtio,newtio;
    char buf[255];

    if ( (argc < 2) ||
         ((strcmp("/dev/ttyS10", argv[1])!=0) &&
          (strcmp("/dev/ttyS11", argv[1])!=0) )) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }


    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd < 0) { perror(argv[1]); exit(-1); }

    if (tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

    /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) próximo(s) caracter(es)
    */

    
    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    printf("Writing back SET\n");
    res_read = SET_read(fd);

    if(res_read == 0)
        printf("Read SET correctly\n");
    else 
        printf("Error read SET\n");

    res_write = send_UA(fd);
    if (res_write != 0)
        printf("Error sending SET\n");

    
    res = received_message(fd);
    if(res != 0)
        printf("Error received message\n");

    sleep(1);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
