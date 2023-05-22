#include "frame.h"
#include "linklayer.h"

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

int send_SET(int fd)
{
    int res;
    char buf[buf_size]={F, A_set, C_set, A_set^C_set, F};

    res = write(fd,buf,strlen(buf));
    printf("%d bytes written SET\n", res);

    return 0;   
}

int send_UA(int fd)
{
    int res;
    char buf[buf_size]={F, A_ua, C_ua, A_ua^C_ua, F};

    res = write(fd,buf,strlen(buf));
    printf("%d bytes written UA\n", res);

    return 0;
}

//char sequence_number = 0;

int write_frame(const char* buf, int bufSize, int fd)
{
    int res, i, frame_size = 4;
    char frame[2*buf_size]={F, A_set, C_set, A_set^C_set};
    char bcc = 0;


    for(i = 0; i < bufSize; i++)
    {   
        bcc = buf[i] ^ bcc;

        if(buf[i] == F){ //0x5c -> ESC 0x7c
           frame[frame_size++] = ESC;
           frame[frame_size++] = 0x7C;            
        }

        else if(buf[i] == ESC){ //0x5d -> ESC 0x7d 
            frame[frame_size++] = ESC;
            frame[frame_size++] = 0x7D;             
        }
        
        else
            frame[frame_size++] = buf[i];
    }

    frame[frame_size++] = bcc;

    frame[frame_size++] = F;

    //frame[frame_size++] = '\0';

    //for (i=0; i< strlen(frame); i++)
    //    printf("buf[%d] = %02x\n", i, frame[i]);

    printf("\tBCC2: 0x%02x\n", bcc);

    printf("tam:%d\n", frame_size);
    res = write(fd, frame, frame_size); 
    printf("%d bytes written frame\n", res);

    return 0;   
}


int read_frame(char* packet, int fd){
    uint8_t temp = 0, address, control;
    int res, i = 0, aux=0;
    char bcc;

    int frame_size = 0;

    FRAME_State currentState = START;

    while(currentState != END){

        switch(currentState){

            case START:

                if (read(fd, &temp, 1) < 0) 
                    return -1; 

                if(temp == F){                    
                    printf("\tReceived: 0x%02x\n", temp);
                    currentState = FLAG_RCV;
                }

            break;
            
            case FLAG_RCV:

                if (read(fd, &temp, 1) < 0) 
                    return -1;

                if(temp == F){
                    printf("\tReceived: 0x%02x\n", temp);
                    currentState = FLAG_RCV;
                }
                else if(temp ==0x03 || temp == 0x01){
                    printf("\tReceived: 0x%02x\n", temp);
                    address = temp;
                    currentState = A_RCV;
                }                            
                
            break; 

            case A_RCV:

                if (read(fd, &temp, 1) < 0) 
                    return -1;

                if(temp == F){
                    printf("\tReceived: 0x%02x\n", temp);
                    currentState = FLAG_RCV;
                }
                else{                  
                    printf("\tReceived: 0x%02x\n", temp);
                    control = temp;                    
                    currentState = C_RCV;
                }
                               
            break;

            case C_RCV:

                if (read(fd, &temp, 1) < 0) 
                    return -1;
                
                if(temp == F){
                    printf("\tReceived: 0x%02x\n", temp);
                    currentState = FLAG_RCV;
                }

                if(temp == control ^ address){
                    printf("\tReceived: 0x%02x\n", temp);                    
                    currentState = BCC_RCV;                   
                }
                
            break;

            case BCC_RCV:

                if (read(fd, &temp, 1) < 0) 
                    return -1;

                if(temp == ESC){
                    aux = 1;
                    printf("\tReceived: 0x%02x\n", temp);
                    currentState = ESC_RCV;
                }
                
                else if (temp == F){
                    if(aux == 0){                        
                        printf("\tReceived: 0x%02x\n", temp);
                        currentState = END;
                        printf("erro 490394\n");
                    }
            
                    if(aux == 1){                        
                        printf("\tReceived: 0x%02x\n", temp);
                        //for(i = 0; i < strlen(buf); i++)
                        //    printf("\tbuf[%d]: 0x%x\n", i, buf[i]);
                        currentState = BCC2;
                        printf("erro90382490\n");
                    }
                } 
                
                else { 
                    aux = 1;
                    printf("\tReceived: 0x%x\n", temp);                   
                    packet[frame_size++] = temp;
                    
                } 
 
            break;

            case ESC_RCV:

                if (read(fd, &temp, 1) < 0) 
                    return -1;

                if(temp == 0x7D){
                    printf("\tReceived: 0x%02x\n", temp);
                    packet[frame_size++] = ESC;
                    currentState = BCC_RCV;
                }
                else if(temp == 0x7C){
                    printf("\tReceived: 0x%02x\n", temp);
                    packet[frame_size++] = F;
                    currentState = BCC_RCV;
                }
                else {
                    printf("Error: unknown escape sequence\n");
                    return -1;
                }

            break;

            case BCC2:

                bcc = 0;
                printf("erro284938\n");
                for(i = 0; i < frame_size-1; i++){
                    //printf("fodjfo: %x\n", i);
                    bcc = packet[i] ^ bcc;
                }
                printf("tam:%d\n", frame_size);
                printf("\tBCC2: 0x%02x\n", bcc);

                printf("bccccc: %x\n", packet[frame_size - 1]);

                if(bcc==packet[frame_size - 1]){
                    currentState = END;
                }
                else{
                    printf("Error na transmissao de dados\n");
                    return -1;
                }

            break;

            case END:
            break; 
        }
    }

    return frame_size-1;
}