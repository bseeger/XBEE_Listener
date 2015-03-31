/* 
 * XBEE_controller (c) Copyright 2015 B. Seeger
 * XBEE_controller can be copied and distributed freely for 
 * any non-commercial purposes. XBEE_controller can only be 
 * incorporated into commercial software with the permission 
 * of the current author [B. Seeger]. 
 * 
 * Written By:  B. Seeger
 * Date:  1/23/2015
 * 
 *
 * XBEE_controller - listens for XBEE broadcast on port PORT
 * and will print out information to stdout based on what
 * it finds. 
 * You can configure this software to work with many different 
 * devices hanging off of the XBEE.  
 *
 * Currently supported devices: 
 *    1) TMP36   - callback: TMP36_processor
 *
 *
 * @TODO - add timeout so the client doesn't wait forever to 
 *         get data from a potentially dead XBEE
 *
 * 
 */

#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>

#include "XBEE_Controller.h"

#define VREF 2500 /* 250 mv, or 1250 (1.25 mv) */ 

#define MSB 0
#define LSB 1

#define TRUE 1
#define FALSE 0

int g_verbose = FALSE;
int g_mo_mode = FALSE;  /* minimal output mode */

typedef struct  {
    unsigned char number1[2];     /* 0, 1 */
    unsigned char number2[2];     /* 2, 3 */
    unsigned char packetID;       /* 4 */
    unsigned char encryptPad;     /* 5 */
    unsigned char commandID;      /* 6 */
    unsigned char commandOpt;     /* 7 */
    unsigned char numSamples;     /* 8 */
    unsigned char digitalMask[2]; /* 9, 10 */
    unsigned char analogMask;     /* 11 */
    /* If digitalMask is anything other then zero, the samples[0] & samples[1] will 
     * represent the digital Sample.  Then the analog samples will start at samples[2].
     *
     * If digitalMask == 0, then there will be no digital sample and analog samples will
     * start at samples[0].
     */
    unsigned char samples[32];    
} XBEE_Message;

#define TMP36_OFFSET_VOLTAGE 500  /* think 500 mv = 0 celsius */
void TMP36_processor(int id, unsigned char data1, unsigned char data2, XBEE_Message_Output *output) {
   
    int AD_dec = (data1 << 8) | data2;
    float AD_mv = ((AD_dec * VREF) / 1023) - TMP36_OFFSET_VOLTAGE;
    float celsius = (AD_mv / 10);    /* 10 mv per degree C */
    float fahrenheit = celsius * 1.8 + 32;
    if (g_verbose) {
        printf("Signal %d: A/D dec: %d   A/D mV: %g  %g C %g f\n", id, AD_dec, AD_mv, celsius, fahrenheit);
    }

    if (g_mo_mode) {
        printf("%g,", fahrenheit);
    } else {
    	printf("Signal %d: %g f\n", id, fahrenheit);
    }
   
    // check this here, so we can at least get printf's if the output is not set.  
    if (output) {
        output->celsius = celsius;
    }
}

void quit(char *msg) {
    perror(msg);
    exit(EXIT_SUCCESS); // should this be failure?  
}

void signal_handler(int sig) { 
    if (sig == SIGINT) {
        printf("Good Bye!");
        exit (EXIT_SUCCESS);
    } 

    printf("Recieved signal %d (%s)\n", sig, strsignal(sig));
    exit (EXIT_SUCCESS);
} 

void print_XBEE_Message(XBEE_Message *message) {
    
    if (message == NULL) return;

    printf("Number 1:   %02X %02X\n", message->number1[0], message->number1[1]);
    printf("Number 2:   %02X %02X\n", message->number2[0], message->number2[1]);
    printf("Packet ID:  %02X\n", message->packetID);
    printf("Encryption Pad:  %02X\n", message->encryptPad);
    printf("Command ID: %02X\n", message->commandID);
    printf("Command Options: %02X\n", message->commandOpt);
    printf("Number of Samples: %02X\n", message->numSamples);
    printf("Digital Mask:   %02X %02X\n", message->digitalMask[MSB], message->digitalMask[LSB]);
    printf("Analog Mask: %02X\n", message->analogMask);

    int spot = 0; 
    int sampleNo = 0;

    if (message->digitalMask[MSB] & 0xFF || message->digitalMask[LSB] & 0xFF) { 
        printf("Digital Sample:   %02X %02X\n", message->samples[sampleNo], message->samples[sampleNo+1]);
        sampleNo += 2;
    } 

    for (spot = 0; spot < 8; spot++) {
        unsigned char mask = 1 << spot;
        if (mask & message->analogMask) { 
            printf("Analog %d:   %02X %02X\n", sampleNo / 2, message->samples[sampleNo], message->samples[sampleNo+1]);
        }
        sampleNo += 2;
    }
}

