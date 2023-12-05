/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
  FILE_OK = 0,
  FATFS_ERROR = 1,
  FILE_ERROR = 2,
  OTHER_ERROR = 10
} READ_FILE_RESULT;

#define BPP24 24
#define BI_RGB 0x00000000
#define BMP_FILE_TYPE 0x4D42    /**< "BM"をリトルエンディアンで解釈した値 */
#define BMP_FILE_HEADER_SIZE 14 /**< BMPファイルヘッダサイズ */
#define BMP_INFO_HEADER_SIZE 40 /**< Windowsヘッダサイズ */
#define BMP_DEFAULT_HEADER_SIZE (BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE)
 /**< 標準のヘッダサイズ */

#pragma pack(2)
typedef struct BITMAPFILEHEADER {
  uint16_t bfType;      /**< ファイルタイプ、必ず"BM" */
  uint32_t bfSize;      /**< ファイルサイズ */
  uint16_t bfReserved1; /**< リザーブ */
  uint16_t bfReserved2; /**< リサーブ */
  uint32_t bfOffBits;   /**< 先頭から画像情報までのオフセット */
} BITMAPFILEHEADER;
#pragma pack()
typedef struct BITMAPINFOHEADER {
  uint32_t biSize;         /**< この構造体のサイズ */
  int32_t biWidth;         /**< 画像の幅 */
  int32_t biHeight;        /**< 画像の高さ */
  uint16_t biPlanes;       /**< 画像の枚数、通常1 */
  uint16_t biBitCount;     /**< 一色のビット数 */
  uint32_t biCompression;  /**< 圧縮形式 */
  uint32_t biSizeImage;    /**< 画像領域のサイズ */
  int32_t biXPelsPerMeter; /**< 画像の横方向解像度情報 */
  int32_t biYPelsPerMeter; /**< 画像の縦方向解像度情報*/
  uint32_t biClrUsed;      /**< カラーパレットのうち実際に使っている色の個数 */
  uint32_t biClrImportant; /**< カラーパレットのうち重要な色の数 */
} BITMAPINFOHEADER;


#define CHUNKHEADER_SIZE 8
#define RIFF_CHUNK_SIZE (4 + CHUNKHEADER_SIZE)
#define WAVEFORMATEX_CHUNK_SIZE (18 + CHUNKHEADER_SIZE)

typedef struct CHUNKHEADER {
  uint8_t chunkID[4];
  uint32_t chunkSize;
} CHUNKHEADER;

typedef struct RIFF {
  CHUNKHEADER header;
  uint8_t format[4];
} RIFF;

#define WAVE_FORMAT_PCM 0x0001

#pragma pack(2)
typedef struct WAVEFORMATEX {
  uint16_t wFormatTag;
  uint16_t nChannels;
  uint32_t nSamplesPerSec;
  uint32_t nAvgBytesPerSec;
  uint16_t nBlockAlign;
  uint16_t wBitsPerSample;
  uint16_t cbSize;
} WAVEFORMATEX;
#pragma pack()






#define FCC(ch4) ((((uint32_t)(ch4) & 0xFF) << 24) |     \
                  (((uint32_t)(ch4) & 0xFF00) << 8) |    \
                  (((uint32_t)(ch4) & 0xFF0000) >> 8) |  \
                  (((uint32_t)(ch4) & 0xFF000000) >> 24))
/*
typedef struct _fourcc {
  uint32_t fcc;
} FOURCC;
*/
typedef struct _fourcc {
  uint8_t fcc[4];
} FOURCC;

typedef struct _riffchunk {
  FOURCC fcc;
  uint32_t  cb;
} RIFFCHUNK, * LPRIFFCHUNK;
typedef struct _rifflist {
  FOURCC fcc;
  uint32_t  cb;
  FOURCC fccListType;
} RIFFLIST, * LPRIFFLIST;


#define RIFFROUND(cb) ((cb) + ((cb)&1))
#define RIFFNEXT(pChunk) (LPRIFFCHUNK)((uint8_t *)(pChunk) + sizeof(RIFFCHUNK) + RIFFROUND(((LPRIFFCHUNK)pChunk)->cb))



//
// ==================== avi header structures ===========================
//

// main header for the avi file (compatibility header)
//
#define ckidMAINAVIHEADER FCC('avih')
typedef struct _avimainheader {
    FOURCC fcc;                    // 'avih'
    uint32_t  cb;                     // size of this structure -8
    uint32_t  dwMicroSecPerFrame;     // frame display rate (or 0L)
    uint32_t  dwMaxBytesPerSec;       // max. transfer rate
    uint32_t  dwPaddingGranularity;   // pad to multiples of this size; normally 2K.
    uint32_t  dwFlags;                // the ever-present flags
    #define AVIF_HASINDEX        0x00000010 // Index at end of file?
    #define AVIF_MUSTUSEINDEX    0x00000020
    #define AVIF_ISINTERLEAVED   0x00000100
    #define AVIF_TRUSTCKTYPE     0x00000800 // Use CKType to find key frames
    #define AVIF_WASCAPTUREFILE  0x00010000
    #define AVIF_COPYRIGHTED     0x00020000
    uint32_t  dwTotalFrames;          // # frames in first movi list
    uint32_t  dwInitialFrames;
    uint32_t  dwStreams;
    uint32_t  dwSuggestedBufferSize;
    uint32_t  dwWidth;
    uint32_t  dwHeight;
    uint32_t  dwReserved[4];
    } AVIMAINHEADER;

#define ckidODML          FCC('odml')
#define ckidAVIEXTHEADER  FCC('dmlh')
typedef struct _aviextheader {
   FOURCC  fcc;                    // 'dmlh'
   uint32_t   cb;                     // size of this structure -8
   uint32_t   dwGrandFrames;          // total number of frames in the file
   uint32_t   dwFuture[61];           // to be defined later
   } AVIEXTHEADER;

//
// structure of an AVI stream header riff chunk
//
#define ckidSTREAMLIST   FCC('strl')

#ifndef ckidSTREAMHEADER
#define ckidSTREAMHEADER FCC('strh')
#endif
typedef struct _avistreamheader {
   FOURCC fcc;          // 'strh'
   uint32_t  cb;           // size of this structure - 8

   FOURCC fccType;      // stream type codes

   #ifndef streamtypeVIDEO
   #define streamtypeVIDEO FCC('vids')
   #define streamtypeAUDIO FCC('auds')
   #define streamtypeMIDI  FCC('mids')
   #define streamtypeTEXT  FCC('txts')
   #endif

   FOURCC fccHandler;
   uint32_t  dwFlags;
   #define AVISF_DISABLED          0x00000001
   #define AVISF_VIDEO_PALCHANGES  0x00010000

   uint16_t   wPriority;
   uint16_t   wLanguage;
   uint32_t  dwInitialFrames;
   uint32_t  dwScale;
   uint32_t  dwRate;       // dwRate/dwScale is stream tick rate in ticks/sec
   uint32_t  dwStart;
   uint32_t  dwLength;
   uint32_t  dwSuggestedBufferSize;
   uint32_t  dwQuality;
   uint32_t  dwSampleSize;
   struct {
      int16_t left;
      int16_t top;
      int16_t right;
      int16_t bottom;
      }   rcFrame;
   } AVISTREAMHEADER;


//
// structure of an AVI stream format chunk
//
#ifndef ckidSTREAMFORMAT
#define ckidSTREAMFORMAT FCC('strf')
#endif
//
// avi stream formats are different for each stream type
//
// BITMAPINFOHEADER for video streams
// WAVEFORMATEX or PCMWAVEFORMAT for audio streams
// nothing for text streams
// nothing for midi streams

typedef struct _avioldindex_entry {
   FOURCC   dwChunkId;
   uint32_t   dwFlags;

     #ifndef AVIIF_LIST
     #define AVIIF_LIST       0x00000001
     #define AVIIF_KEYFRAME   0x00000010
     #endif

     #define AVIIF_NO_TIME    0x00000100
     #define AVIIF_COMPRESSOR 0x0FFF0000  // unused?
   uint32_t   dwOffset;    // offset of riff chunk header for the data
   uint32_t   dwSize;      // size of the data (excluding riff header size)
     } aIndex;          // size of this array

//
// structure of old style AVI index
//
#define ckidAVIOLDINDEX FCC('idx1')
typedef struct _avioldindex {
   FOURCC  fcc;        // 'idx1'
   uint32_t   cb;         // size of this structure -8
   aIndex avoidindex;
   } AVIOLDINDEX;

typedef struct{
  float video_frame_rate;
  uint32_t video_length;
  FOURCC video_data_chunk_name;
  uint16_t audio_channels;
  uint32_t audio_sampling_rate;
  uint32_t audio_length;
  FOURCC audio_data_chunk_name;
  uint32_t avi_streams_count;
  uint32_t movi_list_position;
  uint32_t avi_old_index_position;
  uint32_t avi_old_index_size;
  uint32_t avi_file_size;
} AVI_INFO;

typedef struct{
  uint8_t file_name[_MAX_LFN];
  AVI_INFO avi_info;
}PLAY_INFO;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//TIM6　オーディオサンプルレート設定用
//TIM7　再生FPS測定用
//TIM14 赤外線リモコンパルス幅測定用
#define TIM6_FREQ 108000000
#define TIM6_PRESCALER 0

#define TIM7_FREQ 108000000
#define TIM7_PRESCALER 150

#define TIM14_FREQ 108000000
#define TIM14_PRESCALER 181

#define IR_MODULATION_UNIT_TOLERANCE 50

#define SET_AUDIO_SAMPLERATE(x) __HAL_TIM_SET_AUTORELOAD(&htim6, ((unsigned int)(TIM6_FREQ / ((x) * (TIM6_PRESCALER + 1))) - 1))

#define COLOR_R 0
#define COLOR_G 1
#define COLOR_B 2

#define VERTICAL 0
#define HORIZONTAL 1

#define PLAY 0
#define STOP 1
#define PAUSE 2

#define LOOP_NO 0
#define LOOP_SINGLE 1
#define LOOP_ALL 2

#define LINKMAP_TABLE_SIZE 0x14

#define MATRIXLED_X_COUNT 256
#define MATRIXLED_Y_COUNT 128
#define MATRIXLED_COLOR_COUNT 3
#define MATRIXLED_PWM_RESOLUTION 256

#define SPI_SEND_COMMAND_COUNT 4

#define SPI_DELAY_TIME_0 200
#define SPI_DELAY_TIME_1 1000

#define FONT_DATA_PIXEL_SIZE_X 5
#define FONT_DATA_PIXEL_SIZE_Y 8
#define FONT_DATA_MAX_CHAR 192
#define DISPLAY_TEXT_TIME 1000
#define DISPLAY_TEXT_MAX 45

#define MIN_VIDEO_FLAME_RATE 15
#define MAX_VIDEO_FLAME_RATE 60
#define MIN_AUDIO_SAMPLE_RATE 44099
#define MAX_AUDIO_SAMPLE_RATE 48000
#define MIN_AUDIO_CHANNEL 1
#define MAX_AUDIO_CHANNEL 2

#define IMAGE_READ_DELAY_FLAME 2

#define DISPLAY_FPS_AVERAGE_TIME 0.4

#define MAX_PLAYLIST_COUNT 9
#define MAX_TRACK_COUNT 100

#define SKIP_TIME 5

/*
const unsigned int IR_Remote_SW_Value[7][3] = {
    {0xba45ff00, 0xb946ff00, 0xb847ff00},
    {0xbb44ff00, 0xbf40ff00, 0xbc43ff00},
    {0xf807ff00, 0xea15ff00, 0xf609ff00},
    {0xe916ff00, 0xe619ff00, 0xf20dff00},
    {0xf30cff00, 0xe718ff00, 0xa15eff00},
    {0xf708ff00, 0xe31cff00, 0xa55aff00},
    {0xbd42ff00, 0xad52ff00, 0xb54aff00}
};*/
#define IR_REMOTE_SW_0_0 0xba45ff00
#define IR_REMOTE_SW_0_1 0xb946ff00
#define IR_REMOTE_SW_0_2 0xb847ff00

#define IR_REMOTE_SW_1_0 0xbb44ff00
#define IR_REMOTE_SW_1_1 0xbf40ff00
#define IR_REMOTE_SW_1_2 0xbc43ff00

#define IR_REMOTE_SW_2_0 0xf807ff00
#define IR_REMOTE_SW_2_1 0xea15ff00
#define IR_REMOTE_SW_2_2 0xf609ff00

#define IR_REMOTE_SW_3_0 0xe916ff00
#define IR_REMOTE_SW_3_1 0xe619ff00
#define IR_REMOTE_SW_3_2 0xf20dff00

#define IR_REMOTE_SW_4_0 0xf30cff00
#define IR_REMOTE_SW_4_1 0xe718ff00
#define IR_REMOTE_SW_4_2 0xa15eff00

#define IR_REMOTE_SW_5_0 0xf708ff00
#define IR_REMOTE_SW_5_1 0xe31cff00
#define IR_REMOTE_SW_5_2 0xa55aff00

#define IR_REMOTE_SW_6_0 0xbd42ff00
#define IR_REMOTE_SW_6_1 0xad52ff00
#define IR_REMOTE_SW_6_2 0xb54aff00

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

DAC_HandleTypeDef hdac;

SD_HandleTypeDef hsd1;
DMA_HandleTypeDef hdma_sdmmc1_rx;
DMA_HandleTypeDef hdma_sdmmc1_tx;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART3 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}

extern uint8_t retSD; /* Return value for SD */
extern char SDPath[4]; /* SD logical drive path */
FATFS fs __attribute__((aligned(4)));

