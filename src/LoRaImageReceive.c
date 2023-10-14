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

#include <stdio.h>
#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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
    printf("received string: \"%02x\";\t", *rx->buf);//Data we've received
    printf("RSSI: %d;\t", rx->RSSI);
    printf("SNR: %f\n\n", rx->SNR);

    memcpy(modem->tx.data.buf, "Image received", 15);//copy data we'll sent to buffer
    modem->tx.data.size = sizeof(modem->tx.data.buf);//Payload len
    
    printf("Received data copied into transfer buffer\n");

    int* width = malloc(sizeof(int));
    int* height = malloc(sizeof(int));
    int* channels = malloc(sizeof(int));

    *width = 4;
    *height = 3;
    *channels = 1;

    

    unsigned char *img;

    printf("rx callback test 1\n");
    
    img = rx->buf;//stbi_load(rx->buf, width, height, channels, 0);

    printf("Image loaded from buffer\n");

    modem->tx.data.size = 15;//Payload len. 0 for unlimited

    if(img == NULL) {
        printf("Error in loading the image\n");
        exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", *width, *height, *channels);

    stbi_write_jpg("/home/ubagley18/Documents/projects/LoRaImageReceive/src/copy3.jpg", *width, *height, *channels, img, 100);
    
    LoRa_send(modem);
    printf("Time on air data - Tsym: %f;\t", modem->tx.data.Tsym);
    printf("Tpkt: %f;\t", modem->tx.data.Tpkt);
    printf("payloadSymbNb: %u\n", modem->tx.data.payloadSymbNb);

    // Don't forget to free the dynamically allocated memory when you're done with it
    free(p);
    stbi_image_free(img);
    free(width);
    free(height);
    free(channels);

    return NULL;
}



int main(){

    char txbuf[2000];
    char rxbuf[2000];
    LoRa_ctl modem;

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

    LoRa_begin(&modem);
    LoRa_receive(&modem);
    
    while(LoRa_get_op_mode(&modem) != SLEEP_MODE){
        sleep(1);
    }

    printf("end\n");

    LoRa_end(&modem);
}