# video-player-stm32-HUB75
video player stm32 HUB75

on first two board and then changed to on board on



https://www.youtube.com/watch?v=0SOvpV5kFw0&list=PLQ42hqhrp2pTx7ryOxfo36ZguzxlAPJ-I&index=15&t=17s

![video player stm32 HUB75](https://github.com/offpic/video-player-stm32-HUB75/assets/31142397/7bebc2f9-d98f-496b-8c8d-a0bc650d6796)

 made an LED display with STM32F730R8T6 and four 128x64 pixel P2 dot matrix LEDs.
Read AVI file from micro SD card.
Video and audio can be played simultaneously.

LED display specifications
Resolution: 256x128 pixels
PWM resolution: 1 to 12 bits per color
Refresh rate: 63RPS (when PWM resolution is 12 bits and brightness is 50%), 85RPS (when PWM resolution is 8 bits and brightness is 50%)
Real-time gamma correction: 0.6 to 5.0

The video buffer is a double buffer. However, tearing is terrible because it is not synchronized with the camera refresh rate.

Video format
Video
Format: RGB24
Codec Info: Basic Windows bitmap format.
Width: 256 pixels
Height: 128 pixels
Frame rate: 15.000 to 30.000 FPS
Audio
Format: PCM
Format settings, Endianness: Little
Format settings, Sign: Signed
Bit depth: 16 bits
Channel (s): 1 to 2 channels
Sampling rate: 44.1 to 48 kHz
Alignment: Aligned on interleaves

![image](https://github.com/offpic/video-player-stm32-HUB75/assets/31142397/1bbce720-93c2-4492-942e-6fd18030eb32)

![image](https://github.com/offpic/video-player-stm32-HUB75/assets/31142397/8ea1fdd7-9aed-44c6-b29b-c07eae3b66d0)

![image](https://github.com/offpic/video-player-stm32-HUB75/assets/31142397/020b9c3c-9a29-473d-917e-bd681e39243c)

![image](https://github.com/offpic/video-player-stm32-HUB75/assets/31142397/4460c199-df39-4f30-bc9d-7bd189269a08)