const unsigned char Font_Data[FONT_DATA_MAX_CHAR][FONT_DATA_PIXEL_SIZE_X] =
{
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x20 ' '
  { 0x00, 0x00, 0x4F, 0x00, 0x00 }, // 0x21 !
  { 0x00, 0x07, 0x00, 0x07, 0x00 }, // 0x22 "
  { 0x14, 0x7F, 0x14, 0x7F, 0x14 }, // 0x23 #
  { 0x24, 0x2A, 0x7F, 0x2A, 0x12 }, // 0x24 $
  { 0x23, 0x13, 0x08, 0x64, 0x62 }, // 0x25 %
  { 0x36, 0x49, 0x55, 0x22, 0x50 }, // 0x26 &
  { 0x00, 0x05, 0x03, 0x00, 0x00 }, // 0x27 '
  { 0x00, 0x1C, 0x22, 0x41, 0x00 }, // 0x28 (
  { 0x00, 0x41, 0x22, 0x1C, 0x00 }, // 0x29 )
  { 0x14, 0x08, 0x3E, 0x08, 0x14 }, // 0x2A *
  { 0x08, 0x08, 0x3E, 0x08, 0x08 }, // 0x2B +
  { 0x00, 0x50, 0x30, 0x00, 0x00 }, // 0x2C ,
  { 0x08, 0x08, 0x08, 0x08, 0x08 }, // 0x2D -
  { 0x00, 0x60, 0x60, 0x00, 0x00 }, // 0x2E .
  { 0x20, 0x10, 0x08, 0x04, 0x02 }, // 0x2F /
  { 0x3E, 0x51, 0x49, 0x45, 0x3E }, // 0x30 0
  { 0x00, 0x42, 0x7F, 0x40, 0x00 }, // 0x31 1
  { 0x42, 0x61, 0x51, 0x49, 0x46 }, // 0x32 2
  { 0x21, 0x41, 0x45, 0x4B, 0x31 }, // 0x33 3
  { 0x18, 0x14, 0x12, 0x7F, 0x10 }, // 0x34 4
  { 0x27, 0x45, 0x45, 0x45, 0x39 }, // 0x35 5
  { 0x3C, 0x4A, 0x49, 0x49, 0x30 }, // 0x36 6
  { 0x03, 0x01, 0x71, 0x09, 0x07 }, // 0x37 7
  { 0x36, 0x49, 0x49, 0x49, 0x36 }, // 0x38 8
  { 0x06, 0x49, 0x49, 0x29, 0x1E }, // 0x39 9
  { 0x00, 0x36, 0x36, 0x00, 0x00 }, // 0x3A :
  { 0x00, 0x56, 0x36, 0x00, 0x00 }, // 0x3B ;
  { 0x08, 0x14, 0x22, 0x41, 0x00 }, // 0x3C <
  { 0x14, 0x14, 0x14, 0x14, 0x14 }, // 0x3D =
  { 0x00, 0x41, 0x22, 0x14, 0x08 }, // 0x3E >
  { 0x02, 0x01, 0x51, 0x09, 0x06 }, // 0x3F ?
  { 0x32, 0x49, 0x79, 0x41, 0x3E }, // 0x40 @
  { 0x7E, 0x11, 0x11, 0x11, 0x7E }, // 0x41 A
  { 0x7F, 0x49, 0x49, 0x49, 0x36 }, // 0x42 B
  { 0x3E, 0x41, 0x41, 0x41, 0x22 }, // 0x43 C
  { 0x7F, 0x41, 0x41, 0x22, 0x1C }, // 0x44 D
  { 0x7F, 0x49, 0x49, 0x49, 0x41 }, // 0x45 E
  { 0x7F, 0x09, 0x09, 0x09, 0x01 }, // 0x46 F
  { 0x3E, 0x41, 0x49, 0x49, 0x7A }, // 0x47 G
  { 0x7F, 0x08, 0x08, 0x08, 0x7F }, // 0x48 H
  { 0x00, 0x41, 0x7F, 0x41, 0x00 }, // 0x49 I
  { 0x20, 0x40, 0x41, 0x3F, 0x01 }, // 0x4A J
  { 0x7F, 0x08, 0x14, 0x22, 0x41 }, // 0x4B K
  { 0x7F, 0x40, 0x40, 0x40, 0x40 }, // 0x4C L
  { 0x7F, 0x02, 0x0C, 0x02, 0x7F }, // 0x4D M
  { 0x7F, 0x04, 0x08, 0x10, 0x7F }, // 0x4E N
  { 0x3E, 0x41, 0x41, 0x41, 0x3E }, // 0x4F O
  { 0x7F, 0x09, 0x09, 0x09, 0x06 }, // 0x50 P
  { 0x3E, 0x41, 0x51, 0x21, 0x5E }, // 0x51 Q
  { 0x7F, 0x09, 0x19, 0x29, 0x46 }, // 0x52 R
  { 0x46, 0x49, 0x49, 0x49, 0x31 }, // 0x53 S
  { 0x01, 0x01, 0x7F, 0x01, 0x01 }, // 0x54 T
  { 0x3F, 0x40, 0x40, 0x40, 0x3F }, // 0x55 U
  { 0x1F, 0x20, 0x40, 0x20, 0x1F }, // 0x56 V
  { 0x3F, 0x40, 0x38, 0x40, 0x3F }, // 0x57 W
  { 0x63, 0x14, 0x08, 0x14, 0x63 }, // 0x58 X
  { 0x07, 0x08, 0x70, 0x08, 0x07 }, // 0x59 Y
  { 0x61, 0x51, 0x49, 0x45, 0x43 }, // 0x5A Z
  { 0x00, 0x7F, 0x41, 0x41, 0x00 }, // 0x5B [
  { 0x15, 0x16, 0x7C, 0x16, 0x15 }, // 0x5C '\'
  { 0x00, 0x41, 0x41, 0x7F, 0x00 }, // 0x5D ]
  { 0x04, 0x02, 0x01, 0x02, 0x04 }, // 0x5E ^
  { 0x40, 0x40, 0x40, 0x40, 0x40 }, // 0x5F _
  { 0x00, 0x01, 0x02, 0x04, 0x00 }, // 0x60 `
  { 0x20, 0x54, 0x54, 0x54, 0x78 }, // 0x61 a
  { 0x7F, 0x48, 0x44, 0x44, 0x38 }, // 0x62 b
  { 0x38, 0x44, 0x44, 0x44, 0x20 }, // 0x63 c
  { 0x38, 0x44, 0x44, 0x48, 0x7F }, // 0x64 d
  { 0x38, 0x54, 0x54, 0x54, 0x18 }, // 0x65 e
  { 0x08, 0x7E, 0x09, 0x01, 0x02 }, // 0x66 f
  { 0x0C, 0x52, 0x52, 0x52, 0x3E }, // 0x67 g
  { 0x7F, 0x08, 0x04, 0x04, 0x78 }, // 0x68 h
  { 0x00, 0x44, 0x7D, 0x40, 0x00 }, // 0x69 i
  { 0x20, 0x40, 0x44, 0x3D, 0x00 }, // 0x6A j
  { 0x7F, 0x10, 0x28, 0x44, 0x00 }, // 0x6B k
  { 0x00, 0x41, 0x7F, 0x40, 0x00 }, // 0x6C l
  { 0x7C, 0x04, 0x18, 0x04, 0x78 }, // 0x6D m
  { 0x7C, 0x08, 0x04, 0x04, 0x78 }, // 0x6E n
  { 0x38, 0x44, 0x44, 0x44, 0x38 }, // 0x6F o
  { 0x7C, 0x14, 0x14, 0x14, 0x08 }, // 0x70 p
  { 0x08, 0x14, 0x14, 0x18, 0x7C }, // 0x71 q
  { 0x7C, 0x08, 0x04, 0x04, 0x08 }, // 0x72 r
  { 0x48, 0x54, 0x54, 0x54, 0x20 }, // 0x73 s
  { 0x04, 0x3F, 0x44, 0x40, 0x20 }, // 0x74 t
  { 0x3C, 0x40, 0x40, 0x20, 0x7C }, // 0x75 u
  { 0x1C, 0x20, 0x40, 0x20, 0x1C }, // 0x76 v
  { 0x3C, 0x40, 0x38, 0x40, 0x3C }, // 0x77 w
  { 0x44, 0x28, 0x10, 0x28, 0x44 }, // 0x78 x
  { 0x0C, 0x50, 0x50, 0x50, 0x3C }, // 0x79 y
  { 0x44, 0x64, 0x54, 0x4C, 0x44 }, // 0x7A z
  { 0x00, 0x08, 0x36, 0x41, 0x00 }, // 0x7B {
  { 0x00, 0x00, 0x7F, 0x00, 0x00 }, // 0x7C |
  { 0x00, 0x41, 0x36, 0x08, 0x00 }, // 0x7D }
  { 0x08, 0x08, 0x2A, 0x1C, 0x08 }, // 0x7E '->'
  { 0x08, 0x1C, 0x2A, 0x08, 0x08 }, // 0x7F '<-'
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x80
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x81
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x82
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x83
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x84
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x85
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x86
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x87
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x88
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x89
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x8A
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x8B
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x8C
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x8D
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x8E
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x8F
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x90
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x91
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x92
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x93
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x94
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x95
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x96
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x97
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x98
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x99
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x9A
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x9B
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x9C
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x9D
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x9E
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0x9F
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0xA0
  { 0x70, 0x50, 0x70, 0x00, 0x00 }, // 0xA1 ｡
  { 0x00, 0x00, 0x0F, 0x01, 0x01 }, // 0xA2 ｢
  { 0x40, 0x40, 0x78, 0x00, 0x00 }, // 0xA3 ｣
  { 0x10, 0x20, 0x40, 0x00, 0x00 }, // 0xA4 ､
  { 0x00, 0x18, 0x18, 0x00, 0x00 }, // 0xA5 ･
  { 0x0A, 0x0A, 0x4A, 0x2A, 0x1E }, // 0xA6 ｦ
  { 0x04, 0x44, 0x34, 0x14, 0x0C }, // 0xA7 ｧ
  { 0x20, 0x10, 0x78, 0x04, 0x00 }, // 0xA8 ｨ
  { 0x18, 0x08, 0x4C, 0x48, 0x38 }, // 0xA9 ｩ
  { 0x44, 0x44, 0x7C, 0x44, 0x44 }, // 0xAA ｪ
  { 0x48, 0x28, 0x18, 0x7C, 0x08 }, // 0xAB ｫ
  { 0x08, 0x7C, 0x08, 0x28, 0x18 }, // 0xAC ｬ
  { 0x40, 0x48, 0x48, 0x78, 0x40 }, // 0xAD ｭ
  { 0x54, 0x54, 0x54, 0x7C, 0x00 }, // 0xAE ｮ
  { 0x18, 0x00, 0x58, 0x40, 0x38 }, // 0xAF ｯ
  { 0x08, 0x08, 0x08, 0x08, 0x08 }, // 0xB0 ｰ
  { 0x01, 0x41, 0x3D, 0x09, 0x07 }, // 0xB1 ｱ
  { 0x10, 0x08, 0x7C, 0x02, 0x01 }, // 0xB2 ｲ
  { 0x0E, 0x02, 0x43, 0x22, 0x1E }, // 0xB3 ｳ
  { 0x42, 0x42, 0x7E, 0x42, 0x42 }, // 0xB4 ｴ
  { 0x22, 0x12, 0x0A, 0x7F, 0x02 }, // 0xB5 ｵ
  { 0x42, 0x3F, 0x02, 0x42, 0x3E }, // 0xB6 ｶ
  { 0x0A, 0x0A, 0x7F, 0x0A, 0x0A }, // 0xB7 ｷ
  { 0x08, 0x46, 0x42, 0x22, 0x1E }, // 0xB8 ｸ
  { 0x04, 0x03, 0x42, 0x3E, 0x02 }, // 0xB9 ｹ
  { 0x42, 0x42, 0x42, 0x42, 0x7E }, // 0xBA ｺ
  { 0x02, 0x4F, 0x22, 0x1F, 0x02 }, // 0xBB ｻ
  { 0x4A, 0x4A, 0x40, 0x20, 0x1C }, // 0xBC ｼ
  { 0x42, 0x22, 0x12, 0x2A, 0x46 }, // 0xBD ｽ
  { 0x02, 0x3F, 0x42, 0x4A, 0x46 }, // 0xBE ｾ
  { 0x06, 0x48, 0x40, 0x20, 0x1E }, // 0xBF ｿ
  { 0x08, 0x46, 0x4A, 0x32, 0x1E }, // 0xC0 ﾀ
  { 0x0A, 0x4A, 0x3E, 0x09, 0x08 }, // 0xC1 ﾁ
  { 0x0E, 0x00, 0x4E, 0x20, 0x1E }, // 0xC2 ﾂ
  { 0x04, 0x45, 0x3D, 0x05, 0x04 }, // 0xC3 ﾃ
  { 0x00, 0x7F, 0x08, 0x10, 0x00 }, // 0xC4 ﾄ
  { 0x44, 0x24, 0x1F, 0x04, 0x04 }, // 0xC5 ﾅ
  { 0x40, 0x42, 0x42, 0x42, 0x40 }, // 0xC6 ﾆ
  { 0x42, 0x2A, 0x12, 0x2A, 0x06 }, // 0xC7 ﾇ
  { 0x22, 0x12, 0x7B, 0x16, 0x22 }, // 0xC8 ﾈ
  { 0x00, 0x40, 0x20, 0x1F, 0x00 }, // 0xC9 ﾉ
  { 0x78, 0x00, 0x02, 0x04, 0x78 }, // 0xCA ﾊ
  { 0x3F, 0x44, 0x44, 0x44, 0x44 }, // 0xCB ﾋ
  { 0x02, 0x42, 0x42, 0x22, 0x1E }, // 0xCC ﾌ
  { 0x04, 0x02, 0x04, 0x08, 0x30 }, // 0xCD ﾍ
  { 0x32, 0x02, 0x7F, 0x02, 0x32 }, // 0xCE ﾎ
  { 0x02, 0x12, 0x22, 0x52, 0x0E }, // 0xCF ﾏ
  { 0x00, 0x2A, 0x2A, 0x2A, 0x40 }, // 0xD0 ﾐ
  { 0x38, 0x24, 0x22, 0x20, 0x70 }, // 0xD1 ﾑ
  { 0x40, 0x28, 0x10, 0x28, 0x06 }, // 0xD2 ﾒ
  { 0x0A, 0x3E, 0x4A, 0x4A, 0x4A }, // 0xD3 ﾓ
  { 0x04, 0x7F, 0x04, 0x14, 0x0C }, // 0xD4 ﾔ
  { 0x40, 0x42, 0x42, 0x7E, 0x40 }, // 0xD5 ﾕ
  { 0x4A, 0x4A, 0x4A, 0x4A, 0x7E }, // 0xD6 ﾖ
  { 0x04, 0x05, 0x45, 0x25, 0x1C }, // 0xD7 ﾗ
  { 0x0F, 0x40, 0x20, 0x1F, 0x00 }, // 0xD8 ﾘ
  { 0x7C, 0x00, 0x7E, 0x40, 0x30 }, // 0xD9 ﾙ
  { 0x7E, 0x40, 0x20, 0x10, 0x08 }, // 0xDA ﾚ
  { 0x7E, 0x42, 0x42, 0x42, 0x7E }, // 0xDB ﾛ
  { 0x0E, 0x02, 0x42, 0x22, 0x1E }, // 0xDC ﾜ
  { 0x42, 0x42, 0x40, 0x20, 0x18 }, // 0xDD ﾝ
  { 0x02, 0x04, 0x01, 0x02, 0x00 }, // 0xDE ﾞ
  { 0x07, 0x05, 0x07, 0x00, 0x00 }  // 0xDF ﾟ
};

//unsigned char LEDdisplay_Command[SPI_SEND_COMMAND_COUNT];
unsigned char Flame_Buffer[MATRIXLED_Y_COUNT][MATRIXLED_X_COUNT][MATRIXLED_COLOR_COUNT];

signed short Audio_Buffer[2][MAX_AUDIO_SAMPLE_RATE * MAX_AUDIO_CHANNEL / MIN_VIDEO_FLAME_RATE] = {0};

unsigned int Audio_Flame_Data_Count;
unsigned char Audio_Double_Buffer;
unsigned char Audio_Channnel_Count;

volatile unsigned char Video_End_Flag = RESET;
volatile unsigned char Audio_End_Flag = RESET;

volatile unsigned char Audio_Flame_End_flag = RESET;

unsigned char SPI_DMA_Send_Data_Count = 0;

unsigned char Display_ON_OFF = SET;
unsigned char Display_Brightness = 128;
unsigned char Buffer_Select = 0;
unsigned char Pwm_Res = 8;
unsigned char Gamma = 22;

unsigned char Status = PLAY;
unsigned char Mute = RESET;

float Volume_Value_float = 0;

