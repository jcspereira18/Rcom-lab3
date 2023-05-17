/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int STOP=FALSE;
int fd, cont = 0, flag=1;

#define F 0x5C
#define A 0x01
#define C 0x07
#define BCC A^C
#define ESC 0x5D

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_RCV,
    END
} UA_State;


int send_message(int fd)
{
    int res, i;
    char ch, buf[255];

    printf("\nWrite a message: ");
    for (i =0; (i<255) && ((ch = getchar()) != EOF) && (ch != '\n'); i++) 
        buf[i] = ch;
    buf[i] = '\0';

    printf("Writing \"%s\"\n", buf);
 
    res = write(fd,buf,strlen(buf) + 1);
    printf("%d bytes written\n", res);


    res = read(fd,buf,255);

    printf("Read %d bytes of a string of %d: %s\n", res, strlen(buf) + 1, buf);

    return 0;
}

int send_SET(int fd)
{
    int res;
    char buf[255]={0};

    buf[0]=0x5C;
    buf[1]=0x01;
    buf[2]=0x03;
    buf[3]=0x01^0x03;
    buf[4]=0x5C;

    res = write(fd,buf,strlen(buf));
    printf("%d bytes written SET\n", res);

    return 0;   
}

int UA_read(int fd)
{
    uint8_t temp = 0;
    int res;

    UA_State currentState = START;

    printf("\nWriting back UA\n");
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

void alarme()
{
    int res;
    char buf[255]={0};
    //send SET
    buf[0]=0x5C;
    buf[1]=0x01;
    buf[2]=0x03;
    buf[3]=0x01^0x03;
    buf[4]=0x5C;

    printf("\nDidn´t receive UA\n");
    res = write(fd,buf,strlen(buf));
    printf("%d bytes written SET\n", res);
    
    cont ++;
  
    if (cont == 5){
        printf("Tou farto de esperar, ate logo\n"); 
        exit(1);
    }
        
    alarm(3);
    
}


int write_frame(int fd)
{
    int res, i;
    char buf[255]={0} , buf2[255]={0x5c, 0x5D, 0x01, 0x06, 0x01};
    char bcc = buf2[0];

    buf[0]=0x5C;
    buf[1]=0x01;
    buf[2]=0x03;
    buf[3]=0x01^0x03;
    
    write(fd,buf,strlen(buf));

    for(i = 0; i < strlen(buf2); i++)
    {   
        if(buf2[i] == 0x5C){ //0x5c -> ESC 0x7c
            char aux[255] = {ESC, 0x7C};              
            write(fd, aux, 2);    
        }

        else if(buf2[i] == 0x5D){ //0x5d -> ESC 0x7d
            char aux2[255] = {ESC, 0x7D};
            write(fd, aux2, 2);
        }
        
        else
            write(fd,&buf2[i],1);
    }

    for(i = 1; i < strlen(buf2); i++)
        bcc = buf2[i] ^ bcc;

    printf("\tBCC2: 0x%x\n", bcc);

    buf[4]=bcc;
    write(fd,&buf[4],1);

    buf[5]=0x5C;
    write(fd,&buf[5],1); 

    //printf("%d bytes written SET\n", res);

    return 0;   
}


int main(int argc, char** argv)
{
    int c, res, res_read, res_write, res_frame;
    struct termios oldtio,newtio;
    char buf[255] = {0};
    int sum = 0, speed = 0;

    if ( (argc < 2) ||
         ((strcmp("/dev/ttyS10", argv[1])!=0) &&
          (strcmp("/dev/ttyS1", argv[1])!=0) )) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }


    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */


    fd = open(argv[1], O_RDONLY | O_RDWR );
    if (fd < 0) { perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
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

    (void) signal(SIGALRM, alarme);

    printf("\nSend SET\n");
    res_write = send_SET(fd);
    if (res_write != 0)
        printf("Error sending SET\n");
    
    alarm(3);

    res_read = UA_read(fd);
    alarm(0);

    if(res_read == 0)
        printf("Read UA correctly\n");
    else 
        printf("Error read UA\n");

    printf("\nSend Frame\n");
    res_frame = write_frame(fd);
    if(res_frame != 0)
        printf("Error writing Frame\n");


    res = send_message(fd);
    if(res != 0)
        printf("Error sending message\n");

    sleep(1);

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }


    close(fd);
    return 0;
}
