#!/bin/bash

#Start LoRaReceive
sudo ../build/./LoRaImageReceive

#Wait for ack

dir="sharedFolder"
x=0
while [ $x -eq 0 ]; do
    for f in "$dir"/*;do
        if [ $f = "sharedFolder/ack" ]
        then 
            rm sharedFolder/ack
            #rm sharedFolder/gc.jpg
            x=1
            break
        fi
    done
done

echo "Ack received"

#Now put together image
montage -mode concatenate -tile 32x skyChoppedBlur_*.jpg skyTiled.jpg