uint32_t IR_Receive_Data;
uint32_t IR_Receive_Count;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_DAC_Init(void);
static void MX_SDMMC1_SD_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);
static void MX_TIM14_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
void fill_flame_buffer_color(unsigned char, unsigned char, unsigned char);
void fill_flame_buffer_random(void);
void fill_flame_buffer_testpattern(void);
void draw_text(unsigned char, unsigned char, char*, unsigned char, unsigned char, unsigned char);
void draw_bargraph(float, float, unsigned char, unsigned char, unsigned char, unsigned char, unsigned char, unsigned char, unsigned char, unsigned char);
void send_command_led_display(unsigned char, unsigned char, unsigned char, unsigned char);
void send_data_led_display(void);
void pop_noise_reduction(void);
READ_FILE_RESULT get_playlist(char *, PLAY_INFO **, uint8_t *);
READ_FILE_RESULT read_avi_header(PLAY_INFO *);
READ_FILE_RESULT read_avi_stream(PLAY_INFO *, uint32_t);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  PLAY_INFO *playlist;

  unsigned char playlist_count = 0;
  unsigned char track_count = 0;
  unsigned char all_track_count = 0;
  unsigned int frame_count = 0;
  char playlist_filename[_MAX_LFN] = "list0.csv";

  float flame_rate;

  unsigned char loop_status = LOOP_ALL;
  unsigned char ledpanel_brightness = 100;
  unsigned char volume_value = 5;

  unsigned int previous_sw_value = 0;

  char display_text_buffer[DISPLAY_TEXT_MAX];
  unsigned short display_text_flame;
  unsigned short display_text_flame_count = 0;
  char status_text[DISPLAY_TEXT_MAX] = {0};

  char movie_time[20] = {0};
  unsigned char movie_total_time_min, movie_total_time_sec;
  unsigned char movie_current_time_min, movie_current_time_sec;

  char mediainfo_char_buffer[DISPLAY_TEXT_MAX];

  unsigned char display_OSD = SET;
  unsigned char display_status = SET;
  unsigned char display_mediainfo = RESET;

  unsigned short tim7_count_value;
  unsigned int tim7_count_add = 0;
  unsigned char display_fps_average_count = 0;

  /* USER CODE END 1 */
  

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DAC_Init();
  MX_SDMMC1_SD_Init();
  MX_SPI1_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_TIM14_Init();
  MX_USART2_UART_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  __HAL_DMA_ENABLE_IT(&hdma_spi1_tx, DMA_IT_TC);

  send_command_led_display(0x00, 0, MATRIXLED_X_COUNT - 1, 0x00);
  send_command_led_display(0x04, Display_ON_OFF, 0x01, Display_Brightness);
  send_command_led_display(0x05, Pwm_Res, Gamma, 0x00);
  HAL_Delay(2);
  send_command_led_display(0x06, 255, 200, 255);
  HAL_Delay(2);
  send_command_led_display(0x08, 2, 0, 1);
  send_command_led_display(0x09, 0x01, 0x00, 0x00);
  send_data_led_display();

  retSD = f_mount(&fs, SDPath, 1);
  if(FR_OK != retSD){
    printf("f_mount NG fatfs_result=%d\r\n", retSD);
  }//*/

  snprintf(playlist_filename, _MAX_LFN, "list%1u.csv", playlist_count);
  draw_text(0, 0, playlist_filename, 0xff, 0xff, 0xff);
  send_data_led_display();
  get_playlist(playlist_filename, &playlist, &all_track_count);

  read_avi_stream(&playlist[0], 0);
  snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "%02u:%s", track_count, playlist[track_count].file_name);
  draw_text(0, 0, display_text_buffer, 0xff, 0xff, 0xff);
  display_text_flame_count = 0;

  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
  HAL_DAC_Start(&hdac, DAC_CHANNEL_2);
  pop_noise_reduction();

  HAL_TIM_IC_Start_IT(&htim14, TIM_CHANNEL_1);
  __HAL_TIM_ENABLE_IT(&htim14, TIM_IT_UPDATE);

  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_Base_Start_IT(&htim7);

  /*
  uint32_t test_retval;
  test_retval = SDMMC_CmdSwitch(&hsd1.Instance, (uint32_t)0x80FFFF01);
  if(test_retval != HAL_OK){
    printf("SDMMC_CmdSwitch error 0x%08x\r\n", test_retval);
  }else{
    printf("SDMMC_CmdSwitch ok\r\n");
  }//*/

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    Display_Brightness = ((unsigned int)ledpanel_brightness * 0xff) / 100;

    if(Mute == RESET){
      Volume_Value_float = (float)volume_value / 100;
    }else{
      Volume_Value_float = 0;
    }

    switch(IR_Receive_Data){
    //LEDディスプレイのONOFF
    case IR_REMOTE_SW_0_0:
      if(previous_sw_value != IR_REMOTE_SW_0_0){
        Display_ON_OFF = (~Display_ON_OFF & 0x01);
        Status = STOP;
      }
    break;

    //onスクリーンディスプレイのONOFF
    case IR_REMOTE_SW_6_0:
      if(previous_sw_value != IR_REMOTE_SW_6_0){
        if(display_OSD == RESET){
          display_OSD = SET;
          display_text_flame_count = 0;
          snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "OSD ON");
        }else{
          display_OSD = RESET;
        }
      }
      break;

    //FPS表示のONOFF
    case IR_REMOTE_SW_6_1:
      if(previous_sw_value != IR_REMOTE_SW_6_1){
        if(display_status == RESET){
          display_status = SET;
        }else{
          display_status = RESET;
        }
      }
      break;

    //PlayList切り替え
    case IR_REMOTE_SW_3_0:
      if(previous_sw_value != IR_REMOTE_SW_3_0){
        track_count = 0;
        frame_count = 0;
        if(Status == PLAY){
          Status = STOP;
          do{
            if(playlist_count < MAX_PLAYLIST_COUNT){
              playlist_count++;
            }else{
              playlist_count = 0;
            }
            snprintf(playlist_filename, _MAX_LFN, "list%1u.csv", playlist_count);
            free(playlist);
            get_playlist(playlist_filename, &playlist, &all_track_count);
          }while(all_track_count == 0);

          display_text_flame_count = 0;
          snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "%s", playlist_filename);
          Status = PLAY;
        }else{
          Status = STOP;
          do{
            if(playlist_count < MAX_PLAYLIST_COUNT){
              playlist_count++;
            }else{
              playlist_count = 0;
            }
            snprintf(playlist_filename, _MAX_LFN, "list%1u.csv", playlist_count);
            free(playlist);
            get_playlist(playlist_filename, &playlist, &all_track_count);
          }while(all_track_count == 0);

          display_text_flame_count = 0;
          snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "%s", playlist_filename);
        }
      }
      break;

    //LoopMode切り替え
    case IR_REMOTE_SW_0_1:
      if(previous_sw_value != IR_REMOTE_SW_0_1){
        if(loop_status == LOOP_ALL){
          loop_status = LOOP_SINGLE;
          display_text_flame_count = 0;
          snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "Loop one");
        }else if(loop_status == LOOP_SINGLE){
          loop_status = LOOP_NO;
          display_text_flame_count = 0;
          snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "No loop");
        }else{
          loop_status = LOOP_ALL;
          display_text_flame_count = 0;
          snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "Loop all");
        }
      }
      break;

    //PlayPause切り替え
    case IR_REMOTE_SW_1_0:
      if(previous_sw_value != IR_REMOTE_SW_1_0){
        if(Status == PLAY){
          Status = PAUSE;
          display_text_flame_count = 0;
          snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "Pause");
        }else if((Status == PAUSE) || (Status == STOP)){
          Status = PLAY;
          display_text_flame_count = 0;
          snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "Play");
        }else{
        }
      }
      break;

    //Stop
    case IR_REMOTE_SW_2_0:
      if(previous_sw_value != IR_REMOTE_SW_2_0){
        Status = STOP;
        frame_count = 0;
        display_text_flame_count = 0;
        snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "Stop");
      }
      break;

    //Track戻し
    case IR_REMOTE_SW_1_1:
      if(previous_sw_value != IR_REMOTE_SW_1_1){
        frame_count = 0;
        if(track_count > 0){
          track_count--;
        }else{
          track_count = all_track_count - 1;
        }
        display_text_flame_count = 0;
        snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "%02u:%s", track_count, playlist[track_count].file_name);
        if(Status == PLAY){
          Status = PAUSE;
          Status = PLAY;
        }else{
        }
      }
      break;

    //Track進め
    case IR_REMOTE_SW_1_2:
      if(previous_sw_value != IR_REMOTE_SW_1_2){
        frame_count = 0;
        if(track_count < (all_track_count - 1)){
          track_count++;
        }else{
          track_count = 0;
        }
        display_text_flame_count = 0;
        snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "%02u:%s", track_count, playlist[track_count].file_name);
        if(Status == PLAY){
          Status = PAUSE;
          Status = PLAY;
        }else{
        }
      }
      break;

    //Volume増加
    case IR_REMOTE_SW_2_2:
      if(previous_sw_value != IR_REMOTE_SW_2_2){
        if(Mute == RESET){
          if(volume_value < (100 - 4)){
            volume_value += 5;
          }
          display_text_flame_count = 0;
          snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "Volume%u%%", volume_value);
        }else{
          display_text_flame_count = 0;
          snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "Mute");
        }
      }
      break;

    //Volume減少
    case IR_REMOTE_SW_2_1:
      if(previous_sw_value != IR_REMOTE_SW_2_1){
        if(Mute == RESET){
          if(volume_value >= 5){
            volume_value -= 5;
          }
          display_text_flame_count = 0;
          snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "Volume%u%%", volume_value);
        }else{
          display_text_flame_count = 0;
          snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "Mute");
        }
      }
      break;

    //Display明るさ増加
    case IR_REMOTE_SW_3_2:
      if(previous_sw_value != IR_REMOTE_SW_3_2){
        if(ledpanel_brightness < (100 - 4)){
          ledpanel_brightness += 5;
        }
        display_text_flame_count = 0;
        snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "Bright%u%%", ledpanel_brightness);
      }
      break;

    //Display明るさ減少
    case IR_REMOTE_SW_3_1:
      if(previous_sw_value != IR_REMOTE_SW_3_1){
        if(ledpanel_brightness >= 5){
          ledpanel_brightness -= 5;
        }
        display_text_flame_count = 0;
        snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "Bright%u%%", ledpanel_brightness);
      }
      break;

      //PWM値増加
      case IR_REMOTE_SW_5_2:
        if(previous_sw_value != IR_REMOTE_SW_5_2){
          if(Pwm_Res < 12){
            Pwm_Res += 1;
          }
          display_text_flame_count = 0;
          snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "PWM%ubit", Pwm_Res);
        }
        break;

    //PWM値減少
    case IR_REMOTE_SW_5_1:
      if(previous_sw_value != IR_REMOTE_SW_5_1){
        if(Pwm_Res > 1){
          Pwm_Res -= 1;
        }
        display_text_flame_count = 0;
        snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "PWM%ubit", Pwm_Res);
      }
      break;

      //ガンマ値増加
      case IR_REMOTE_SW_4_0:
        if(previous_sw_value != IR_REMOTE_SW_4_0){
          if(Gamma < 50){
            Gamma += 2;
          }
          display_text_flame_count = 0;
          snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "GAMMA%u", Gamma);
        }
        break;

      //ガンマ値減少
      case IR_REMOTE_SW_5_0:
        if(previous_sw_value != IR_REMOTE_SW_5_0){
          if(Gamma > 6){
            Gamma -= 2;
          }
          display_text_flame_count = 0;
          snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "GAMMA%u", Gamma);
        }
        break;

    //Mute切り替え
    case IR_REMOTE_SW_0_2:
      if(previous_sw_value != IR_REMOTE_SW_0_2){
        if(Mute == RESET){
          Mute = SET;
          display_text_flame_count = 0;
          snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "Mute");
        }else if(Mute == SET){
          Mute = RESET;
          display_text_flame_count = 0;
          snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "Volume%u%%", volume_value);
        }else{
        }
      }
      break;
      //SKIP_TIME

    //SKIP
    case IR_REMOTE_SW_4_2:
      if((frame_count + SKIP_TIME) < playlist[track_count].avi_info.video_length){
        frame_count += SKIP_TIME;
      }
      /*
      if(previous_sw_value != IR_REMOTE_SW_4_2){
        if((frame_count + SKIP_TIME * playlist[track_count].avi_info.video_frame_rate) < playlist[track_count].avi_info.video_length){
          frame_count += SKIP_TIME * playlist[track_count].avi_info.video_frame_rate;
        }
      }*/
      break;

    //BACK
    case IR_REMOTE_SW_4_1:
      if(((int64_t)frame_count - (int64_t)SKIP_TIME) > 0){
        frame_count -= SKIP_TIME;
      }
      /*
      if(previous_sw_value != IR_REMOTE_SW_4_1){
        if((frame_count - SKIP_TIME * playlist[track_count].avi_info.video_frame_rate) > 0){
          frame_count -= SKIP_TIME * playlist[track_count].avi_info.video_frame_rate;
        }
      }*/
      break;

    //MediaInfo表示のONOFF
    case IR_REMOTE_SW_6_2:
      if(previous_sw_value != IR_REMOTE_SW_6_2){
        if(display_mediainfo == RESET){
          display_mediainfo = SET;
        }else{
          display_mediainfo = RESET;
        }
      }
      break;

    default:
      break;
    }

  previous_sw_value = IR_Receive_Data;

  flame_rate = playlist[track_count].avi_info.video_frame_rate;
  display_text_flame = (unsigned short)(flame_rate * 1000) / DISPLAY_TEXT_TIME;

  read_avi_stream(&playlist[track_count], frame_count);

  if(PLAY == Status){
    if(frame_count < playlist[track_count].avi_info.video_length){
      frame_count++;
    }else{
      frame_count = 0;
      Video_End_Flag = SET;
      Audio_End_Flag = SET;
    }
  }else if(PAUSE == Status){

  }else{
    frame_count = 0;
  }

  if((Video_End_Flag == SET) && (Audio_End_Flag == SET)){
    switch(loop_status){
    case LOOP_NO:
      Status = STOP;
      display_text_flame_count = 0;
      snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "Stop");
      break;
    case LOOP_SINGLE:
      display_text_flame_count = 0;
      snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "%02u:%s", track_count, playlist[track_count].file_name);
      break;
    case LOOP_ALL:
      if(track_count < (all_track_count - 1)){
        track_count++;
      }else{
        track_count = 0;
      }
      display_text_flame_count = 0;
      snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "%02u:%s", track_count, playlist[track_count].file_name);
      break;
    default:
      Status = STOP;
      display_text_flame_count = 0;
      snprintf(display_text_buffer, DISPLAY_TEXT_MAX, "Stop");
      break;
    }

    Video_End_Flag = RESET;
    Audio_End_Flag = RESET;
  }

  if((display_text_flame_count <= display_text_flame) && (display_OSD == SET)){
    if(display_mediainfo == SET){
      draw_text(64, 0, display_text_buffer, 0xff, 0xff, 0xff);
    }else{
      draw_text(0, 0, display_text_buffer, 0xff, 0xff, 0xff);
    }
    display_text_flame_count++;
  }else{
    display_text_flame_count = display_text_flame + 1;
  }

  if(display_status == SET){
    //FPS計算
    tim7_count_value = __HAL_TIM_GET_COUNTER(&htim7);
    __HAL_TIM_SET_COUNTER(&htim7, 0);
    if((DISPLAY_FPS_AVERAGE_TIME * flame_rate) < 1){
      snprintf(status_text, DISPLAY_TEXT_MAX, "%5.2fFPS", ((float)TIM7_FREQ / (float)(TIM7_PRESCALER * tim7_count_value)));
      display_fps_average_count = 0;
      tim7_count_add = 0;
    }else if(display_fps_average_count < (DISPLAY_FPS_AVERAGE_TIME * flame_rate)){
      tim7_count_add += tim7_count_value;
      display_fps_average_count++;
    }else{
      snprintf(status_text, DISPLAY_TEXT_MAX, "%5.2fFPS", ((float)display_fps_average_count * ((float)TIM7_FREQ / (float)(TIM7_PRESCALER * tim7_count_add))));
      display_fps_average_count = 0;
      tim7_count_add = 0;
    }

    //AVI動画時間計算
    movie_total_time_min = playlist[track_count].avi_info.video_length / (playlist[track_count].avi_info.video_frame_rate * 60);
    movie_total_time_sec = (unsigned int)(playlist[track_count].avi_info.video_length / playlist[track_count].avi_info.video_frame_rate) % 60;
    movie_current_time_min = frame_count / (playlist[track_count].avi_info.video_frame_rate * 60);
    movie_current_time_sec = (unsigned int)(frame_count / playlist[track_count].avi_info.video_frame_rate) % 60;
    snprintf(movie_time, DISPLAY_TEXT_MAX, "%2u:%02u/%2u:%02u", movie_current_time_min, movie_current_time_sec, movie_total_time_min, movie_total_time_sec);

    draw_bargraph(frame_count, playlist[track_count].avi_info.video_length, HORIZONTAL, 0, 124, 255, 127, 0xff, 0xff, 0x00);
    draw_text(0, 115, status_text, 0xff, 0xff, 0xff);
    draw_text(189, 115, movie_time, 0xff, 0xff, 0xff);

  }else{
    display_fps_average_count = 0;
    tim7_count_add = 0;
  }

  if(display_mediainfo == SET){
    draw_text(0, 0, playlist_filename, 0xff, 0xff, 0xff);
    snprintf(mediainfo_char_buffer, DISPLAY_TEXT_MAX, "%02u:%s", track_count, playlist[track_count].file_name);
    draw_text(0, 9, mediainfo_char_buffer, 0xff, 0xff, 0xff);
    snprintf(mediainfo_char_buffer, DISPLAY_TEXT_MAX, "%.2fMB", ((float)playlist[track_count].avi_info.avi_file_size / (float)0x00100000));
    draw_text(0, 18, mediainfo_char_buffer, 0xff, 0xff, 0xff);
    snprintf(mediainfo_char_buffer, DISPLAY_TEXT_MAX, "%.3fFPS", playlist[track_count].avi_info.video_frame_rate);
    draw_text(0, 27, mediainfo_char_buffer, 0xff, 0xff, 0xff);
    snprintf(mediainfo_char_buffer, DISPLAY_TEXT_MAX, "%.1fkHz %uch", ((float)playlist[track_count].avi_info.audio_sampling_rate / 1000.0), playlist[track_count].avi_info.audio_channels);
    draw_text(0, 36, mediainfo_char_buffer, 0xff, 0xff, 0xff);

    snprintf(mediainfo_char_buffer, DISPLAY_TEXT_MAX, "IRdata:0x%08x", (unsigned int)IR_Receive_Data);
    draw_text(0, 45, mediainfo_char_buffer, 0xff, 0xff, 0xff);
    snprintf(mediainfo_char_buffer, DISPLAY_TEXT_MAX, "Volume%u%%", volume_value);
    draw_text(0, 54, mediainfo_char_buffer, 0xff, 0xff, 0xff);
    snprintf(mediainfo_char_buffer, DISPLAY_TEXT_MAX, "Bright%u%%", ledpanel_brightness);
    draw_text(0, 63, mediainfo_char_buffer, 0xff, 0xff, 0xff);
    snprintf(mediainfo_char_buffer, DISPLAY_TEXT_MAX, "Gamma%u", Gamma);
    draw_text(0, 72, mediainfo_char_buffer, 0xff, 0xff, 0xff);
    snprintf(mediainfo_char_buffer, DISPLAY_TEXT_MAX, "PWM%ubit", Pwm_Res);
    draw_text(0, 81, mediainfo_char_buffer, 0xff, 0xff, 0xff);
    if(loop_status == LOOP_ALL){
      draw_text(0, 90, "loop all", 0xff, 0xff, 0xff);
    }else if(loop_status == LOOP_SINGLE){
      draw_text(0, 90, "loop one", 0xff, 0xff, 0xff);
    }else{
      draw_text(0, 90, "no loop", 0xff, 0xff, 0xff);
    }
  }

  send_data_led_display();

  while(Audio_Flame_End_flag == RESET);
  Audio_Flame_End_flag = RESET;

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure LSE Drive Capability 
  */
  HAL_PWR_EnableBkUpAccess();
  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_SDMMC1
                              |RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
  PeriphClkInitStruct.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_CLK48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */
  /** DAC Initialization 
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }
  /** DAC channel OUT1 config 
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /** DAC channel OUT2 config 
  */
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

}

