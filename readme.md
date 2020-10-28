# OLED Video Player

Play a video on a 128*64 OLED screen.

## 1. Convert the video data

Using ```Video_converter.py``` can convert the video into the binary bitmap which can be played on the OLED display. In this case, we using 128 * 64 OLED display. It has a built-in 1KB GDDRAM. Which means the size of one frame is 1KB. Here is a very obvious picture provided by _lastminuteengineers.com_ can show how the pixels information arranged in the memory.
<p align="center">
  <img src="https://lastminuteengineers.com/wp-content/uploads/arduino/1KB-128x64-OLED-Display-RAM-Memory-Map.png">
</p>
The Video_converter.py can read the video and convert it to black and white display. Then save a binary file for the converted video. And it is ready to load to the OLED screen. During the conversion, a preview window will provided.
<p align="center">
  <img src="/pics/1.JPG">
</p>

After converting the video into binary bitmap, launch console in the working directory and using the following code to write it into SD card, so it can be read by MCU later:

```sudo dd if=./binfile.bin of=/dev/sde```

Because this is a low level operation (I/O access on media), ```sudo``` is required.

## 2. Read one frame of the video from SD card

The core of this player is an ATmeage328P.

The SD card has two interface, SDC and SPI. SDC is a fast interface, but requires special hardware, the specification of SDC interface is not public. Another way to access the SD card is by using SPI interface. In this prpject, the SPI interface of the AVR MCU is used to connect with the SD card.

TODO: More detail

## 3. Send the bitmap to OLED screen

After reading one frame from the SD card into MCU's memory, the MCU will send it to the display device. In this case, I2C bus is used.

TODO: More detail