void usage(char *progname) {
    printf("Usage: \n\tXBEE_Controller %s [-c NUM]\n", progname);
    printf("\t-c NUM    where NUM is the number of times to listen for data. Default: 1\n");
    printf("\t-m 	minimal output mode, just output TMP1, TMP2, etc.\n");
}

int main (int argc, char **argv) {
    
    struct sockaddr_in si_server;
    struct sockaddr_in si_mine;
    
    int option = 0;
    int run_count = 1;
    int timeout = 30; /* seconds */

    int sockfd = 0;
    socklen_t sockaddr_len = sizeof(si_server);
    int msgsize = 0;

    XBEE_Message message;
    XBEE_Config config;
    XBEE_Message_Output output;

    memset(&message, 0, sizeof(message));
    memset(&config, 0, sizeof(config));
    memset(&output, 0, sizeof(output));

    while ((option = getopt(argc, argv, "c:mv")) != -1) {
        switch(option) {
            case 'c':  
                run_count = atoi(optarg);
                break;
            case 'm':
                g_mo_mode = TRUE;
                break;
            case 'v':  
                g_verbose = TRUE;
                break;
            default:  usage(argv[0]);
        }
    }

    /* This directs the code to the right data processor, 
     * based on what is physically connected to your board. 
     * In my case, we have a TMP36 hooked up to analog 1  
     * and that is all. 
     */
    config.analog_processors[0] = &TMP36_processor;
    config.analog_processors[1] = &TMP36_processor;

    /* register signal handler, so we can kill the program gracefully */
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        quit("Unable to register signal handler for SIGINT");
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if (sockfd == -1) {
        quit("Unable to open socket");
    }

    memset((char*)&si_server, 0, sockaddr_len);

    memset(&(si_mine), 0, sizeof(si_mine));

    si_mine.sin_family = AF_INET;
    si_mine.sin_addr.s_addr = INADDR_ANY;
    si_mine.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&si_mine, sizeof(struct sockaddr)) == -1) { 
        quit("Bind failed");
    }

    if (g_verbose) {
    	printf("Listening on port %d now...\n", PORT);
    }

    while (run_count > 0) {

        if ((msgsize = recvfrom(sockfd, (unsigned char*)&message, sizeof(message), 0, (struct sockaddr *) &si_server, &sockaddr_len)) == -1) {
            quit("Received bad packet. Goodbye!\n"); 
        }
    
        if (g_verbose)  {
            printf("Got message. It is %d bytes long\n", msgsize);
            printf("-----\n");
            print_XBEE_Message(&message);
        }

        // verify that the first two numbers XOR into 0x4242
        unsigned char msb_fortytwo = message.number1[1] ^ message.number2[1];
        unsigned char lsb_fortytwo = message.number1[0] ^ message.number2[0];

        if (g_verbose) {
            printf("Sanity check (should be 0x4242) 0x%02X%02X\n", msb_fortytwo, lsb_fortytwo);
        }
        
        int sampleNo = 0;
        int spot = 0;
        if (message.digitalMask[MSB] & 0xFF || message.digitalMask[LSB] & 0xFF) { 
            // this is untested - I don't have anything on the DIO stuff. 
            if (config.digital_processor != NULL) {
                config.digital_processor(sampleNo/2, message.samples[sampleNo], message.samples[sampleNo+1], &output);
            }
            // Always move past, since there is a digital sample, even if there is 
            // no processor for it. 
            sampleNo+=2; // move past digital sample
        }

        for(spot = 0; spot < 8; spot++) {
            unsigned char mask = 1 << spot;
            if (message.analogMask & mask) { 
                /* then run this spot! */
                if (config.analog_processors[spot] != NULL) {
                    if (g_verbose) {
                        printf("Processing analog signal %d. \n", spot);
                    }
                    config.analog_processors[spot](sampleNo/2, message.samples[sampleNo], message.samples[sampleNo+1], &output);
                } else {
                    printf("Skipping analog signal %d: no processor registered for it.\n", spot+1);
                }
                sampleNo+=2;
            } 
        }

        // TODO - ship off the XBEE_Message_Output to somewhwere. 

        memset(&message, 0, sizeof(message));
        run_count -= 1;
   }

   if (g_verbose) {
   	printf("Goodbye!");
   }
   exit(EXIT_SUCCESS);
}