/**
  * @brief SDMMC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC1_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC1_Init 0 */

  /* USER CODE END SDMMC1_Init 0 */

  /* USER CODE BEGIN SDMMC1_Init 1 */

  /* USER CODE END SDMMC1_Init 1 */
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockBypass = SDMMC_CLOCK_BYPASS_ENABLE;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_1B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
  hsd1.Init.ClockDiv = 0;
  /* USER CODE BEGIN SDMMC1_Init 2 */
  //hsd1.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;

  /* USER CODE END SDMMC1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 0;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 2448;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 150;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 65535;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 181;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 65535;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 15;
  if (HAL_TIM_IC_ConfigChannel(&htim14, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  /* DMA2_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OUT_SPI_DC_SELECT_GPIO_Port, OUT_SPI_DC_SELECT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : OUT_SPI_DC_SELECT_Pin */
  GPIO_InitStruct.Pin = OUT_SPI_DC_SELECT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(OUT_SPI_DC_SELECT_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void fill_flame_buffer_color(unsigned char red, unsigned char green, unsigned char blue){
  unsigned char red_temp, green_temp, blue_temp;
  unsigned int fill_buffer_loop_x, fill_buffer_loop_y;

  if(red < MATRIXLED_PWM_RESOLUTION) red_temp = red;
  else red_temp = MATRIXLED_PWM_RESOLUTION - 1;

  if(green < MATRIXLED_PWM_RESOLUTION) green_temp = green;
  else green_temp = MATRIXLED_PWM_RESOLUTION - 1;

  if(blue < MATRIXLED_PWM_RESOLUTION) blue_temp = blue;
  else blue_temp = MATRIXLED_PWM_RESOLUTION - 1;

  for(fill_buffer_loop_y = 0;fill_buffer_loop_y < MATRIXLED_Y_COUNT;fill_buffer_loop_y++){
    for(fill_buffer_loop_x = 0;fill_buffer_loop_x < MATRIXLED_X_COUNT;fill_buffer_loop_x++){
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x][COLOR_R] = red_temp;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x][COLOR_G] = green_temp;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x][COLOR_B] = blue_temp;
    }
  }
}

void fill_flame_buffer_random(void){
  unsigned int fill_buffer_loop_x, fill_buffer_loop_y;

  srand(rand());

  for(fill_buffer_loop_y = 0;fill_buffer_loop_y < MATRIXLED_Y_COUNT;fill_buffer_loop_y++){
    for(fill_buffer_loop_x = 0;fill_buffer_loop_x < MATRIXLED_X_COUNT;fill_buffer_loop_x++){
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x][COLOR_R] = rand() % MATRIXLED_PWM_RESOLUTION;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x][COLOR_G] = rand() % MATRIXLED_PWM_RESOLUTION;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x][COLOR_B] = rand() % MATRIXLED_PWM_RESOLUTION;
    }
  }
}

void fill_flame_buffer_testpattern(void){
  unsigned int fill_buffer_loop_x, fill_buffer_loop_y;

  for(fill_buffer_loop_y = 0;fill_buffer_loop_y < MATRIXLED_Y_COUNT;fill_buffer_loop_y++){
    for(fill_buffer_loop_x = 0;fill_buffer_loop_x < (MATRIXLED_X_COUNT / 12);fill_buffer_loop_x++){
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x][COLOR_R] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x][COLOR_G] = (MATRIXLED_PWM_RESOLUTION - 1);
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x][COLOR_B] = (MATRIXLED_PWM_RESOLUTION - 1);

      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12)][COLOR_R] = (MATRIXLED_PWM_RESOLUTION - 1);
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12)][COLOR_G] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12)][COLOR_B] = (MATRIXLED_PWM_RESOLUTION - 1);

      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 2][COLOR_R] = (MATRIXLED_PWM_RESOLUTION - 1);
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 2][COLOR_G] = (MATRIXLED_PWM_RESOLUTION - 1);
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 2][COLOR_B] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;

      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 3][COLOR_R] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 3][COLOR_G] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 3][COLOR_B] = (MATRIXLED_PWM_RESOLUTION - 1);

      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 4][COLOR_R] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 4][COLOR_G] = (MATRIXLED_PWM_RESOLUTION - 1);
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 4][COLOR_B] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;

      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 5][COLOR_R] = (MATRIXLED_PWM_RESOLUTION - 1);
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 5][COLOR_G] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 5][COLOR_B] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;

      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 6][COLOR_R] = 0;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 6][COLOR_G] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 6][COLOR_B] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;

      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 7][COLOR_R] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 7][COLOR_G] = 0;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 7][COLOR_B] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;

      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 8][COLOR_R] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 8][COLOR_G] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 8][COLOR_B] = 0;

      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 9][COLOR_R] = 0;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 9][COLOR_G] = 0;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 9][COLOR_B] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;

      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 10][COLOR_R] = 0;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 10][COLOR_G] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 10][COLOR_B] = 0;

      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 11][COLOR_R] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 11][COLOR_G] = 0;
      Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 11][COLOR_B] = 0;

      if(fill_buffer_loop_x < (MATRIXLED_X_COUNT % 12)){
        Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 12][COLOR_R] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;
        Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 12][COLOR_G] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;
        Flame_Buffer[fill_buffer_loop_y][fill_buffer_loop_x + (MATRIXLED_X_COUNT / 12) * 12][COLOR_B] = fill_buffer_loop_y * MATRIXLED_PWM_RESOLUTION / MATRIXLED_Y_COUNT;
      }
    }
  }
}

void draw_text(unsigned char start_pixel_x, unsigned char start_pixel_y, char* chara_array, unsigned char font_color_r, unsigned char font_color_g, unsigned char font_color_b){
  unsigned int loop = 0, pixel_count_x = 0;
  unsigned char ascii;
  unsigned char pixel_data[MATRIXLED_X_COUNT + FONT_DATA_PIXEL_SIZE_X * 2 + 2] = {};

  unsigned char font_data_lead_loop;

  unsigned int x_loop, y_loop;

  if(start_pixel_x >= MATRIXLED_X_COUNT) return;
  if((start_pixel_y + FONT_DATA_PIXEL_SIZE_Y) >= MATRIXLED_Y_COUNT) return;

  while((chara_array[loop] |= '\0') && (pixel_count_x < MATRIXLED_X_COUNT))
  {
    pixel_count_x = loop * (FONT_DATA_PIXEL_SIZE_X + 1) + 1;
    ascii = chara_array[loop] - 0x20;
    if(ascii >= FONT_DATA_MAX_CHAR){
      ascii = 0;
    }
    for(font_data_lead_loop = 0;font_data_lead_loop <= FONT_DATA_PIXEL_SIZE_X;font_data_lead_loop++){
      if(font_data_lead_loop < FONT_DATA_PIXEL_SIZE_X){
        pixel_data[pixel_count_x + font_data_lead_loop] = Font_Data[ascii][font_data_lead_loop];
      }else{
        pixel_data[pixel_count_x + font_data_lead_loop] = 0x00;
      }
    }
    loop++;
  }

  pixel_count_x += FONT_DATA_PIXEL_SIZE_X;

  if((start_pixel_x + pixel_count_x) >= MATRIXLED_X_COUNT) pixel_count_x = MATRIXLED_X_COUNT - start_pixel_x - 1;

  for(y_loop = 0;y_loop <= FONT_DATA_PIXEL_SIZE_Y;y_loop++){
    for(x_loop = 0;x_loop <= pixel_count_x;x_loop++){
      if(((pixel_data[x_loop] >> (y_loop - 1)) & 0x01) == 0x01){
        Flame_Buffer[MATRIXLED_Y_COUNT - (start_pixel_y + y_loop + 1)][start_pixel_x + x_loop][COLOR_R] = font_color_r;
        Flame_Buffer[MATRIXLED_Y_COUNT - (start_pixel_y + y_loop + 1)][start_pixel_x + x_loop][COLOR_G] = font_color_g;
        Flame_Buffer[MATRIXLED_Y_COUNT - (start_pixel_y + y_loop + 1)][start_pixel_x + x_loop][COLOR_B] = font_color_b;
      }else{
        Flame_Buffer[MATRIXLED_Y_COUNT - (start_pixel_y + y_loop + 1)][start_pixel_x + x_loop][COLOR_R] >>= 1;
        Flame_Buffer[MATRIXLED_Y_COUNT - (start_pixel_y + y_loop + 1)][start_pixel_x + x_loop][COLOR_G] >>= 1;
        Flame_Buffer[MATRIXLED_Y_COUNT - (start_pixel_y + y_loop + 1)][start_pixel_x + x_loop][COLOR_B] >>= 1;
      }
    }
  }
}

void draw_bargraph(float value, float max_value, unsigned char direction, unsigned char start_pixel_x, unsigned char start_pixel_y, unsigned char end_pixel_x, unsigned char end_pixel_y, unsigned char graph_color_r, unsigned char graph_color_g, unsigned char graph_color_b){
  unsigned int x_loop, y_loop;
  float value_temp;
  unsigned int bargraph_length_pixel = 0;

  if((start_pixel_x < end_pixel_x) && (start_pixel_y < end_pixel_y) &&
     (end_pixel_x < MATRIXLED_X_COUNT) && (end_pixel_y < MATRIXLED_Y_COUNT) &&
    ((end_pixel_x - start_pixel_x) >= 3) && ((end_pixel_y - start_pixel_y) >= 3)){

    if(value < 0.0){
      value_temp = 0.0;
    }else if(value > max_value){
      value_temp = max_value;
    }else{
      value_temp = value;
    }

    if(direction == VERTICAL){
      bargraph_length_pixel = round(((float)end_pixel_y - (float)start_pixel_y - 1) * value_temp / max_value);
    }else if(direction == HORIZONTAL){
      bargraph_length_pixel = round(((float)end_pixel_x - (float)start_pixel_x - 1) * value_temp / max_value);
    }else{
      bargraph_length_pixel = 0;
    }

    for(y_loop = 0;y_loop <= (end_pixel_y - start_pixel_y);y_loop++){
      for(x_loop = 0;x_loop <= (end_pixel_x - start_pixel_x);x_loop++){
        if((x_loop > 0) && (x_loop < (end_pixel_x - start_pixel_x)) &&
           (y_loop > 0) && (y_loop < (end_pixel_y - start_pixel_y)) &&
           (((direction == VERTICAL) && (bargraph_length_pixel >= (end_pixel_y - start_pixel_y - y_loop))) ||
            ((direction == HORIZONTAL) && (bargraph_length_pixel >= x_loop)))){
          Flame_Buffer[MATRIXLED_Y_COUNT - (start_pixel_y + y_loop + 1)][start_pixel_x + x_loop][COLOR_R] = graph_color_r;
          Flame_Buffer[MATRIXLED_Y_COUNT - (start_pixel_y + y_loop + 1)][start_pixel_x + x_loop][COLOR_G] = graph_color_g;
          Flame_Buffer[MATRIXLED_Y_COUNT - (start_pixel_y + y_loop + 1)][start_pixel_x + x_loop][COLOR_B] = graph_color_b;
        }else{
          Flame_Buffer[MATRIXLED_Y_COUNT - (start_pixel_y + y_loop + 1)][start_pixel_x + x_loop][COLOR_R] >>= 1;
          Flame_Buffer[MATRIXLED_Y_COUNT - (start_pixel_y + y_loop + 1)][start_pixel_x + x_loop][COLOR_G] >>= 1;
          Flame_Buffer[MATRIXLED_Y_COUNT - (start_pixel_y + y_loop + 1)][start_pixel_x + x_loop][COLOR_B] >>= 1;
        }
      }
    }

  }
}
/*
void send_data_led_display(void){

  //while(SPI_DMA_Complete_Flag == RESET);
  //SPI_DMA_Complete_Flag = RESET;

  LEDdisplay_Command[1] = 63;

  HAL_GPIO_WritePin(OUT_SPI_DC_SELECT_GPIO_Port, OUT_SPI_DC_SELECT_Pin, GPIO_PIN_SET);
  for(uint16_t loop = 0;loop < 250;loop++){asm("nop");}
  HAL_SPI_Transmit(&hspi1, LEDdisplay_Command, SPI_SEND_COMMAND_COUNT, 1000);

  //SPI_DMA_Complete_Flag = RESET;

  HAL_GPIO_WritePin(OUT_SPI_DC_SELECT_GPIO_Port, OUT_SPI_DC_SELECT_Pin, GPIO_PIN_RESET);
  for(uint16_t loop = 0;loop < 250;loop++){asm("nop");}
  HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)Flame_Buffer, MATRIXLED_Y_COUNT * MATRIXLED_X_COUNT * MATRIXLED_COLOR_COUNT >> 1);

}*/

