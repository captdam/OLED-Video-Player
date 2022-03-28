# OLED Video Player

![Mr.Banana 1](/pics/banana1.gif "Banana 1") | ![Mr.Banana 2](/pics/banana2.gif "Banana 1")
--- | ---
BANANA 1 | BANANA 2

Play a video on a 128*64 OLED screen.

# Convert the video data on computer

We want to play the video on MCU connected with OLED screen. MCU does not have the power to decode regular video file such as mp4; therefore, we need to convert the video into plain bitmap stream before we can play it on MCU.

Using ```Video_converter.py``` can convert the video into the binary bitmap stream which can be played on the OLED display. In this case, we using 128 * 64 OLED display. It has a built-in 1KB GDDRAM. Which means the size of one frame is 1KB. Here is a very obvious picture provided by _lastminuteengineers.com_ can show how the pixels information arranged in the memory.

![OLED memory layout](https://lastminuteengineers.com/wp-content/uploads/arduino/1KB-128x64-OLED-Display-RAM-Memory-Map.png "OLED memory layout")

The Video_converter.py can read the video and convert it to black and white display. Then save a binary file for the converted video. And it is ready to load to the OLED screen. During the conversion, a preview window will provided.

![Bitmap](/pics/1.JPG "Bitmap")

After converting the video into binary bitmap, launch console in the working directory and using the following code to write it into SD card, so it can be read by MCU later:

```sudo dd if=./binfile.bin of=/dev/sde```

Because this is a low level operation (I/O access on media), ```sudo``` is required.

# Play the video on MCU

The core of this player is an ATmeage328P. Entry point is ```/lcd.c```.

Hardware configuration is shown in the comment. The hardware must be connected in this way, to utilize all the hardware perf required.

## Read one frame of the video from SD card

The SD card has two interface, SDC and SPI. SDC is a fast interface, but requires special hardware, the specification of SDC interface is not public. Another way to access the SD card is by using SPI interface. In this prpject, the SPI interface of the AVR MCU is used to connect with the SD card.

See code in ```/sd.h```

First step, init the SD card, using routine ```uint8_t initSDHC()```. This routine will config the SD card for us, such as mode, block size...

After init, we can begin to fetch data from the SD card. We can use routine ```uint8_t readBlock(uint32_t sector, uint8_t buffer[])```. This routine will fetch a 512-byte block of data from SD card sector ```sector```, and write the data to address pointed by ```buffer```. The application should init the buffer, making sure there is enough memory space to hold the block data.

## Send the bitmap to OLED screen

After reading one frame from the SD card into MCU's memory, the MCU will send it to the display device.

To send data to the OLED, I2C bus is used. Common I2C function is in ```/i2c.h```.

Before we can strat playing the video on LCD, we need to init it as well. See code in ```/lcd.c``` in routine ```int main()```

After this, we can begin our main loop.

For every iteration, we grap two block of data from SD card (one block is 512 bytes, one video frame is 1Ki bytes) via SPI, the send them to OLED via I2C. See code in ```/lcd.c``` in routine ```int main()``` for detail.



![Mr.Banana 1](/pics/banana1.gif "Banana 1") | ![Mr.Banana 1](/pics/banana1.gif "Banana 1") | ![Mr.Banana 1](/pics/banana1.gif "Banana 1")
--- | --- | ---
![Mr.Banana 1](/pics/banana1.gif "Banana 1") | ![Mr.Banana 1](/pics/banana1.gif "Banana 1") | ![Mr.Banana 1](/pics/banana1.gif "Banana 1")
![Mr.Banana 1](/pics/banana1.gif "Banana 1") | ![Mr.Banana 1](/pics/banana1.gif "Banana 1") | ![Mr.Banana 1](/pics/banana1.gif "Banana 1")
![Mr.Banana 1](/pics/banana1.gif "Banana 1") | ![Mr.Banana 1](/pics/banana1.gif "Banana 1") | ![Mr.Banana 1](/pics/banana1.gif "Banana 1")
![Mr.Banana 1](/pics/banana1.gif "Banana 1") | ![Mr.Banana 1](/pics/banana1.gif "Banana 1") | ![Mr.Banana 1](/pics/banana1.gif "Banana 1")
