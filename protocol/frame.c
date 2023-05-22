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

char* last_frame = NULL;
int last_frame_size = 0;

int send_SET(int fd)
{
    printf("\nSend SET\n");

    int res;
    char buf[buf_size]={F, A_set, C_set, A_set^C_set, F};

    if (last_frame != NULL) {
        free(last_frame);
    }

    last_frame = malloc(5);
    last_frame_size = 5;
    memcpy(last_frame, buf, 5);

    res = write(fd,buf,strlen(buf));
    printf("%d bytes written SET\n", res);

    return 0;   
}

int read_SET(int fd)
{
     printf("\nRead SET\n");

    uint8_t temp, address, control;
    
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
                else if( temp == 0x01){
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
                    if (temp != C_set) {
                        printf("erro: frame errada. Queria %02x, recebeu %02x\n", C_set, temp);
                        // return -1;
                    }                    
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
            
                if (temp == F){                      
                        printf("\tReceived: 0x%02x\n", temp);
                        currentState = END;
                    }               
        }
    }   
    return 0;
}

int send_UA(int fd)
{
    printf("\nSend UA\n");

    int res;
    char buf[buf_size]={F, A_ua, C_ua, A_ua^C_ua, F};

    if (last_frame != NULL) {
        free(last_frame);
    }

    last_frame = malloc(5);
    last_frame_size = 5;
    memcpy(last_frame, buf, 5);

    res = write(fd,buf,strlen(buf));
    printf("%d bytes written UA\n", res);

    return 0;
}

int read_UA(int fd) {

    printf("\nRead UA\n");

    uint8_t temp, address, control;
    
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
                else if(temp ==0x03){
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
                    if (temp != C_ua) {
                        printf("erro: frame errada. Queria %02x, recebeu %02x\n", C_ua, temp);
                        return -1;
                    }                    
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
                
                else if (temp == F){                       
                    printf("\tReceived: 0x%02x\n", temp);
                    currentState = END;
                
                }
        }
    }
    return 0;
}

int send_DISC(int fd)
{

    printf("\nSend DISC\n");

    int res;
    char buf[buf_size]={F, A_ua, DISC, A_ua^DISC, F};

    if (last_frame != NULL) {
        free(last_frame);
    }

    last_frame = malloc(5);
    last_frame_size = 5;
    memcpy(last_frame, buf, 5);

    res = write(fd,buf,strlen(buf));
    printf("%d bytes written UA\n", res);

    return 0;
}

int read_DISC(int fd) {

    printf("\nRead DISC\n");

    uint8_t temp, address, control;
    
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
                else if(temp ==0x03){
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
                    if (temp != DISC) {
                        printf("erro: frame errada. Queria %02x, recebeu %02x\n", DISC, temp);
                        return -1;
                    }                    
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
                
                else if (temp == F){                       
                    printf("\tReceived: 0x%02x\n", temp);
                    currentState = END;
                
                }
        }
    }
    return 0;
}


char sequence_number = 0;

int send_frame(const char* buf, int bufSize, int fd)
{
    int res = write_frame(buf, bufSize, fd, I(sequence_number));

    if (res < 0) return res;

    sequence_number = 1 - sequence_number;

    do {
        // nao da segfault porque RR nao tem dados logo nao escreve nada no buf
        res = expect_frame(NULL, fd, RR(sequence_number));
    } while (res == -2);

    return res;
}

int write_frame(const char* buf, int bufSize, int fd, char control)
{
    int res, i, frame_size = 4;
    char frame[2*buf_size]={F, A_set, control, A_set^control};
    
    if (bufSize > 0) {
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
        printf("\tBCC2: 0x%02x\n", bcc);
    }

    frame[frame_size++] = F;

    res = write(fd, frame, frame_size);

    if (res < 0) return res;

    if (last_frame != NULL) {
        free(last_frame);
    }

    last_frame = (char*) malloc(frame_size);
    memcpy(last_frame, frame, frame_size);
    last_frame_size = frame_size;

    printf("%d bytes written frame\n", res);

    return res;   
}

int expect_frame(char* packet, int fd, char expected_control) {
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

                if(temp == F)        
                    currentState = FLAG_RCV;

            break;
            
            case FLAG_RCV:

                if (read(fd, &temp, 1) < 0) 
                    return -1;

                if(temp == F)
                    currentState = FLAG_RCV;
                
                else if(temp ==0x03 || temp == 0x01){
                    address = temp;
                    currentState = A_RCV;
                }                            
                
            break; 

            case A_RCV:

                if (read(fd, &temp, 1) < 0) 
                    return -1;

                if(temp == F)
                    currentState = FLAG_RCV;

                else{                  
                    if (temp != expected_control) {
                        printf("Erro: recebeu frame errada. Queria %02x, recebeu %02x\n", expected_control, temp);
                        return -2;
                    }                    
                    control = temp;
                    currentState = C_RCV;
                }
                               
            break;

            case C_RCV:

                if (read(fd, &temp, 1) < 0) 
                    return -1;
                
                if(temp == F)
                    currentState = FLAG_RCV;

                if(temp == control ^ address)                  
                    currentState = BCC_RCV;                   
                else {
                    printf("Erro: recebeu frame errada. Queria %02x, recebeu %02x\n", control ^ address, temp);
                    return -2;
                }
                
            break;

            case BCC_RCV:

                if (read(fd, &temp, 1) < 0) 
                    return -1;

                if(temp == ESC){
                    aux = 1;
                    currentState = ESC_RCV;
                }
                
                else if (temp == F){
                    if(aux == 0)                      
                        currentState = END;                
            
                    if(aux == 1)                                   
                        currentState = BCC2;                   
                } 
                
                else { 
                    aux = 1;              
                    packet[frame_size++] = temp;                  
                } 
 
            break;

            case ESC_RCV:

                if (read(fd, &temp, 1) < 0) 
                    return -1;

                if(temp == 0x7D){
                    packet[frame_size++] = ESC;
                    currentState = BCC_RCV;
                }
                else if(temp == 0x7C){
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

                for(i = 0; i < frame_size - 1; i++)                  
                    bcc = packet[i] ^ bcc;          
            
                printf("\tBCC2: 0x%02x\n", bcc);

                if(bcc==packet[frame_size - 1])
                    currentState = END;
                
                else{
                    printf("Error na transmissao de dados\n");
                    return -2;
                }

            break;

            case END:
            break; 
        }
    }

    return frame_size;
}

int read_frame(char* packet, int fd) {
    
    int res = expect_frame(packet, fd, I(sequence_number));

    if (res == -1) return res;
    else if (res == -2) {
        return write_frame(NULL, 0, fd, REJ(sequence_number));
    } 
    else {
        sequence_number = 1 - sequence_number;

        write_frame(NULL, 0, fd, RR(sequence_number));

        return res;
    }
}