READ_FILE_RESULT get_playlist(char *playlist_filename, PLAY_INFO **playlist, uint8_t *track_count){
  FILINFO playlist_fileinfo;
  FRESULT fatfs_result;
  FIL playlist_fileobject;

  uint8_t *token_pointer;
  uint8_t playlist_data[_MAX_LFN];
  uint8_t track_count_temp = 0;

  PLAY_INFO *playlist_temp0;
  PLAY_INFO *playlist_temp1;

  fatfs_result = f_stat(playlist_filename, &playlist_fileinfo);
  if(FR_OK != fatfs_result){
    printf("%s f_stat error %u\r\n", playlist_filename, fatfs_result);
    //fatfs_result = f_open(&playlist_fileobject, playlist_filename, FA_WRITE|FA_OPEN_ALWAYS);
    //fatfs_result = f_write(&playlist_fileobject, Playlist_Default_Message, 461, NULL);
    //fatfs_result = f_close(&playlist_fileobject);
    goto FATFS_ERROR_PROCESS;
  }

  fatfs_result = f_open(&playlist_fileobject, (TCHAR*)&playlist_fileinfo.fname, FA_READ);
  if(FR_OK != fatfs_result){
    printf("%s f_open error %u\r\n", playlist_filename, fatfs_result);
    goto FATFS_ERROR_PROCESS;
  }

  while((f_gets((TCHAR*)playlist_data, (_MAX_LFN + 16), &playlist_fileobject) != NULL) && (track_count_temp <= MAX_TRACK_COUNT)){
    //printf("%s\r\n", strtok((char*)playlist_data, "\r\n"));
    track_count_temp++;
  }
  //printf("\r\n");
  if(0 == track_count_temp){
    printf("item does not exist in playlist\r\n");
    goto FILE_ERROR_PROCESS;
  }

  fatfs_result = f_lseek(&playlist_fileobject, 0);
  if(FR_OK != fatfs_result){
    printf("%s f_lseek error %u\r\n", playlist_filename, fatfs_result);
    goto FATFS_ERROR_PROCESS;
  }

  playlist_temp0 = (PLAY_INFO *)malloc(sizeof(PLAY_INFO) * track_count_temp);
  if(NULL == playlist_temp0){
    printf("malloc error\r\n");
    goto OTHER_ERROR_PROCESS;
  }

  track_count_temp = 0;
  while((f_gets((TCHAR*)playlist_data, (_MAX_LFN + 16), &playlist_fileobject) != NULL) && (track_count_temp <= MAX_TRACK_COUNT)){
    token_pointer = (uint8_t*)strtok((char*)playlist_data, ",\r\n");
    if((token_pointer != NULL) && (strncmp((char*)playlist_data, "//", 2) != 0)){
      strcpy((char*)playlist_temp0[track_count_temp].file_name, (char*)token_pointer);
      if(FILE_OK == read_avi_header(&playlist_temp0[track_count_temp])){
        track_count_temp++;
      }
    }
  }
  if(0 == track_count_temp){
    free(playlist_temp0);
    printf("item does not exist in playlist\r\n");
    goto FILE_ERROR_PROCESS;
  }

  playlist_temp1 = (PLAY_INFO *)realloc(playlist_temp0, (sizeof(PLAY_INFO) * track_count_temp));
  if(NULL == playlist_temp1){
    free(playlist_temp0);
    printf("realloc error\r\n");
    goto OTHER_ERROR_PROCESS;
  }

  fatfs_result = f_close(&playlist_fileobject);
  if(FR_OK != fatfs_result){
    free(playlist_temp1);
    printf("%s f_close error %u\r\n", playlist_filename, fatfs_result);
    goto FATFS_ERROR_PROCESS;
  }


  /*
  for(int testloop = 0;testloop < track_count_temp;testloop++){
    printf("ファイル名:%s\r\n", playlist_temp1[testloop].file_name);
    printf("映像フレームレート:%6.3fFPS\r\n", playlist_temp1[testloop].avi_info.video_frame_rate);
    printf("映像長さ:%luフレーム\r\n", playlist_temp1[testloop].avi_info.video_length);
    printf("映像データチャンク名:%.4s\r\n", playlist_temp1[testloop].avi_info.video_data_chunk_name.fcc);
    printf("音声チャンネル数:%uチャンネル\r\n", playlist_temp1[testloop].avi_info.audio_channels);
    printf("音声サンプリング周波数:%luHz\r\n", playlist_temp1[testloop].avi_info.audio_sampling_rate);
    printf("音声長さ:%luサンプル\r\n", playlist_temp1[testloop].avi_info.audio_length);
    printf("音声データチャンク名:%.4s\r\n", playlist_temp1[testloop].avi_info.audio_data_chunk_name.fcc);
    printf("ストリーム数:%lu\r\n", playlist_temp1[testloop].avi_info.avi_streams_count);
    printf("moviリスト位置:0x%08lx\r\n", playlist_temp1[testloop].avi_info.movi_list_position);
    printf("idx1インデックス位置:0x%08lx\r\n", playlist_temp1[testloop].avi_info.avi_old_index_position);
    printf("idx1インデックス長さ:0x%08lx\r\n", playlist_temp1[testloop].avi_info.avi_old_index_size);
    printf("AVIファイルサイズ:%luB\r\n", playlist_temp1[testloop].avi_info.avi_file_size);
    printf("\r\n");
  }
  //*/

  (*playlist) = playlist_temp1;
  (*track_count) = track_count_temp;
  return FILE_OK;

  FILE_ERROR_PROCESS:
  (*playlist) = NULL;
  (*track_count) = 0;
  return FILE_ERROR;

  FATFS_ERROR_PROCESS:
  (*playlist) = NULL;
  (*track_count) = 0;
  return FATFS_ERROR;

  OTHER_ERROR_PROCESS:
  (*playlist) = NULL;
  (*track_count) = 0;
  return OTHER_ERROR;
};



