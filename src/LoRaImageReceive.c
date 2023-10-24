/*
 * LoRaImageReceive.c
 * 
 * Copyright 2023  <ubagley18@UldisRpi4BA>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include "LoRa.h"
#include "Image.h"

#include <stdio.h>
#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

/**
 * @brief Writes the jpg file into the specified path from a buffer 
 * @param jpgBuffer a pointer to the buffer that contains the jpg
 * @param filePath a pointer to the path to the file
 * @param fileSize a pointer to the size of the file
*/
void writeFileToPath(unsigned char* jpgBuffer, const char* newFilePath, int fileSize);

int nbOfFilesInDirectory(void);

static Image* image;
static char fileName[100]; //100 characters filename max
static int fileIndex;
int dataFlag;
unsigned char dataSize;
unsigned char* imageBufPtr;
static int* nbOfFilesReceived = NULL;

void tx_f(txData *tx){
    LoRa_ctl *modem = (LoRa_ctl *)(tx->userPtr);
    printf("tx done;\t");
    printf("sent string: \"%s\"\n", tx->buf);//Data we've sent
    
    LoRa_sleep(modem);
}

void * rx_f(void *p){
    rxData *rx = (rxData *)p;
    LoRa_ctl *modem = (LoRa_ctl *)(rx->userPtr);
    LoRa_stop_receive(modem);//manually stoping RxCont mode
    printf("rx done;\t");
    printf("CRC error: %d;\t", rx->CRC);
    printf("Data size: %d;\t", rx->size);
    printf("RSSI: %d;\t", rx->RSSI);
    printf("SNR: %f\n\n", rx->SNR);
    
    printf("Data copied into transfer buffer\n");

    // unsigned char imageBuf[dataSize];
    // printf("A\n");

    // // Copy the data from rx->buf to imageBuf
    // memcpy(imageBuf, rx->buf, rx->size);
    // printf("B\n");

    // imageBufPtr = imageBuf;
    // printf("C\n");

    // dataFlag = 1;
    // dataSize = rx->size;

    // printf("D\n");


    //START IMAGE LOAD TEST

    // int* width2 = malloc(sizeof(int));
    // int* height2 = malloc(sizeof(int));
    // int* channels2 = malloc(sizeof(int));

    // *width2 = 32;
    // *height2 = 24;
    // *channels2 = 1;

    // printf("rx callback test 1\n");
    
    // //img2 = modem.tx.data.buf;//stbi_load(modem.tx.data.buf, width2, height2, channels2, 0);
    // unsigned char *img2 = stbi_load("/home/ubagley18/Documents/projects/LoRaImageReceive/src/testSplitReceive/image_0006.jpg", width2, height2, channels2, 0);
    // // Print each byte of the buffer as a hexadecimal value
    // for (int i = 0; i < 163; i++) {
    //     printf("%02x ", img2[i]);
    // }

    // printf("\n\n");

    // printf("Image loaded from buffer\n");

    // if(img2 == NULL) {
    //     printf("Error in loading the image\n");
    //     exit(1);
    // }

    // printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", *width2, *height2, *channels2);

    // stbi_write_jpg("/home/ubagley18/Documents/projects/LoRaImageReceive/src/testSplitReceive/copy.jpg", *width2, *height2, *channels2, img2, 100);


    // END IMAGE LOAD TEST

    printf("rx callback test 1\n");

    unsigned char imageBuf[rx->size];

    // Copy the data from rx->buf to imageBuf
    memcpy(imageBuf, rx->buf, rx->size);

    printf("Data copied into imageBuf\n");

    // Print each byte of the buffer as a hexadecimal value and assign to pointer
    for (int j = 0; j < rx->size; j++) {
        printf("%02x ", imageBuf[j]);
        
    }
    printf("\n");

    // // Create a FILE * from memory data using fmemopen
    // FILE *memoryFile = fmemopen(imageBuf, rx->size, "r");

    // unsigned char *img = stbi_load_from_file(memoryFile, width, height, channels, 0);
    //unsigned char *img = stbi_load_from_memory(imageBuf, rx->size, width, height, channels, 0);
    

    
    
    sprintf(fileName, "/home/ubagley18/Documents/projects/LoRaImageReceive/src/testSplitReceive/image_%04d.jpg", fileIndex);
    printf("%s\n", fileName);
    fileIndex++;

    
    writeFileToPath(imageBuf, fileName, rx->size);


    printf("Image loaded from buffer\n");
    int nbOfFiles;
    nbOfFiles = nbOfFilesInDirectory();
    *nbOfFilesReceived  = nbOfFiles;
    memcpy(modem->tx.data.buf, nbOfFilesReceived, sizeof(*nbOfFilesReceived));//copy data we'll sent to buffer
    modem->tx.data.size = sizeof(modem->tx.data.buf);//Payload len

    //stbi_write_jpg(fileName, *width, *height, *channels, img, 100);
    modem->tx.data.size = sizeof(*nbOfFilesReceived);//Payload len. 0 for unlimited
    LoRa_send(modem);
    printf("Time on air data - Tsym: %f;\t", modem->tx.data.Tsym);
    printf("Tpkt: %f;\t", modem->tx.data.Tpkt);
    printf("payloadSymbNb: %u\n", modem->tx.data.payloadSymbNb);

    //Don't forget to free the dynamically allocated memory when you're done with it
    free(p);
    //stbi_image_free(img);
    // Image_free(image);
    // free(width);
    // free(height);
    // free(channels);

    return NULL;
}



