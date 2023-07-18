

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//#include "platform.h"
//#include "display.h"
#include <USB-MIDI.h>
#include <Multi_BitBang.h>
#include <Multi_OLED.h>

#define NUM_DISPLAYS 19
#define NUM_BUSES 19
#define SCL_PIN 29

// I2C bus info
uint8_t scl_list[NUM_BUSES] = {SCL_PIN,SCL_PIN,SCL_PIN,SCL_PIN,SCL_PIN,SCL_PIN,SCL_PIN,SCL_PIN,SCL_PIN,SCL_PIN,SCL_PIN,SCL_PIN,SCL_PIN,SCL_PIN,SCL_PIN,SCL_PIN,SCL_PIN,SCL_PIN,SCL_PIN};// 
uint8_t sda_list[NUM_BUSES] = {0, 1, 2, 3, 4, 5, 6,7,8,9,10,11,12,13,24,25,26,27, 28}; 
int32_t speed_list[NUM_BUSES] = {800000L, 800000L, 800000L, 800000L, 800000L, 800000L, 800000L, 800000L, 800000L, 800000L, 800000L, 800000L, 800000L, 800000L, 800000L, 800000L, 800000L, 800000L, 800000L};
// OLED display info
uint8_t bus_list[NUM_DISPLAYS] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
uint8_t addr_list[NUM_DISPLAYS] = {0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c};
uint8_t type_list[NUM_DISPLAYS] = {OLED_128x64, OLED_128x64, OLED_128x64, OLED_128x64, OLED_128x64,OLED_128x64,OLED_128x64,OLED_128x64,OLED_128x64,OLED_128x64,OLED_128x64,OLED_128x64,OLED_128x64,OLED_128x64,OLED_128x64,OLED_128x64,OLED_128x64,OLED_128x64,OLED_128x64};
uint8_t flip_list[NUM_DISPLAYS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t invert_list[NUM_DISPLAYS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

                  
USBMIDI_CREATE_DEFAULT_INSTANCE();

unsigned long t0 = millis();

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


char logic_peak_levels[8] = { 0 };
char logic_peak_offset[8] = { 0 };

char logic_control_strip[2][8][7] = { 0 };

//const unsigned reaperMessSize = 300;
const byte dispLineLengthLargeFont = 8;
const unsigned reaperMessSize = 790;
const byte dispLineLength = 16;
char dispLineBuf[dispLineLength];

const int sysexHeaderSize = 6;

const byte messBufSize = 400; //some extra bytes for safety
char messBuf[messBufSize];
unsigned messBufPos = 0;
bool updateSpacerTop = true;
bool updateSpacerBottom = true;

int i = 0;
int j = 0;

bool previous_m_chan[NUM_DISPLAYS];

char dispCompareBuf[NUM_DISPLAYS][2][dispLineLength]; //[nrOfLCD][line][xPos]

void substituteSpacer()
{
  for (int i = 0; i < dispLineLength; i++)
    if (dispLineBuf[i] == 0x7C)
    {
      dispLineBuf[i] = 199;
      j++;
    }
}

void printSpacer(int dispNr, bool updateSpacerTop, bool updateSpacerBottom)
{
          //print spacer
        char spacerBuf[2] = {186, 0};

        if (dispNr<18) //no spacer for rightmost display
          switch (dispNr % 3)
          {
            case 0:
              if (updateSpacerTop)
                Multi_OLEDWriteString(dispNr, 5*16, 0, spacerBuf, FONT_LARGE, 0);
              if (updateSpacerBottom)  
                Multi_OLEDWriteString(dispNr, 5*16, 4, spacerBuf, FONT_LARGE, 0);    
              break;
            case 1:
              if (updateSpacerTop)
                Multi_OLEDWriteString(dispNr, 3*16, 0, spacerBuf, FONT_LARGE, 0);
              if (updateSpacerBottom)  
                Multi_OLEDWriteString(dispNr, 3*16, 4, spacerBuf, FONT_LARGE, 0);   
              break; 
            case 2:
              if (updateSpacerTop)
                Multi_OLEDWriteString(dispNr, 1*16, 0, spacerBuf, FONT_LARGE, 0);
              if (updateSpacerBottom)  
                Multi_OLEDWriteString(dispNr, 1*16, 4, spacerBuf, FONT_LARGE, 0); 
              if (updateSpacerTop)
                Multi_OLEDWriteString(dispNr, 7*16, 0, spacerBuf, FONT_LARGE, 0);
              if (updateSpacerBottom)  
                Multi_OLEDWriteString(dispNr, 7*16, 4, spacerBuf, FONT_LARGE, 0);   
              break; 
            default:
            break;
          }
}

void OnMidiSysEx(byte* buf, unsigned len)
{
  /*
    // DEBUG

    Multi_OLEDFill(i, 0);
    sprintf(dispLineBuf, "len %d", len);
    Multi_OLEDWriteString(i, 0, 0, dispLineBuf, FONT_LARGE, 0);
    i++;
    if (i>18) i=0;
  */

    /// WORKING ///
  // Tascam 2400 multiple small buffers

    if(buf[0] == 0xF0){
      if (buf[3] == 0x66) {

        char dispNr = buf[4];

        bool m_chan = buf[5];
        

        byte topLineNrChan = 0;
        byte bottomLineNrChan = 1;

        byte topLineNrPanLargeFont = 3;     
        byte bottomLineNrPan = 7;
/*
        if (m_chan)
        {
          topLineNr = 0;
          bottomLineNr = 1;
        }
        else
        {
          topLineNr = 6;
          topLineNrLargeFont = 3;
          bottomLineNr = 7;
        }
*/
        //Multi_OLEDFill(dispNr, 0);

        updateSpacerTop = false;
        updateSpacerBottom = false;

        ///////Top Line
        if (m_chan)
          sprintf(dispLineBuf, (char*)buf + sysexHeaderSize, dispLineLength);
        else
          sprintf(dispLineBuf, (char*)buf + sysexHeaderSize, dispLineLengthLargeFont);

        substituteSpacer();

        if (memcmp(&dispCompareBuf[dispNr][0][0],dispLineBuf,dispLineLength)!=0)
        {
          //erase channel strip when switching to pan mode
          if (!m_chan && previous_m_chan[dispNr])
          {
            Multi_OLEDWriteString(dispNr, 0, topLineNrChan, "                ", FONT_NORMAL, 0);
            updateSpacerTop = true;
          }

          if (m_chan)
            Multi_OLEDWriteString(dispNr, 0, topLineNrChan, dispLineBuf, FONT_NORMAL, 0);
          else
            Multi_OLEDWriteString(dispNr, 0, topLineNrPanLargeFont, dispLineBuf, FONT_LARGE, 0);

          memcpy(&dispCompareBuf[dispNr][0][0], dispLineBuf, dispLineLength);
          if (m_chan)
            updateSpacerTop = true;
          else
            updateSpacerBottom = true;
        }

        ///////Bottom Line
        sprintf(dispLineBuf, (char*)buf + sysexHeaderSize + dispLineLength, dispLineLength);
        substituteSpacer();

        if (memcmp(&dispCompareBuf[dispNr][1][0],dispLineBuf,dispLineLength)!=0)
        {
          if (!m_chan && previous_m_chan[dispNr])
          {
            Multi_OLEDWriteString(dispNr, 0, bottomLineNrChan, "                ", FONT_NORMAL, 0);
            updateSpacerTop = true;
          }
          if (m_chan)
            Multi_OLEDWriteString(dispNr, 0, bottomLineNrChan, dispLineBuf, FONT_NORMAL, 0);  
          else
            Multi_OLEDWriteString(dispNr, 0, bottomLineNrPan, dispLineBuf, FONT_NORMAL, 0);  

          memcpy(&dispCompareBuf[dispNr][1][0], dispLineBuf, dispLineLength);
          if (m_chan)
            updateSpacerTop = true;
          else
            updateSpacerBottom = true;       
        }

        //print mode on screen 19
       
        if (dispNr == 18)
        {
          if (m_chan)
            Multi_OLEDWriteString(dispNr, 0, topLineNrPanLargeFont, "  CHAN  ", FONT_LARGE, 0);
          else
            Multi_OLEDWriteString(dispNr, 0, topLineNrPanLargeFont, "  PAN   ", FONT_LARGE, 0);


        }
       
        printSpacer(dispNr, updateSpacerTop, updateSpacerBottom);

        previous_m_chan[dispNr] = m_chan;
      }
   }
   /*
  /// WORKING ///
  // Tascam 2400 multiple small buffers
  // Font size BIG, 5 chars (6 with delimiter)
    if(buf[0] == 0xF0){
      if (buf[3] == 0x66) {

        char dispNr = buf[4];
        Multi_OLEDFill(dispNr, 0);
        //Top Line
        sprintf(dispLineBuf, (char*)buf+5, dispLineLength);
        substituteSpacer();
        Multi_OLEDWriteString(dispNr, 0, 0, dispLineBuf, FONT_LARGE, 0);
        //Bottom Line
        sprintf(dispLineBuf, (char*)buf+13, dispLineLength);
        substituteSpacer();
        Multi_OLEDWriteString(dispNr, 0, 4, dispLineBuf, FONT_LARGE, 0);    
      }
   }
   */
}

void setup() {
  // put your setup code here, to run once:
  // Listen for MIDI messages
  MIDI.begin();

  MIDI.setHandleSystemExclusive(OnMidiSysEx);

  Multi_I2CInit(sda_list, scl_list, speed_list, NUM_BUSES);
  Multi_OLEDInit(bus_list, addr_list, type_list, flip_list, invert_list, NUM_DISPLAYS);

  for (int i = 0; i < NUM_DISPLAYS; i++)
    previous_m_chan[i] = false;

}

void loop() 
{
    midiEventPacket_t rx;
    uint8_t i = 0;
    char szTemp[16];

  for (i=0; i<NUM_DISPLAYS; i++)
  {
    Multi_OLEDFill(i, 0);
    Multi_OLEDSetContrast(i, 127);
    //Multi_OLEDDrawLine(i, 64,0,64,63);
    printSpacer(i, true, true);
    
    if (i==18)
    {
      Multi_OLEDWriteString(i, 0, 2, (char *)"TASCAM 2400", FONT_NORMAL, 0);
      Multi_OLEDWriteString(i, 0, 3, (char *)"Reaper Csurf", FONT_NORMAL, 0);
      Multi_OLEDWriteString(i, 0, 4, (char *)"OLED Display", FONT_NORMAL, 0);
      Multi_OLEDWriteString(i, 0, 5, (char *)"by ZAQ Audio", FONT_NORMAL, 0);

      delay(2000);
      Multi_OLEDFill(i, 0);
    }


  }
    while (1) {
        //uint32_t poll_time = platform_jiffies();
        //rx = MidiUSB.read();
      if (MIDI.read())                    // If we have received a message
      {        
        //digitalWrite(LED_BUILTIN, HIGH);

        //draw_logic_strip();
        //for (i=0; i<=6; i++)
          //szTemp[i] = logic_control_strip[0][0][i];
        
        //Multi_OLEDWriteString(0, 0, 0, szTemp, FONT_NORMAL, 0);

        //delay(1000);		            // Wait for a second
        //digitalWrite(LED_BUILTIN, LOW);
        
      }
      
    }

}

/*
char displayMap[24][2]; //channelNr; which LCDs to update
displayMap[0][0] = 0; displayMap[0][1] = -1;
displayMap[1][0] = 0; displayMap[1][1] = 1;
displayMap[2][0] = 1; displayMap[2][1] = 2;
displayMap[3][0] = 2; displayMap[3][1] = -1;

displayMap[4][0] = 3; displayMap[4][1] = -1;
displayMap[5][0] = 3; displayMap[5][1] = 4;
displayMap[6][0] = 4; displayMap[6][1] = 5;
displayMap[7][0] = 5; displayMap[7][1] = -1;


void writeOLEDAgnosticNormalFont(char pos, char* buf, char line)
{
//treat the strip as one big display (16x18 = 288 char with, 8 char height)

  const char totalWidth = 18*16;
  const char totalHeight = 8;
  const char length = sizeof (buf);
  const char startPosOnFirstOled = pos%16;
  char* charBuf [totalWidth];

  char xPos = startPosOnFirstOled;
  char yPos = line;

  if (pos > totalWidth)
    return;
  if (pos+length > totalWidth)
    return;
  if (line > 8)
    return;

  // determine OLED of beginning
  char oledNrBegin = pos / 16;
  // determine OLED of ending
  char oledNrEnd = pos+length / 16;

  for (int i = oledNrBegin; i < oledNrEnd; i++)
  {

    Multi_OLEDWriteString(i, xPos, yPos, buf, FONT_NORMAL, 0);
  }

}
*/