READ_FILE_RESULT read_avi_header(PLAY_INFO *play_info){
  FRESULT fatfs_result;
  FIL avi_fileobject;

  uint32_t linkmap_table[LINKMAP_TABLE_SIZE];

  RIFFCHUNK *riff_chunk = NULL;
  RIFFLIST *riff_list = NULL;
  FOURCC *four_cc = NULL;
  AVIMAINHEADER *avi_main_header = NULL;
  RIFFLIST *strl_list = NULL;
  AVISTREAMHEADER *avi_stream_header = NULL;
  RIFFCHUNK *strf_chunk = NULL;
  BITMAPINFOHEADER *bitmap_info_header = NULL;
  WAVEFORMATEX *wave_format_ex = NULL;
  RIFFLIST *unknown_list = NULL;
  RIFFLIST *movi_list = NULL;
  RIFFCHUNK *idx1_chunk = NULL;
  //aIndex *avi_old_index;

  uint8_t strl_list_find_loop_count = 0;

  float video_frame_rate;
  uint32_t video_length;
  uint16_t audio_channels;
  uint32_t audio_sampling_rate;
  uint32_t audio_length;
  uint32_t avi_streams_count;
  uint32_t movi_list_position;
  uint32_t avi_old_index_position;
  uint32_t avi_old_index_size;
  uint32_t avi_file_size;

  uint32_t audio_suggest_buffer_size;

  uint8_t chunk_name_temp[5];

  FOURCC audio_data_chunk_name;
  FOURCC video_data_chunk_name;

  uint8_t video_stream_find_flag = RESET;
  uint8_t audio_stream_find_flag = RESET;

  UINT read_data_byte_result;

  printf("read_avi_header %s\r\n", play_info->file_name);

  //ファイルオープン
  fatfs_result = f_open(&avi_fileobject, (TCHAR*)play_info->file_name, FA_READ);
  if(FR_OK != fatfs_result){
    printf("read_avi_header f_open NG fatfs_result=%d\r\n", fatfs_result);
    goto FATFS_ERROR_PROCESS;
  }

  avi_fileobject.cltbl = linkmap_table;
  linkmap_table[0] = LINKMAP_TABLE_SIZE;
  fatfs_result = f_lseek(&avi_fileobject, CREATE_LINKMAP);
  if(FR_OK != fatfs_result){
    printf("read_avi_header create linkmap table NG fatfs_result=%d\r\n", fatfs_result);
    printf("need linkmap table size %lu\r\n", linkmap_table[0]);
    goto FATFS_ERROR_PROCESS;
  }

  //RIFFチャンクの読み込み
  riff_chunk = (RIFFCHUNK *)malloc(sizeof(RIFFCHUNK));
  if(NULL == riff_chunk){
    printf("riff_chunk malloc error\r\n");
    goto OTHER_ERROR_PROCESS;
  }
  fatfs_result = f_read(&avi_fileobject, riff_chunk, sizeof(RIFFCHUNK), &read_data_byte_result);
  if(FR_OK != fatfs_result){
    printf("riff_chunk f_read NG fatfs_result=%d\r\n", fatfs_result);
    goto FATFS_ERROR_PROCESS;
  }
  /*
  printf("RIFFチャンク識別子:%.4s\r\n", riff_chunk->fcc.fcc);
  printf("RIFFチャンクサイズ:%u\r\n", riff_chunk->cb);
  //*/

  avi_file_size = riff_chunk->cb + sizeof(RIFFCHUNK);

  if(strncmp((char*)riff_chunk->fcc.fcc, "RIFF", sizeof(FOURCC)) != 0){
    printf("this file is not RIFF format\r\n");
    goto FILE_ERROR_PROCESS;
  }

  //RIFFフォームタイプの読み込み
  four_cc = (FOURCC *)malloc(sizeof(FOURCC));
  if(NULL == four_cc){
    printf("four_cc malloc error\r\n");
    goto OTHER_ERROR_PROCESS;
  }
  fatfs_result = f_read(&avi_fileobject, four_cc, sizeof(FOURCC), &read_data_byte_result);
  if(FR_OK != fatfs_result){
    printf("four_cc f_read NG fatfs_result=%d\r\n", fatfs_result);
    goto FATFS_ERROR_PROCESS;
  }
  /*
  printf("RIFFチャンクフォームタイプ:%.4s\r\n", four_cc->fcc);
  //*/
  if(strncmp((char*)four_cc->fcc, "AVI ", sizeof(FOURCC)) != 0){
    printf("RIFF form type is not AVI \r\n");
    goto FILE_ERROR_PROCESS;
  }

  //hdrlリストの読み込み
  riff_list = (RIFFLIST *)malloc(sizeof(RIFFLIST));
  if(NULL == riff_list){
    printf("riff_list malloc error\r\n");
    goto OTHER_ERROR_PROCESS;
  }
  fatfs_result = f_read(&avi_fileobject, riff_list, sizeof(RIFFLIST), &read_data_byte_result);
  if(FR_OK != fatfs_result){
    printf("riff_list f_read NG fatfs_result=%d\r\n", fatfs_result);
    goto FATFS_ERROR_PROCESS;
  }
  /*
  printf("  hdrlリスト識別子:%.4s\r\n", riff_list->fcc.fcc);
  printf("  hdrlリストサイズ:%u\r\n", riff_list->cb);
  printf("  hdrlリストタイプ:%.4s\r\n", riff_list->fccListType.fcc);
  //*/
  if((strncmp((char*)riff_list->fcc.fcc, "LIST", sizeof(FOURCC)) != 0) || (strncmp((char*)riff_list->fccListType.fcc, "hdrl ", sizeof(FOURCC)) != 0)){
    printf("could not find hdrl list\r\n");
    goto FILE_ERROR_PROCESS;
  }

  //メインAVIヘッダの読み込み
  avi_main_header = (AVIMAINHEADER *)malloc(sizeof(AVIMAINHEADER));
  if(NULL == avi_main_header){
    printf("avi_main_header malloc error\r\n");
    goto OTHER_ERROR_PROCESS;
  }
  fatfs_result = f_read(&avi_fileobject, avi_main_header, sizeof(AVIMAINHEADER), &read_data_byte_result);
  if(FR_OK != fatfs_result){
    printf("avi_main_header f_read NG fatfs_result=%d\r\n", fatfs_result);
    goto FATFS_ERROR_PROCESS;
  }
  /*
  printf("    avihチャンク識別子:%.4s\r\n", avi_main_header->fcc.fcc);
  printf("    avihチャンクサイズ:%u\r\n", avi_main_header->cb);
  printf("    1フレームの持続時間[us]:%u\r\n", avi_main_header->dwMicroSecPerFrame);
  printf("    最大バイト毎秒[B/s]:%u\r\n", avi_main_header->dwMaxBytesPerSec);
  printf("    データのアライメント:%u\r\n", avi_main_header->dwPaddingGranularity);
  printf("    なんかのフラグ:0x%08x\r\n", avi_main_header->dwFlags);
  printf("    RIFF-AVIチャンクに含まれる総フレーム数:%u\r\n", avi_main_header->dwTotalFrames);
  printf("    開始フレーム:%u\r\n", avi_main_header->dwInitialFrames);
  printf("    ストリーム数:%u\r\n", avi_main_header->dwStreams);
  printf("    推奨バッファサイズ[B]:%u\r\n", avi_main_header->dwSuggestedBufferSize);
  printf("    動画の幅[ピクセル]:%u\r\n", avi_main_header->dwWidth);
  printf("    動画の高さ[ピクセル]:%u\r\n", avi_main_header->dwHeight);
  printf("    予約領域\r\n");
  //*/
  if(strncmp((char*)avi_main_header->fcc.fcc, "avih", sizeof(FOURCC)) != 0){
    printf("could not find avi main header\r\n");
    goto FILE_ERROR_PROCESS;
  }
  if(AVIF_HASINDEX != (avi_main_header->dwFlags & AVIF_HASINDEX)){
    printf("avi file does not have index\r\n");
    goto FILE_ERROR_PROCESS;
  }
  if((MATRIXLED_X_COUNT != avi_main_header->dwWidth) && (MATRIXLED_Y_COUNT != avi_main_header->dwHeight)){
    printf("wrong size avi file\r\n");
    goto FILE_ERROR_PROCESS;
  }
  /*
  if(2 != avi_main_header->dwStreams){
    printf("avi file has too few or too many streams\r\n");
    goto FILE_ERROR_PROCESS;
  }
  //*/

  avi_streams_count = avi_main_header->dwStreams;

  strl_list = (RIFFLIST *)malloc(sizeof(RIFFLIST));
  if(NULL == strl_list){
    printf("strl_list malloc error\r\n");
    goto OTHER_ERROR_PROCESS;
  }
  avi_stream_header = (AVISTREAMHEADER *)malloc(sizeof(AVISTREAMHEADER));
  if(NULL == avi_stream_header){
    printf("avi_stream_header malloc error\r\n");
    goto OTHER_ERROR_PROCESS;
  }
  strf_chunk = (RIFFCHUNK *)malloc(sizeof(RIFFCHUNK));
  if(NULL == strf_chunk){
    printf("strf_chunk malloc error\r\n");
    goto OTHER_ERROR_PROCESS;
  }
  bitmap_info_header = (BITMAPINFOHEADER *)malloc(sizeof(BITMAPINFOHEADER));
  if(NULL == bitmap_info_header){
    printf("bitmap_info_header malloc error\r\n");
    goto OTHER_ERROR_PROCESS;
  }
  wave_format_ex = (WAVEFORMATEX *)malloc(sizeof(WAVEFORMATEX));
  if(NULL == wave_format_ex){
    printf("wave_format_ex malloc error\r\n");
    goto OTHER_ERROR_PROCESS;
  }

  do{
    //AVIストリームリストの読み込み
    fatfs_result = f_read(&avi_fileobject, strl_list, sizeof(RIFFLIST), &read_data_byte_result);
    if(FR_OK != fatfs_result){
      printf("strl_list f_read NG fatfs_result=%d\r\n", fatfs_result);
      goto FATFS_ERROR_PROCESS;
    }
    /*
    printf("    strlリスト識別子:%.4s\r\n", strl_list->fcc.fcc);
    printf("    strlリストサイズ:%u\r\n", strl_list->cb);
    printf("    strlリストタイプ:%.4s\r\n", strl_list->fccListType.fcc);
    //*/

    if((strncmp((char*)strl_list->fcc.fcc, "LIST", sizeof(FOURCC)) == 0) && (strncmp((char*)strl_list->fccListType.fcc, "strl", sizeof(FOURCC)) == 0)){
      //AVIストリームヘッダの読み込み
      fatfs_result = f_read(&avi_fileobject, avi_stream_header, sizeof(AVISTREAMHEADER), &read_data_byte_result);
      if(FR_OK != fatfs_result){
        printf("avi_stream_header f_read NG fatfs_result=%d\r\n", fatfs_result);
        goto FATFS_ERROR_PROCESS;
      }
      /*
      printf("      strhチャンク識別子:%.4s\r\n", avi_stream_header->fcc.fcc);
      printf("      strhチャンクサイズ:%u\r\n", avi_stream_header->cb);
      printf("      ストリームデータタイプ:%.4s\r\n", avi_stream_header->fccType.fcc);
      printf("      指定コーデック:%.4s\r\n", avi_stream_header->fccHandler.fcc);
      printf("      なんかのフラグ:0x%08x\r\n", avi_stream_header->dwFlags);
      printf("      ストリームの優先順位:%u\r\n", avi_stream_header->wPriority);
      printf("      wLanguage 言語？:%u\r\n", avi_stream_header->wLanguage);
      printf("      ビデオフレームに対するオーディオデータの遅延フレーム数:%u\r\n", avi_stream_header->dwInitialFrames);
      printf("      サンプリングレートorフレームレート:%u\r\n", avi_stream_header->dwRate / avi_stream_header->dwScale);
      //printf("      :%u\r\n", avi_stream_header->dwRate);
      printf("      ストリームの開始時間:%u\r\n", avi_stream_header->dwStart);
      printf("      ストリームの長さ(総ビデオフレーム数or総オーディオデータ数):%u\r\n", avi_stream_header->dwLength);
      printf("      推奨バッファサイズ:%u\r\n", avi_stream_header->dwSuggestedBufferSize);
      printf("      データの品質:%u\r\n", avi_stream_header->dwQuality);
      printf("      1サンプルのサイズ:%u\r\n", avi_stream_header->dwSampleSize);
      printf("      転送先矩形位置 左:%u\r\n", avi_stream_header->rcFrame.left);
      printf("      転送先矩形位置 上:%u\r\n", avi_stream_header->rcFrame.top);
      printf("      転送先矩形位置 右:%u\r\n", avi_stream_header->rcFrame.right);
      printf("      転送先矩形位置 下:%u\r\n", avi_stream_header->rcFrame.bottom);
      //*/
      if(strncmp((char*)avi_stream_header->fcc.fcc, "strh", sizeof(FOURCC)) == 0){
        if((strncmp((char*)avi_stream_header->fccType.fcc, "vids", sizeof(FOURCC)) == 0) && (video_stream_find_flag == RESET)){
          if((avi_stream_header->dwFlags & (AVISF_DISABLED | AVISF_VIDEO_PALCHANGES)) == 0){
            if((MAX_VIDEO_FLAME_RATE >= (uint32_t)((float)avi_stream_header->dwRate / (float)avi_stream_header->dwScale)) && (MIN_VIDEO_FLAME_RATE <= ((float)avi_stream_header->dwRate / (float)avi_stream_header->dwScale))){
              //strfチャンクの読み込み
              fatfs_result = f_read(&avi_fileobject, strf_chunk, sizeof(RIFFCHUNK), &read_data_byte_result);
              if(FR_OK != fatfs_result){
                printf("strf_chunk f_read NG fatfs_result=%d\r\n", fatfs_result);
                goto FATFS_ERROR_PROCESS;
              }
              /*
              printf("      strfチャンク識別子:%.4s\r\n", strf_chunk->fcc.fcc);
              printf("      strfチャンクサイズ:%u\r\n", strf_chunk->cb);
              //*/
              if(strncmp((char*)strf_chunk->fcc.fcc, "strf", sizeof(FOURCC)) == 0){
                //ビデオストリームフォーマットの読み込み
                fatfs_result = f_read(&avi_fileobject, bitmap_info_header, sizeof(BITMAPINFOHEADER), &read_data_byte_result);
                if(FR_OK != fatfs_result){
                  printf("bitmap_info_header f_read NG fatfs_result=%d\r\n", fatfs_result);
                  goto FATFS_ERROR_PROCESS;
                }
                /*
                printf("      構造体の大きさ:%u\r\n", bitmap_info_header->biSize);
                printf("      ビットマップの幅:%u\r\n", bitmap_info_header->biWidth);
                printf("      ビットマップの高さ:%u\r\n", bitmap_info_header->biHeight);
                printf("      面の数:%u\r\n", bitmap_info_header->biPlanes);
                printf("      1ピクセル当たりのビット数:%u\r\n", bitmap_info_header->biBitCount);
                printf("      圧縮形式:0x%08x\r\n", bitmap_info_header->biCompression);
                printf("      イメージのサイズ:%u\r\n", bitmap_info_header->biSizeImage);
                printf("      水平1m当たりのピクセル:%u\r\n", bitmap_info_header->biXPelsPerMeter);
                printf("      垂直1m当たりのピクセル:%u\r\n", bitmap_info_header->biYPelsPerMeter);
                printf("      実際に使用するカラーインデックス数:%u\r\n", bitmap_info_header->biClrUsed);
                printf("      重要なカラーインデックス数:%u\r\n", bitmap_info_header->biClrImportant);
                //*/
                if((MATRIXLED_X_COUNT == bitmap_info_header->biWidth) && (MATRIXLED_Y_COUNT == bitmap_info_header->biHeight)){
                  if(BPP24 == bitmap_info_header->biBitCount){
                    if(BI_RGB == bitmap_info_header->biCompression){
                      //ここまできてAVIファイルのビデオストリームが正しく再生できると判定される
                      video_frame_rate = (float)avi_stream_header->dwRate / (float)avi_stream_header->dwScale;
                      video_length = avi_stream_header->dwLength;
                      snprintf((char*)chunk_name_temp, 5, "%02udb", strl_list_find_loop_count);
                      memmove(video_data_chunk_name.fcc, chunk_name_temp, sizeof(FOURCC));
                      video_stream_find_flag = SET;
                      //printf("video_strem read success\r\n");
                    }else{
                      printf("video flame is not BI_RGB\r\n");
                    }
                  }else{
                    printf("video flame is not 24bpp\r\n");
                  }
                }else{
                  printf("wrong size video stream\r\n");
                }

                fatfs_result = f_lseek(&avi_fileobject, f_tell(&avi_fileobject) - sizeof(BITMAPINFOHEADER));
                if(FR_OK != fatfs_result){
                  printf("bitmap_info_header f_lseek NG fatfs_result=%d\r\n", fatfs_result);
                  goto FATFS_ERROR_PROCESS;
                }
              }else{
                printf("could not find strf_chunk\r\n");
              }

              fatfs_result = f_lseek(&avi_fileobject, f_tell(&avi_fileobject) - sizeof(RIFFCHUNK));
              if(FR_OK != fatfs_result){
                printf("strf_chunk f_lseek NG fatfs_result=%d\r\n", fatfs_result);
                goto FATFS_ERROR_PROCESS;
              }
            }else{
              printf("video flame rate is out of range\r\n");
            }
          }else{
            printf("stream is disabled\r\n");
          }

        }else if((strncmp((char*)avi_stream_header->fccType.fcc, "auds", sizeof(FOURCC)) == 0)  && (audio_stream_find_flag == RESET)){
          if((avi_stream_header->dwFlags & AVISF_DISABLED) == 0){
            if((MAX_AUDIO_SAMPLE_RATE >= (avi_stream_header->dwRate / avi_stream_header->dwScale)) && (MIN_AUDIO_SAMPLE_RATE <= (avi_stream_header->dwRate / avi_stream_header->dwScale))){
              //strfチャンクの読み込み
              fatfs_result = f_read(&avi_fileobject, strf_chunk, sizeof(RIFFCHUNK), &read_data_byte_result);
              if(FR_OK != fatfs_result){
                printf("strf_chunk f_read NG fatfs_result=%d\r\n", fatfs_result);
                goto FATFS_ERROR_PROCESS;
              }
              /*
              printf("      strfチャンク識別子:%.4s\r\n", strf_chunk->fcc.fcc);
              printf("      strfチャンクサイズ:%u\r\n", strf_chunk->cb);
              //*/
              if(strncmp((char*)strf_chunk->fcc.fcc, "strf", sizeof(FOURCC)) == 0){
                //オーディオストリームフォーマットの読み込み
                fatfs_result = f_read(&avi_fileobject, wave_format_ex, sizeof(WAVEFORMATEX), &read_data_byte_result);
                if(FR_OK != fatfs_result){
                  printf("wave_format_ex f_read NG fatfs_result=%d\r\n", fatfs_result);
                  goto FATFS_ERROR_PROCESS;
                }
                /*
                printf("      波形フォーマットタイプ:0x%04x\r\n", wave_format_ex->wFormatTag);
                printf("      オーディオチャンネル数:%u\r\n", wave_format_ex->nChannels);
                printf("      サンプリング周波数:%u\r\n", wave_format_ex->nSamplesPerSec);
                printf("      平均データ転送レート:%u\r\n", wave_format_ex->nAvgBytesPerSec);
                printf("      ブロックアライメント:%u\r\n", wave_format_ex->nBlockAlign);
                printf("      1サンプル当たりのビット数:%u\r\n", wave_format_ex->wBitsPerSample);
                printf("      追加フォーマット情報のサイズ:%u\r\n", wave_format_ex->cbSize);
                //*/
                if(WAVE_FORMAT_PCM == wave_format_ex->wFormatTag){
                  if((MAX_AUDIO_CHANNEL >= wave_format_ex->nChannels) && (MIN_AUDIO_CHANNEL <= wave_format_ex->nChannels)){
                    if((MAX_AUDIO_SAMPLE_RATE >= wave_format_ex->nSamplesPerSec) && (MIN_AUDIO_SAMPLE_RATE <= wave_format_ex->nSamplesPerSec)){
                      if(16 == wave_format_ex->wBitsPerSample){
                        //ここまできてAVIファイルのオーディオストリームが正しく再生できると判定される
                        audio_channels = wave_format_ex->nChannels;
                        audio_sampling_rate = wave_format_ex->nSamplesPerSec;
                        audio_length = avi_stream_header->dwLength;
                        audio_suggest_buffer_size = avi_stream_header->dwSuggestedBufferSize;
                        snprintf((char*)chunk_name_temp, 5, "%02uwb", strl_list_find_loop_count);
                        memmove(audio_data_chunk_name.fcc, chunk_name_temp, sizeof(FOURCC));
                        audio_stream_find_flag = SET;
                        //printf("audio_strem read success\r\n");
                      }else{
                        printf("audio bits per sample is not 16bit\r\n");
                      }
                    }else{
                      printf("audio sample rate is out of range\r\n");
                    }
                  }else{
                    printf("audio channels is out of range\r\n");
                  }
                }else{
                  printf("audio format is not linearPCM\r\n");
                }

                fatfs_result = f_lseek(&avi_fileobject, f_tell(&avi_fileobject) - sizeof(WAVEFORMATEX));
                if(FR_OK != fatfs_result){
                  printf("bitmap_info_header f_lseek NG fatfs_result=%d\r\n", fatfs_result);
                  goto FATFS_ERROR_PROCESS;
                }
              }else{
                printf("could not find strf_chunk\r\n");
              }

              fatfs_result = f_lseek(&avi_fileobject, f_tell(&avi_fileobject) - sizeof(RIFFCHUNK));
              if(FR_OK != fatfs_result){
                printf("strf_chunk f_lseek NG fatfs_result=%d\r\n", fatfs_result);
                goto FATFS_ERROR_PROCESS;
              }
            }else{
              printf("audio sample rate is out of range\r\n");
            }
          }else{
            printf("stream is disabled\r\n");
          }

        }else{
          printf("stream is not video or audio\r\n");
        }

        //オーディオデータの推奨バッファサイズがおかしいときの処理
        if((audio_stream_find_flag == SET) &&
            (((wave_format_ex->nSamplesPerSec * wave_format_ex->nChannels * 2 / video_frame_rate) >= (audio_suggest_buffer_size + wave_format_ex->nBlockAlign * 2)) ||
            ((wave_format_ex->nSamplesPerSec * wave_format_ex->nChannels * 2 / video_frame_rate) <= (audio_suggest_buffer_size - wave_format_ex->nBlockAlign * 2)))){
          audio_stream_find_flag = RESET;
          printf("audio buffer size is wrong\r\n");
        }

      }else{
        printf("wrong strh header\r\n");
      }

      fatfs_result = f_lseek(&avi_fileobject, f_tell(&avi_fileobject) - sizeof(AVISTREAMHEADER));
      if(FR_OK != fatfs_result){
        printf("avi_stream_header f_lseek NG fatfs_result=%d\r\n", fatfs_result);
        goto FATFS_ERROR_PROCESS;
      }
      strl_list_find_loop_count++;
    }else{
      printf("could not find strl list\r\n");
    }

    fatfs_result = f_lseek(&avi_fileobject, f_tell(&avi_fileobject) + strl_list->cb - sizeof(FOURCC));
    if(FR_OK != fatfs_result){
      printf("avi_stream_header f_lseek NG fatfs_result=%d\r\n", fatfs_result);
      goto FATFS_ERROR_PROCESS;
    }

    if(f_tell(&avi_fileobject) >= (riff_chunk->cb + sizeof(RIFFCHUNK) - 1)){
      printf("could not find playable audio video stream header\r\n");
      goto FILE_ERROR_PROCESS;
    }

  } while((video_stream_find_flag != SET) || (audio_stream_find_flag != SET));

  //printf("read video audio header success\r\n");
  //printf("%.4s\r\n", video_data_chunk_name.fcc);
  //printf("%.4s\r\n", audio_data_chunk_name.fcc);

  unknown_list = (RIFFLIST *)malloc(sizeof(RIFFLIST));
  if(NULL == unknown_list){
    printf("unknown_list malloc error\r\n");
    goto OTHER_ERROR_PROCESS;
  }
  //moviリスト探索
  do{
    fatfs_result = f_read(&avi_fileobject, unknown_list, sizeof(RIFFLIST), &read_data_byte_result);
    if(FR_OK != fatfs_result){
      printf("unknown_list f_read NG fatfs_result=%d\r\n", fatfs_result);
      goto FATFS_ERROR_PROCESS;
    }
    /*
    printf("    不明なリスト識別子:%.4s\r\n", unknown_list->fcc.fcc);
    printf("    不明なリストサイズ:%u\r\n", unknown_list->cb);
    printf("    不明なリストタイプ:%.4s\r\n", unknown_list->fccListType.fcc);
    //*/
    if((strncmp((char*)unknown_list->fcc.fcc, "LIST", sizeof(FOURCC)) == 0) && (strncmp((char*)unknown_list->fccListType.fcc, "movi", sizeof(FOURCC)) == 0)){
      fatfs_result = f_lseek(&avi_fileobject, f_tell(&avi_fileobject) - sizeof(RIFFLIST));
      if(FR_OK != fatfs_result){
        printf("unknown_list f_lseek NG fatfs_result=%d\r\n", fatfs_result);
        goto FATFS_ERROR_PROCESS;
      }

    }else{
      fatfs_result = f_lseek(&avi_fileobject, f_tell(&avi_fileobject) + unknown_list->cb - sizeof(FOURCC));
      if(FR_OK != fatfs_result){
        printf("unknown_list f_lseek NG fatfs_result=%d\r\n", fatfs_result);
        goto FATFS_ERROR_PROCESS;
      }
    }

    if(f_tell(&avi_fileobject) >= (riff_chunk->cb + sizeof(RIFFCHUNK) - 1)){
      printf("could not find movi list\r\n");
      goto FILE_ERROR_PROCESS;
    }

  } while(strncmp((char*)unknown_list->fccListType.fcc, "movi", sizeof(FOURCC)) != 0);
  //printf("movi list found\r\n");

  movi_list = (RIFFLIST *)malloc(sizeof(RIFFLIST));
  if(NULL == movi_list){
    printf("movi_list malloc error\r\n");
    goto OTHER_ERROR_PROCESS;
  }

  //moviリストの読み込み
  fatfs_result = f_read(&avi_fileobject, movi_list, sizeof(RIFFLIST), &read_data_byte_result);
  if(FR_OK != fatfs_result){
    printf("movi_list f_read NG fatfs_result=%d\r\n", fatfs_result);
    goto FATFS_ERROR_PROCESS;
  }
  /*
  printf("  moviリスト識別子:%.4s\r\n", movi_list->fcc.fcc);
  printf("  moviリストサイズ:%u\r\n", movi_list->cb);
  printf("  moviリストタイプ:%.4s\r\n", movi_list->fccListType.fcc);
  //*/

  //moviリストの先頭アドレス記憶する　idx1チャンクの先頭アドレスも記録する
  movi_list_position = f_tell(&avi_fileobject) - sizeof(FOURCC);

  fatfs_result = f_lseek(&avi_fileobject, f_tell(&avi_fileobject) + movi_list->cb - sizeof(FOURCC));
  if(FR_OK != fatfs_result){
    printf("movi_list f_lseek NG fatfs_result=%d\r\n", fatfs_result);
    goto FATFS_ERROR_PROCESS;
  }

  idx1_chunk = (RIFFCHUNK *)malloc(sizeof(RIFFCHUNK));
  if(NULL == idx1_chunk){
    printf("idx1_chunk malloc error\r\n");
    goto OTHER_ERROR_PROCESS;
  }
  //idx1チャンクの読み込み
  fatfs_result = f_read(&avi_fileobject, idx1_chunk, sizeof(RIFFCHUNK), &read_data_byte_result);
  if(FR_OK != fatfs_result){
    printf("idx1_chunk f_read NG fatfs_result=%d\r\n", fatfs_result);
    goto FATFS_ERROR_PROCESS;
  }
  /*
  printf("  idx1チャンク識別子:%.4s\r\n", idx1_chunk->fcc.fcc);
  printf("  idx1チャンクサイズ:%u\r\n", idx1_chunk->cb);
  //*/
  avi_old_index_position = f_tell(&avi_fileobject);
  avi_old_index_size = idx1_chunk->cb;


  play_info->avi_info.video_frame_rate = video_frame_rate;
  play_info->avi_info.video_length = video_length;
  memmove(play_info->avi_info.video_data_chunk_name.fcc, video_data_chunk_name.fcc, sizeof(FOURCC));
  play_info->avi_info.audio_channels = audio_channels;
  play_info->avi_info.audio_sampling_rate = audio_sampling_rate;
  play_info->avi_info.audio_length = audio_length;
  memmove(play_info->avi_info.audio_data_chunk_name.fcc, audio_data_chunk_name.fcc, sizeof(FOURCC));
  play_info->avi_info.avi_streams_count = avi_streams_count;
  play_info->avi_info.movi_list_position = movi_list_position;
  play_info->avi_info.avi_old_index_position = avi_old_index_position;
  play_info->avi_info.avi_old_index_size = avi_old_index_size;
  play_info->avi_info.avi_file_size = avi_file_size;

  //printf("\r\n");
  /*
  printf("映像フレームレート:%6.3fFPS\r\n", video_frame_rate);
  printf("映像長さ:%luフレーム\r\n", video_length);
  printf("映像データチャンク名:%.4s\r\n", video_data_chunk_name.fcc);
  printf("音声チャンネル数:%uチャンネル\r\n", audio_channels);
  printf("音声サンプリング周波数:%luHz\r\n", audio_sampling_rate);
  printf("音声長さ:%luサンプル\r\n", audio_length);
  printf("音声データチャンク名:%.4s\r\n", audio_data_chunk_name.fcc);
  printf("ストリーム数:%lu\r\n", avi_streams_count);
  printf("moviリスト位置:0x%08lx\r\n", movi_list_position);
  printf("idx1インデックス位置:0x%08lx\r\n", avi_old_index_position);
  printf("idx1インデックス長さ:0x%08lx\r\n", avi_old_index_size);
  printf("AVIファイルサイズ:%luB\r\n", avi_file_size);
  printf("\r\n");
  //*/

  f_close(&avi_fileobject);
  free(riff_chunk);
  free(riff_list);
  free(four_cc);
  free(avi_main_header);
  free(strl_list);
  free(avi_stream_header);
  free(strf_chunk);
  free(bitmap_info_header);
  free(wave_format_ex);
  free(unknown_list);
  free(movi_list);
  free(idx1_chunk);
  //free(avi_old_index);
  return FILE_OK;

  FILE_ERROR_PROCESS:
  printf("\r\n");
  f_close(&avi_fileobject);
  free(riff_chunk);
  free(riff_list);
  free(four_cc);
  free(avi_main_header);
  free(strl_list);
  free(avi_stream_header);
  free(strf_chunk);
  free(bitmap_info_header);
  free(wave_format_ex);
  free(unknown_list);
  free(movi_list);
  free(idx1_chunk);
  //free(avi_old_index);
  return FILE_ERROR;

  FATFS_ERROR_PROCESS:
  printf("\r\n");
  f_close(&avi_fileobject);
  free(riff_chunk);
  free(riff_list);
  free(four_cc);
  free(avi_main_header);
  free(strl_list);
  free(avi_stream_header);
  free(strf_chunk);
  free(bitmap_info_header);
  free(wave_format_ex);
  free(unknown_list);
  free(movi_list);
  free(idx1_chunk);
  //free(avi_old_index);
  return FATFS_ERROR;

  OTHER_ERROR_PROCESS:
  printf("\r\n");
  f_close(&avi_fileobject);
  free(riff_chunk);
  free(riff_list);
  free(four_cc);
  free(avi_main_header);
  free(strl_list);
  free(avi_stream_header);
  free(strf_chunk);
  free(bitmap_info_header);
  free(wave_format_ex);
  free(unknown_list);
  free(movi_list);
  free(idx1_chunk);
  //free(avi_old_index);
  return OTHER_ERROR;
}

