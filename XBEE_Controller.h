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
 */


#define PORT 3054
#define SERVER_IP "127.0.0.1"

/* this structure is highly dependent 
 * on what you have attached to the XBEE 
 * and end up doing with the data.
 */
typedef struct {
    float celsius;
} XBEE_Message_Output;

typedef struct {
    void (*digital_processor)(int id, unsigned char data1, unsigned char data2, XBEE_Message_Output *output);
    void (*analog_processors[16])(int id, unsigned char data1, unsigned char data2, XBEE_Message_Output *output);

    // ... add more as needed
} XBEE_Config; 



