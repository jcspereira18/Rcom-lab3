/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

#define F 0x5C
#define A 0x01
#define C 0x07
#define BCC A^C

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_RCV,
    END
} UA_State;

int UA_read(int fd)
{
    uint8_t temp;
    int res;

    UA_State currentState = START;

    printf("Writing back UA\n");
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
                    printf("\tReceived: %x\n", temp);
                    currentState = FLAG_RCV;
                }
            break;
            
            case FLAG_RCV:

                if(temp == F){
                    printf("\tReceived: %x\n", temp);
                    currentState = FLAG_RCV;
                }
                else if(temp == A){
                    printf("\tReceived: 0%x\n", temp);
                    currentState = A_RCV;
                }
                else
                    currentState = START;
                
            break; 

            case A_RCV:
                
                if(temp == F){
                    printf("\tReceived: %x\n", temp);
                    currentState = FLAG_RCV;
                }
                else if(temp == C){                  
                    printf("\tReceived: 0%x\n", temp);
                    currentState = C_RCV;
                }
                else 
                    currentState = START;
                
            break;

            case C_RCV:
                
                if(temp == F){
                    printf("\tReceived: %x\n", temp);
                    currentState = FLAG_RCV;
                }
                else if(temp == BCC){
                    printf("\tReceived: 0%x\n", temp);
                    currentState = BCC_RCV;
                }
                else 
                    currentState = START;

            break;

            case BCC_RCV:

                if(temp == F)
                    currentState = END;
                
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
    int fd,c, res, res_read;
    struct termios oldtio,newtio;
    char buf[255] = {0};
    int i, sum = 0, speed = 0;

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


    fd = open(argv[1], O_RDWR | O_NOCTTY );
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

    //send SET
    buf[0]=0x5C;
    buf[1]=0x01;
    buf[2]=0x03;
    buf[3]=0x01^0x03;
    buf[4]=0x5c;

    res = write(fd,buf,strlen(buf));
    printf("%d bytes written SET\n", res);
    

    res_read = UA_read(fd);

    if(res_read == 0)
        printf("Read UA correctly\n");
    else 
        printf("Error read UA\n");



#if 1
    char ch;

    printf("Write a message: ");
    for (i =0; (i<255) && ((ch = getchar()) != EOF) && (ch != '\n'); i++) 
        buf[i] = ch;
    buf[i] = '\0';

    printf("Writing \"%s\"\n", buf);



    // for (i = 0; i < 255; i++) {
    //     buf[i] = 'a';
    // }

    /*testing*/
    // buf[25] = '\n';

    res = write(fd,buf,strlen(buf) + 1);
    printf("%d bytes written\n", res);


    res = read(fd,buf,255);

    printf("Read %d bytes of a string of %d: %s\n", res, strlen(buf) + 1, buf);

    /*
    O ciclo FOR e as instruções seguintes devem ser alterados de modo a respeitar
    o indicado no guião
    */
#endif 

    sleep(1);

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }


    close(fd);
    return 0;
}