READ_FILE_RESULT read_avi_stream(PLAY_INFO *play_info, uint32_t read_frame_count){
  static FIL avi_fileobject;
  FRESULT fatfs_result;
  UINT read_data_byte_result;
  static uint32_t linkmap_table[LINKMAP_TABLE_SIZE];

  aIndex avi_old_index_0, avi_old_index_1;
  CHUNKHEADER stream_chunk_header;

  uint32_t read_frame_count_temp;
  uint32_t idx1_search_loop;

  static int8_t previous_filename[_MAX_LFN];

  //フレーム数範囲外の処理
  if(play_info->avi_info.video_length < read_frame_count){
    read_frame_count_temp = play_info->avi_info.video_length - 1;
  }else{
    read_frame_count_temp = read_frame_count;
  }

  //ファイル名が変更されたときの処理
  if(strcmp((char*)play_info->file_name, (char*)previous_filename) != 0){
    __HAL_TIM_DISABLE(&htim6);
    memset(Audio_Buffer, 0, (2 * MAX_AUDIO_SAMPLE_RATE * MAX_AUDIO_CHANNEL / MIN_VIDEO_FLAME_RATE));

    fatfs_result = f_close(&avi_fileobject);

    fatfs_result = f_open(&avi_fileobject, (TCHAR*)play_info->file_name, FA_READ);
    if(FR_OK != fatfs_result){
      printf("read_avi_stream f_open NG fatfs_result=%d\r\n", fatfs_result);
      goto FATFS_ERROR_PROCESS;
    }

    avi_fileobject.cltbl = linkmap_table;
    linkmap_table[0] = LINKMAP_TABLE_SIZE;
    fatfs_result = f_lseek(&avi_fileobject, CREATE_LINKMAP);
    if(FR_OK != fatfs_result){
      printf("read_avi_header create linkmap table NG fatfs_result=%d\r\n", fatfs_result);
      printf("need linkmap table size %lu\r\n", linkmap_table[0]);
      goto FATFS_ERROR_PROCESS;
    }

    SET_AUDIO_SAMPLERATE(play_info->avi_info.audio_sampling_rate);
    Audio_Channnel_Count = play_info->avi_info.audio_channels;
  }

  //インデックス読み込みループ
  for(idx1_search_loop = 0;idx1_search_loop < play_info->avi_info.avi_streams_count;idx1_search_loop++){

    //指定フレームのインデックス読み込み
    fatfs_result = f_lseek(&avi_fileobject, (play_info->avi_info.avi_old_index_position + sizeof(aIndex) * (play_info->avi_info.avi_streams_count * read_frame_count_temp + idx1_search_loop)));
    if(FR_OK != fatfs_result){
      printf("read_index f_lseek NG fatfs_result=%d\r\n", fatfs_result);
      goto FATFS_ERROR_PROCESS;
    }
    fatfs_result = f_read(&avi_fileobject, &avi_old_index_0, sizeof(aIndex), &read_data_byte_result);
    if(FR_OK != fatfs_result){
      printf("avi_old_index f_read NG fatfs_result=%d\r\n", fatfs_result);
      goto FATFS_ERROR_PROCESS;
    }

    //インデックスが映像だった場合にフレームを読み込む
    if(strncmp((char*)avi_old_index_0.dwChunkId.fcc, (char*)play_info->avi_info.video_data_chunk_name.fcc, sizeof(FOURCC)) == 0){
      fatfs_result = f_lseek(&avi_fileobject, (avi_old_index_0.dwOffset + play_info->avi_info.movi_list_position));
      if(FR_OK != fatfs_result){
        printf("read video flame chunk f_lseek NG fatfs_result=%d\r\n", fatfs_result);
        goto FATFS_ERROR_PROCESS;
      }
      fatfs_result = f_read(&avi_fileobject, &stream_chunk_header, sizeof(CHUNKHEADER), &read_data_byte_result);
      if(FR_OK != fatfs_result){
        printf("read video flame chunk f_read NG fatfs_result=%d\r\n", fatfs_result);
        goto FATFS_ERROR_PROCESS;
      }
      /*
      printf("映像フレームチャンクID:%.4s\r\n", stream_chunk_header.chunkID);
      printf("映像フレームチャンクサイズ:%u\r\n", stream_chunk_header.chunkSize);
      printf("\r\n");
      //*/
      if(strncmp((char*)stream_chunk_header.chunkID , (char*)play_info->avi_info.video_data_chunk_name.fcc, sizeof(FOURCC)) == 0){
        fatfs_result = f_read(&avi_fileobject, Flame_Buffer, (MATRIXLED_Y_COUNT * MATRIXLED_X_COUNT * MATRIXLED_COLOR_COUNT), &read_data_byte_result);
        if(FR_OK != fatfs_result){
          printf("read video flame f_read NG fatfs_result=%d\r\n", fatfs_result);
          goto FATFS_ERROR_PROCESS;
        }
      }else{
        printf("video flame chunk id is wrong\r\n");
      }

    }

    //指定フレームの時の音声バッファサイズを使用
    if(strncmp((char*)avi_old_index_0.dwChunkId.fcc, (char*)play_info->avi_info.audio_data_chunk_name.fcc, sizeof(FOURCC)) == 0){
      Audio_Flame_Data_Count = avi_old_index_0.dwSize;
    }

    //音声読み込み 0フレームの時
    if(0 == read_frame_count_temp){
      if(strncmp((char*)avi_old_index_0.dwChunkId.fcc, (char*)play_info->avi_info.audio_data_chunk_name.fcc, sizeof(FOURCC)) == 0){
        fatfs_result = f_lseek(&avi_fileobject, (avi_old_index_0.dwOffset + play_info->avi_info.movi_list_position));
        if(FR_OK != fatfs_result){
          printf("read audio data chunk f_lseek NG fatfs_result=%d\r\n", fatfs_result);
          goto FATFS_ERROR_PROCESS;
        }
        fatfs_result = f_read(&avi_fileobject, &stream_chunk_header, sizeof(CHUNKHEADER), &read_data_byte_result);
        if(FR_OK != fatfs_result){
          printf("read audio data chunk f_read NG fatfs_result=%d\r\n", fatfs_result);
          goto FATFS_ERROR_PROCESS;
        }
        /*
        printf("音声データ0チャンクID:%.4s\r\n", stream_chunk_header.chunkID);
        printf("音声データ0チャンクサイズ:%u\r\n", stream_chunk_header.chunkSize);
        printf("\r\n");
        //*/
        if(strncmp((char*)stream_chunk_header.chunkID , (char*)play_info->avi_info.audio_data_chunk_name.fcc, sizeof(FOURCC)) == 0){
          fatfs_result = f_read(&avi_fileobject, &Audio_Buffer[0][0], stream_chunk_header.chunkSize, &read_data_byte_result);
          if(FR_OK != fatfs_result){
            printf("read video flame f_read NG fatfs_result=%d\r\n", fatfs_result);
            goto FATFS_ERROR_PROCESS;
          }
          Audio_Double_Buffer = 0;
        }else{
          printf("audio data chunk id is wrong\r\n");
        }
      }

      //指定フレーム+1のインデックス読み込み
      fatfs_result = f_lseek(&avi_fileobject, (play_info->avi_info.avi_old_index_position + sizeof(aIndex) * (play_info->avi_info.avi_streams_count * (read_frame_count_temp + 1) + idx1_search_loop)));
      if(FR_OK != fatfs_result){
        printf("read_index f_lseek NG fatfs_result=%d\r\n", fatfs_result);
        goto FATFS_ERROR_PROCESS;
      }
      fatfs_result = f_read(&avi_fileobject, &avi_old_index_1, sizeof(aIndex), &read_data_byte_result);
      if(FR_OK != fatfs_result){
        printf("avi_old_index f_read NG fatfs_result=%d\r\n", fatfs_result);
        goto FATFS_ERROR_PROCESS;
      }

      if(strncmp((char*)avi_old_index_1.dwChunkId.fcc, (char*)play_info->avi_info.audio_data_chunk_name.fcc, sizeof(FOURCC)) == 0){
        fatfs_result = f_lseek(&avi_fileobject, (avi_old_index_1.dwOffset + play_info->avi_info.movi_list_position));
        if(FR_OK != fatfs_result){
          printf("read audio data chunk f_lseek NG fatfs_result=%d\r\n", fatfs_result);
          goto FATFS_ERROR_PROCESS;
        }
        fatfs_result = f_read(&avi_fileobject, &stream_chunk_header, sizeof(CHUNKHEADER), &read_data_byte_result);
        if(FR_OK != fatfs_result){
          printf("read audio data chunk f_read NG fatfs_result=%d\r\n", fatfs_result);
          goto FATFS_ERROR_PROCESS;
        }
        /*
        printf("音声データ1チャンクID:%.4s\r\n", stream_chunk_header.chunkID);
        printf("音声データ1チャンクサイズ:%u\r\n", stream_chunk_header.chunkSize);
        printf("\r\n");
        //*/
        if(strncmp((char*)stream_chunk_header.chunkID , (char*)play_info->avi_info.audio_data_chunk_name.fcc, sizeof(FOURCC)) == 0){
          fatfs_result = f_read(&avi_fileobject, &Audio_Buffer[1][0], stream_chunk_header.chunkSize, &read_data_byte_result);
          if(FR_OK != fatfs_result){
            printf("read video flame f_read NG fatfs_result=%d\r\n", fatfs_result);
            goto FATFS_ERROR_PROCESS;
          }
        }else{
          printf("audio data chunk id is wrong\r\n");
        }
      }

    }else if((play_info->avi_info.video_length - 1) <= read_frame_count_temp){
      memset(Audio_Buffer, 0, (2 * MAX_AUDIO_SAMPLE_RATE * MAX_AUDIO_CHANNEL / MIN_VIDEO_FLAME_RATE));

    }else{
      //指定フレーム+1のインデックス読み込み
      fatfs_result = f_lseek(&avi_fileobject, (play_info->avi_info.avi_old_index_position + sizeof(aIndex) * (play_info->avi_info.avi_streams_count * (read_frame_count_temp + 1) + idx1_search_loop)));
      if(FR_OK != fatfs_result){
        printf("read_index f_lseek NG fatfs_result=%d\r\n", fatfs_result);
        goto FATFS_ERROR_PROCESS;
      }
      fatfs_result = f_read(&avi_fileobject, &avi_old_index_1, sizeof(aIndex), &read_data_byte_result);
      if(FR_OK != fatfs_result){
        printf("avi_old_index f_read NG fatfs_result=%d\r\n", fatfs_result);
        goto FATFS_ERROR_PROCESS;
      }

      if(strncmp((char*)avi_old_index_1.dwChunkId.fcc, (char*)play_info->avi_info.audio_data_chunk_name.fcc, sizeof(FOURCC)) == 0){
        fatfs_result = f_lseek(&avi_fileobject, (avi_old_index_1.dwOffset + play_info->avi_info.movi_list_position));
        if(FR_OK != fatfs_result){
          printf("read audio data chunk f_lseek NG fatfs_result=%d\r\n", fatfs_result);
          goto FATFS_ERROR_PROCESS;
        }
        fatfs_result = f_read(&avi_fileobject, &stream_chunk_header, sizeof(CHUNKHEADER), &read_data_byte_result);
        if(FR_OK != fatfs_result){
          printf("read audio data chunk f_read NG fatfs_result=%d\r\n", fatfs_result);
          goto FATFS_ERROR_PROCESS;
        }
        /*
        printf("音声データ1チャンクID:%.4s\r\n", stream_chunk_header.chunkID);
        printf("音声データ1チャンクサイズ:%u\r\n", stream_chunk_header.chunkSize);
        printf("\r\n");
        //*/
        if(strncmp((char*)stream_chunk_header.chunkID , (char*)play_info->avi_info.audio_data_chunk_name.fcc, sizeof(FOURCC)) == 0){
          fatfs_result = f_read(&avi_fileobject, &Audio_Buffer[~Audio_Double_Buffer & 0x01][0], stream_chunk_header.chunkSize, &read_data_byte_result);
          if(FR_OK != fatfs_result){
            printf("read video flame f_read NG fatfs_result=%d\r\n", fatfs_result);
            goto FATFS_ERROR_PROCESS;
          }
        }else{
          printf("audio data chunk id is wrong\r\n");
        }
      }
    }
  }

  __HAL_TIM_ENABLE(&htim6);
  memmove(previous_filename, play_info->file_name, _MAX_LFN);
  return FILE_OK;

  FATFS_ERROR_PROCESS:
  snprintf((char*)previous_filename, _MAX_LFN, "*");

  return FATFS_ERROR;
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
  static uint32_t IR_modulation_unit_CCR;
  static uint8_t repeat_flag;
  static uint32_t IR_receive_data_temp;
  uint32_t tim_ccr_value;

  if(htim->Instance == TIM14){
    __HAL_TIM_SET_COUNTER(&htim14, 0);
    tim_ccr_value = HAL_TIM_ReadCapturedValue(&htim14, TIM_CHANNEL_1);
    if((IR_Receive_Count == 0) && (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_RESET)){
      IR_modulation_unit_CCR = 0;
      repeat_flag = RESET;
      IR_receive_data_temp = 0;
      IR_Receive_Count++;
    }else if((IR_Receive_Count == 1) && (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_SET)){
      IR_modulation_unit_CCR = tim_ccr_value / 16;
      IR_Receive_Count++;
    }else if((IR_Receive_Count == 2) && (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_RESET)){
      if(((IR_modulation_unit_CCR - IR_MODULATION_UNIT_TOLERANCE) < (tim_ccr_value / 8)) &&
         ((IR_modulation_unit_CCR + IR_MODULATION_UNIT_TOLERANCE) > (tim_ccr_value / 8))){
        repeat_flag = RESET;
        IR_Receive_Data = 0;
        IR_Receive_Count++;
      }else if(((IR_modulation_unit_CCR - IR_MODULATION_UNIT_TOLERANCE) < (tim_ccr_value / 4)) &&
                 ((IR_modulation_unit_CCR + IR_MODULATION_UNIT_TOLERANCE) > (tim_ccr_value / 4))){
        repeat_flag = SET;
        IR_Receive_Count++;
      }else{
        IR_Receive_Data = 0;
        IR_Receive_Count = 0;
      }
    }else if(IR_Receive_Count >= 3){
      if((repeat_flag == RESET) && (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_RESET)){
        if(((IR_modulation_unit_CCR - IR_MODULATION_UNIT_TOLERANCE) < tim_ccr_value) &&
           ((IR_modulation_unit_CCR + IR_MODULATION_UNIT_TOLERANCE) > tim_ccr_value)){
          IR_Receive_Count++;
        }else if(((IR_modulation_unit_CCR - IR_MODULATION_UNIT_TOLERANCE) < (tim_ccr_value / 3)) &&
                 ((IR_modulation_unit_CCR + IR_MODULATION_UNIT_TOLERANCE) > (tim_ccr_value / 3))){
          IR_receive_data_temp |= 0x01 << (IR_Receive_Count - 3);
          IR_Receive_Count++;
        }else{
          IR_Receive_Data = 0;
          IR_Receive_Count = 0;
        }
        if(IR_Receive_Count >= 35){
          IR_Receive_Data = IR_receive_data_temp;
          IR_Receive_Count = 0;
        }
      }else if((repeat_flag == SET) && (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_SET)){
        IR_Receive_Count = 0;
      }
    }
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  static unsigned int audio_data_output_count;

  if(htim->Instance == TIM14){
    IR_Receive_Data = 0;
    IR_Receive_Count = 0;
  }

  if(htim->Instance == TIM6){
    if((Audio_End_Flag == RESET) && (Status == PLAY)){
      if(Audio_Channnel_Count == 1){
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_L, ((unsigned int)((Audio_Buffer[Audio_Double_Buffer & 0x01][audio_data_output_count] * Volume_Value_float) + 0x8000) & 0x0000fff0));
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_L, ((unsigned int)((Audio_Buffer[Audio_Double_Buffer & 0x01][audio_data_output_count] * Volume_Value_float) + 0x8000) & 0x0000fff0));
        if(audio_data_output_count < ((Audio_Flame_Data_Count >> 1) - 1)){
          audio_data_output_count++;
        }else{
          Audio_Flame_End_flag = SET;
          if(Audio_Double_Buffer == 0){
            Audio_Double_Buffer = 1;
          }else{
            Audio_Double_Buffer = 0;
          }
          audio_data_output_count = 0;
        }
      }else if(Audio_Channnel_Count == 2){
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_L, ((unsigned int)((Audio_Buffer[Audio_Double_Buffer & 0x01][audio_data_output_count + 0] * Volume_Value_float) + 0x8000) & 0x0000fff0));
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_L, ((unsigned int)((Audio_Buffer[Audio_Double_Buffer & 0x01][audio_data_output_count + 1] * Volume_Value_float) + 0x8000) & 0x0000fff0));
        if(audio_data_output_count < ((Audio_Flame_Data_Count >> 1) - 2)){
          audio_data_output_count += 2;
        }else{
          Audio_Flame_End_flag = SET;
          if(Audio_Double_Buffer == 0){
            Audio_Double_Buffer = 1;
          }else{
            Audio_Double_Buffer = 0;
          }
          audio_data_output_count = 0;
        }
      }
    }else{
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_L, (0x8000 & 0x0000fff0));
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_L, (0x8000 & 0x0000fff0));
      if(Audio_Channnel_Count == 1){
        if(audio_data_output_count < ((Audio_Flame_Data_Count >> 1) - 1)){
          audio_data_output_count++;
        }else{
          Audio_Flame_End_flag = SET;
          Audio_Double_Buffer = 0;
          audio_data_output_count = 0;
        }
      }else if(Audio_Channnel_Count == 2){
        if(audio_data_output_count < ((Audio_Flame_Data_Count >> 1) - 2)){
          audio_data_output_count += 2;
        }else{
          Audio_Flame_End_flag = SET;
          Audio_Double_Buffer = 0;
          audio_data_output_count = 0;
        }
      }
    }
  }
}