int main(){

    char txbuf[2000];
    char rxbuf[2000];
    imageBufPtr = malloc(255);
    image = malloc (1000);
    LoRa_ctl modem;

    fileIndex = 0;
    // Allocate memory for nbOfFilesReceived
    nbOfFilesReceived = (int*)malloc(sizeof(int));

    //See for typedefs, enumerations and there values in LoRa.h header file
    modem.spiCS = 0;//Raspberry SPI CE pin number
    modem.tx.callback = tx_f;
    modem.tx.data.buf = txbuf;
    modem.rx.callback = rx_f;
    modem.rx.data.userPtr = (void *)(&modem);//To handle with chip from rx callback
    modem.tx.data.userPtr = (void *)(&modem);//To handle with chip from tx callback
    modem.eth.preambleLen=6;
    modem.eth.bw = BW250;//Bankdwidth 250kHz//BW62_5;//Bandwidth 62.5KHz
    modem.eth.sf = SF12;//Spreading Factor 7//SF12;//Spreading Factor 12
    modem.eth.ecr = CR5;//Error coding rate CR4/8
    modem.eth.CRC = 0;//Turn off CRC checking
    modem.eth.freq = 434800000;// 434.8MHz
    modem.eth.resetGpioN = 4;//GPIO4 on lora RESET pin
    modem.eth.dio0GpioN = 17;//GPIO17 on lora DIO0 pin to control Rxdone and Txdone interrupts
    modem.eth.outPower = OP20;//Output power
    modem.eth.powerOutPin = PA_BOOST;//Power Amplifire pin
    modem.eth.AGC = 1;//Auto Gain Control
    modem.eth.OCP = 240;// 45 to 240 mA. 0 to turn off protection
    modem.eth.implicitHeader = 0;//1;//Implicit header mode//0;//Explicit header mode
    modem.eth.syncWord = 0x12;
    //For detail information about SF, Error Coding Rate, Explicit header, Bandwidth, AGC, Over current protection and other features refer to sx127x datasheet https://www.semtech.com/uploads/documents/DS_SX1276-7-8-9_W_APP_V5.pdf

    printf("Start LoRa receive\n");

    while (1)
    {
        LoRa_begin(&modem);
        LoRa_receive(&modem);
        
        while(LoRa_get_op_mode(&modem) != SLEEP_MODE)
        {
            sleep(1);
        }

        LoRa_end(&modem);

    }
}

int nbOfFilesInDirectory(void)
{
    int file_count = 0;
    DIR * dirp;
    struct dirent * entry;

    printf("Checking directory\n");

    dirp = opendir("/home/ubagley18/Documents/projects/LoRaImageReceive/src/testSplitReceive"); /* There should be error handling after this */
    if (dirp == NULL)
    {
        printf("Error opening directory");
    }
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_REG) { /* If the entry is a regular file */
            file_count++;
        }
    }
    closedir(dirp);

    return file_count;
}

