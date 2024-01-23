# video-player-stm32-HUB75
video player stm32 HUB75

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

![02b9d3331](https://github.com/offpic/video-player-stm32-HUB75/assets/31142397/a3ef8e5a-8716-4110-afc1-be092a02c78c)

![30f5298b1](https://github.com/offpic/video-player-stm32-HUB75/assets/31142397/7e3a94b6-5076-40fa-8e30-24d237aa729b)