void send_command_led_display(unsigned char command_number, unsigned char command_1, unsigned char command_2, unsigned char command_3){
  unsigned char led_display_command[SPI_SEND_COMMAND_COUNT];

  led_display_command[0] = command_number;
  led_display_command[1] = command_1;
  led_display_command[2] = command_2;
  led_display_command[3] = command_3;

  if(HAL_GPIO_ReadPin(OUT_SPI_DC_SELECT_GPIO_Port, OUT_SPI_DC_SELECT_Pin) != GPIO_PIN_SET){
    HAL_GPIO_WritePin(OUT_SPI_DC_SELECT_GPIO_Port, OUT_SPI_DC_SELECT_Pin, GPIO_PIN_SET);
    for(volatile uint32_t loop = 0;loop < SPI_DELAY_TIME_1;loop++){asm volatile("nop");}
  }
  HAL_SPI_Transmit(&hspi1, led_display_command, SPI_SEND_COMMAND_COUNT, 1000);
  for(volatile uint32_t loop = 0;loop < SPI_DELAY_TIME_0;loop++){asm volatile("nop");}
}

void send_data_led_display(void){
  static unsigned char prev_pwm_res, prev_gamma;

  SPI_DMA_Send_Data_Count = 0;
  send_command_led_display(0x00, 0, MATRIXLED_X_COUNT - 1, 0x00);
  send_command_led_display(0x02, 0, 63, 0x00);
  send_command_led_display(0x04, Display_ON_OFF, 0x01, Display_Brightness);
  send_command_led_display(0x08, 2, 0, 1);
  if((Pwm_Res != prev_pwm_res) ||(Gamma != prev_gamma)){
    //send_command_led_display(0x08, 2, 0, 1);
    send_command_led_display(0x05, Pwm_Res, Gamma, 0x00);
    HAL_Delay(2);
    send_command_led_display(0x06, 255, 200, 255);
    HAL_Delay(1);
  }
  HAL_GPIO_WritePin(OUT_SPI_DC_SELECT_GPIO_Port, OUT_SPI_DC_SELECT_Pin, GPIO_PIN_RESET);
  for(volatile uint32_t loop = 0;loop < SPI_DELAY_TIME_0;loop++){asm volatile("nop");}
  HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)Flame_Buffer, MATRIXLED_Y_COUNT * MATRIXLED_X_COUNT * MATRIXLED_COLOR_COUNT >> 1);

  prev_pwm_res = Pwm_Res;
  prev_gamma = Gamma;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
  if(hspi->Instance == SPI1){
    SPI_DMA_Send_Data_Count++;

    switch (SPI_DMA_Send_Data_Count){
    case 1:
      send_command_led_display(0x02, 64, 127, 0x00);
      HAL_GPIO_WritePin(OUT_SPI_DC_SELECT_GPIO_Port, OUT_SPI_DC_SELECT_Pin, GPIO_PIN_RESET);
      for(volatile uint32_t loop = 0;loop < SPI_DELAY_TIME_0;loop++){asm volatile("nop");}
      HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)(&Flame_Buffer[64][0][0]), MATRIXLED_Y_COUNT * MATRIXLED_X_COUNT * MATRIXLED_COLOR_COUNT >> 1);
      break;

    case 2:
      send_command_led_display(0x07, Buffer_Select, (~Buffer_Select) & 0x01, 0x00);
      Buffer_Select = (~Buffer_Select) & 0x01;
      break;

    default:
      SPI_DMA_Send_Data_Count = 0;
      break;
    }
  }
}

void pop_noise_reduction(void){
  unsigned int loop0, loop1;

  for(loop0 = 0;loop0 <= 0x8000;loop0++){
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_L, (loop0 & 0x0000fff0));
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_L, (loop0 & 0x0000fff0));
    for(loop1 = 0;loop1 < 250;loop1++){asm volatile("nop");}
  }
}

/*
void send_data_led_display(void){

  //while(SPI_DMA_Complete_Flag == RESET);
  //SPI_DMA_Complete_Flag = RESET;
  SPI_DMA_Complete_Flag = SET;
  send_command_led_display(0x02, 0, 63, 0x00);
  SPI_DMA_Complete_Flag = RESET;

  HAL_GPIO_WritePin(OUT_SPI_DC_SELECT_GPIO_Port, OUT_SPI_DC_SELECT_Pin, GPIO_PIN_RESET);
  for(uint16_t loop = 0;loop < 250;loop++){asm("nop");}
  HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)Flame_Buffer, MATRIXLED_Y_COUNT * MATRIXLED_X_COUNT * MATRIXLED_COLOR_COUNT >> 1);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
  if(hspi->Instance == SPI1){
    if(SPI_DMA_Complete_Flag == RESET){
      SPI_DMA_Complete_Flag = SET;

      send_command_led_display(0x02, 64, 127, 0x00);

      HAL_GPIO_WritePin(OUT_SPI_DC_SELECT_GPIO_Port, OUT_SPI_DC_SELECT_Pin, GPIO_PIN_RESET);
      for(uint16_t loop = 0;loop < 250;loop++){asm("nop");}
      HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)(&Flame_Buffer[64][0][0]), MATRIXLED_Y_COUNT * MATRIXLED_X_COUNT * MATRIXLED_COLOR_COUNT >> 1);
    }
  }
}*/

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