void writeFileToPath(unsigned char* jpgBuffer, const char* newFilePath, int fileSize)
{
    FILE* receivedJpgFile = fopen(newFilePath, "wb");
    if (receivedJpgFile == NULL) {
        // Handle file creation error.
        printf("Received JPG is NULL");
        return;
    }

    fwrite(jpgBuffer, 1, fileSize, receivedJpgFile);
    fclose(receivedJpgFile);
}


    // FLAG ATTEMPT IN MAIN

    // if (dataFlag == 1)
        // {
        //     int* width = malloc(sizeof(int));
        //     int* height = malloc(sizeof(int));
        //     int* channels = malloc(sizeof(int));

        //     *width = 32;
        //     *height = 24;
        //     *channels = 1;

        //     printf("rx callback test 1\n");

        //     // unsigned char imageBuf[dataSize];

        //     // // Copy the data from rx->buf to imageBuf
        //     // memcpy(imageBuf, rx->buf, rx->size);

        //     printf("Data copied into imageBuf\n");

        //     // Print each byte of the buffer as a hexadecimal value and assign to pointer
        //     for (int j = 0; j < dataSize; j++) {
        //         printf("%02x ", imageBufPtr[j]);
                
        //     }
        //     printf("\n");

        //     // // Create a FILE * from memory data using fmemopen
        //     // FILE *memoryFile = fmemopen(imageBuf, rx->size, "r");

        //     // unsigned char *img = stbi_load_from_file(memoryFile, width, height, channels, 0);
        //     //unsigned char *img = stbi_load_from_memory(imageBuf, rx->size, width, height, channels, 0);
            

            
            
        //     sprintf(fileName, "/home/ubagley18/Documents/projects/LoRaImageReceive/src/testSplitReceive/image_%04d.jpg", fileIndex);
        //     printf("%s\n", fileName);
        //     fileIndex++;

        //     printf("Trying to create image from binary data\n");

        //     Image_create(image, *width, *height, *channels, 0); //Create an "Image" structure

        //     printf("rx callback test 2\n");

        //     for (int k = 0; k < dataSize; k++) 
        //     {
        //         image->data[k] = imageBufPtr[k];
        //         printf("Strike %d ", k);
        //     }

        //     // if (memoryFile == NULL) {
        //     //     perror("fmemopen error");
        //     // }

        //     printf("\n\n");
        //     printf("Created image from binary data\n");
            
        //     //img = imageBuf; //rx->buf;//stbi_load(rx->buf, width, height, channels, 0);

        //     printf("Image loaded from buffer\n");

        //     // if(img == NULL) {
        //     //     printf("Error in loading the image\n");
        //     //     exit(1);
        //     // }
        //     printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", *width, *height, *channels);

        //     Image_save(image, fileName); //Save this structure to a file and write jpg

        //     printf("Saved image into jpg file");
        //     Image_free(image);
        //     free(width);
        //     free(height);
        //     free(channels);

        //     //Reset data flag
        //     dataFlag = 0;
        // }

        // unsigned char imageBuf[dataSize];




    
    // START RX CALLBACK ROUTINE FOR PIXEL DATA
    // printf("A\n");

    // // Copy the data from rx->buf to imageBuf
    // memcpy(imageBuf, rx->buf, rx->size);
    // printf("B\n");

    // imageBufPtr = imageBuf;
    // printf("C\n");

    // dataFlag = 1;
    // dataSize = rx->size;

    // printf("D\n");


    //START IMAGE LOAD TEST

    // int* width2 = malloc(sizeof(int));
    // int* height2 = malloc(sizeof(int));
    // int* channels2 = malloc(sizeof(int));

    // *width2 = 32;
    // *height2 = 24;
    // *channels2 = 1;

    // printf("rx callback test 1\n");
    
    // //img2 = modem.tx.data.buf;//stbi_load(modem.tx.data.buf, width2, height2, channels2, 0);
    // unsigned char *img2 = stbi_load("/home/ubagley18/Documents/projects/LoRaImageReceive/src/testSplitReceive/image_0006.jpg", width2, height2, channels2, 0);
    // // Print each byte of the buffer as a hexadecimal value
    // for (int i = 0; i < 163; i++) {
    //     printf("%02x ", img2[i]);
    // }

    // printf("\n\n");

    // printf("Image loaded from buffer\n");

    // if(img2 == NULL) {
    //     printf("Error in loading the image\n");
    //     exit(1);
    // }

    // printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", *width2, *height2, *channels2);

    // stbi_write_jpg("/home/ubagley18/Documents/projects/LoRaImageReceive/src/testSplitReceive/copy.jpg", *width2, *height2, *channels2, img2, 100);


    // END IMAGE LOAD TEST

     // END RX CALLBACK ROUTINE FOR PIXEL DATA

    // MORE OF THAT ^
    // int* width = malloc(sizeof(int));
    // int* height = malloc(sizeof(int));
    // int* channels = malloc(sizeof(int));

    // *width = 32;
    // *height = 24;
    // *channels = 1;

    // printf("rx callback test 1\n");

    // unsigned char imageBuf[rx->size];

    // // Copy the data from rx->buf to imageBuf
    // memcpy(imageBuf, rx->buf, rx->size);

    // printf("Data copied into imageBuf\n");

    // // Print each byte of the buffer as a hexadecimal value and assign to pointer
    // for (int j = 0; j < rx->size; j++) {
    //     printf("%02x ", imageBuf[j]);
        
    // }
    // printf("\n");

    // // // Create a FILE * from memory data using fmemopen
    // // FILE *memoryFile = fmemopen(imageBuf, rx->size, "r");

    // // unsigned char *img = stbi_load_from_file(memoryFile, width, height, channels, 0);
    // //unsigned char *img = stbi_load_from_memory(imageBuf, rx->size, width, height, channels, 0);
    

    
    
    // sprintf(fileName, "/home/ubagley18/Documents/projects/LoRaImageReceive/src/testSplitReceive/image_%04d.jpg", fileIndex);
    // printf("%s\n", fileName);
    // fileIndex++;

    // printf("Trying to create image from binary data\n");

    // Image_create(image, *width, *height, *channels, 0); //Create an "Image" structure

    // printf("rx callback test 2\n");

    // for (int k = 0; k < rx->size; k++) 
    // {
    //     image->data[k] = imageBuf[k];
    // }

    // // if (memoryFile == NULL) {
    // //     perror("fmemopen error");
    // // }

    // printf("\n\n");
    // printf("Created image from binary data\n");
    
    // //img = imageBuf; //rx->buf;//stbi_load(rx->buf, width, height, channels, 0);

    // printf("Image loaded from buffer\n");

    // modem->tx.data.size = 15;//Payload len. 0 for unlimited

    // // if(img == NULL) {
    // //     printf("Error in loading the image\n");
    // //     exit(1);
    // // }
    // printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", *width, *height, *channels);

    // Image_save(image, fileName); //Save this structure to a file and write jpg