# OLED Video Player

Play a video on a 128*64 OLED screen.
### 1. Convert the video data

Using Video_converter.py can convert the video into the binary data which can be played on the OLED display. In this case, we using 128 * 64 OLED display. It has a built-in 1KB GDDRAM. Which means the size of one frame is 1KB. Here is a very obvious picture provided by lastminuteengineers.com can show how the pixels information arranged in the memory.
<p align="center">
  <img src="https://lastminuteengineers.com/wp-content/uploads/arduino/1KB-128x64-OLED-Display-RAM-Memory-Map.png">
</p>
The Video_converter.py can read the video and convert it to black and white display. Then save a binary file for the converted video. And it is ready to load to the OLED screen.
