/* This displays numbers using a nice font based on a picture of an actual LED race board I saw.
 *  
 *  The height is 16 and width is 9 (fixed width).
 *  3 pixels between characters looks nice, meaning pitch is 12 horizontally.
 *  4 pixels vertically looks good, meaning pitch is 20 vertically.
 *  
 *  This module assumes "tft" exists and represents an Adafruit 160x128 TFT display.
 */

void drawDigit(int x, int y, int digit, int color)
{
  tft.fillRect(x, y, 9, 16, ST7735_BLACK);
  switch (digit)
  {
   case 0:
      tft.drawFastHLine(x+1, y+0, 7, color);
      tft.drawFastHLine(x+0, y+1, 9, color);
      tft.drawFastHLine(x+0, y+14, 9, color);
      tft.drawFastHLine(x+1, y+15, 7, color);
      tft.drawFastVLine(x+0, y+2, 13, color);
      tft.drawFastVLine(x+1, y+2, 14, color);
      tft.drawFastVLine(x+7, y+2, 14, color);
      tft.drawFastVLine(x+8, y+2, 13, color);
      break;
   case 1:
      tft.drawFastVLine(x+7, y+0, 16, color);
      tft.drawFastVLine(x+8, y+0, 16, color);
      break;
    case 2:
      tft.drawFastHLine(x+1, y+0, 7, color);
      tft.drawFastHLine(x+1, y+1, 8, color);
      tft.drawFastHLine(x+1, y+7, 8, color);
      tft.drawFastHLine(x+0, y+8, 8, color);
      tft.drawFastHLine(x+0, y+14, 8, color);
      tft.drawFastHLine(x+1, y+15, 7, color);
      tft.drawFastVLine(x+7, y+2, 5, color);
      tft.drawFastVLine(x+8, y+2, 5, color);
      tft.drawFastVLine(x+0, y+9, 5, color);
      tft.drawFastVLine(x+1, y+9, 5, color);
      break;
    case 3:
      tft.drawFastHLine(x+1, y+0, 7, color);
      tft.drawFastHLine(x+1, y+1, 8, color);
      tft.drawFastHLine(x+1, y+7, 8, color);
      tft.drawFastHLine(x+1, y+8, 8, color);
      tft.drawFastHLine(x+0, y+14, 8, color);
      tft.drawFastHLine(x+1, y+15, 7, color);
      tft.drawFastVLine(x+7, y+2, 5, color);
      tft.drawFastVLine(x+8, y+2, 5, color);
      tft.drawFastVLine(x+7, y+9, 5, color);
      tft.drawFastVLine(x+8, y+9, 5, color);
      break;
    case 4:
      tft.drawFastHLine(x+2, y+7, 5, color);
      tft.drawFastHLine(x+2, y+8, 5, color);
      tft.drawFastVLine(x+0, y+0, 8, color);
      tft.drawFastVLine(x+1, y+0, 9, color);
      tft.drawFastVLine(x+7, y+0, 16, color);
      tft.drawFastVLine(x+8, y+0, 16, color);
      break;
    case 5:
      tft.drawFastHLine(x+1, y+0, 7, color);
      tft.drawFastHLine(x+0, y+1, 8, color);
      tft.drawFastHLine(x+0, y+7, 8, color);
      tft.drawFastHLine(x+1, y+8, 8, color);
      tft.drawFastHLine(x+1, y+14, 8, color);
      tft.drawFastHLine(x+1, y+15, 7, color);
      tft.drawFastVLine(x+0, y+2, 5, color);
      tft.drawFastVLine(x+1, y+2, 5, color);
      tft.drawFastVLine(x+7, y+9, 5, color);
      tft.drawFastVLine(x+8, y+9, 5, color);
      break;
    case 6:
      tft.drawFastHLine(x+1, y+0, 7, color);
      tft.drawFastHLine(x+0, y+1, 8, color);
      tft.drawFastHLine(x+0, y+7, 8, color);
      tft.drawFastHLine(x+1, y+8, 8, color);
      tft.drawFastHLine(x+1, y+14, 8, color);
      tft.drawFastHLine(x+1, y+15, 7, color);
      tft.drawFastVLine(x+0, y+2, 13, color);
      tft.drawFastVLine(x+1, y+2, 14, color);
      tft.drawFastVLine(x+7, y+9, 5, color);
      tft.drawFastVLine(x+8, y+9, 5, color);
      break;
    case 7:
      tft.drawFastHLine(x+1, y+0, 7, color);
      tft.drawFastHLine(x+1, y+1, 8, color);
      tft.drawFastVLine(x+7, y+2, 14, color);
      tft.drawFastVLine(x+8, y+2, 14, color);
      break;
   case 8:
      tft.drawFastHLine(x+1, y+0, 7, color);
      tft.drawFastHLine(x+0, y+1, 9, color);
      tft.drawFastHLine(x+0, y+7, 9, color);
      tft.drawFastHLine(x+0, y+8, 9, color);
      tft.drawFastHLine(x+0, y+14, 9, color);
      tft.drawFastHLine(x+1, y+15, 7, color);
      tft.drawFastVLine(x+0, y+2, 13, color);
      tft.drawFastVLine(x+1, y+2, 14, color);
      tft.drawFastVLine(x+7, y+2, 14, color);
      tft.drawFastVLine(x+8, y+2, 13, color);
      break;
   case 9:
      tft.drawFastHLine(x+1, y+0, 7, color);
      tft.drawFastHLine(x+0, y+1, 9, color);
      tft.drawFastHLine(x+0, y+7, 9, color);
      tft.drawFastHLine(x+1, y+8, 8, color);
      tft.drawFastHLine(x+1, y+14, 8, color);
      tft.drawFastHLine(x+1, y+15, 7, color);
      tft.drawFastVLine(x+0, y+2, 5, color);
      tft.drawFastVLine(x+1, y+2, 5, color);
      tft.drawFastVLine(x+7, y+2, 14, color);
      tft.drawFastVLine(x+8, y+2, 13, color);
      break;
  }
}

void drawInt_4(int x, int y, int digit, int color)
{
  drawDigit(x, y, (digit/1000) % 10, color);
  drawDigit(x+12, y, (digit/100) % 10, color); 
  drawDigit(x+12*2, y, (digit/10) % 10, color); 
  drawDigit(x+12*3, y, (digit) % 10, color); 
}
void drawInt_3(int x, int y, int digit, int color)
{
  drawDigit(x, y, (digit/100) % 10, color);
  drawDigit(x+12, y, (digit/10) % 10, color); 
  drawDigit(x+12*2, y, (digit) % 10, color); 
}
void drawInt_2(int x, int y, int digit, int color)
{
  drawDigit(x+12*0, y, (digit/10) % 10, color); 
  drawDigit(x+12*1, y, (digit) % 10, color); 
}


// Draws a number in 1.234  format.
// This will require 4*12+3 = 51 total width
void drawMillis_1_3(int x, int y, int digit, int color)
{
  drawDigit(x, y, (digit/1000) % 10, color);
  tft.drawRect(x+11, y+14, 2, 2, color);
  drawDigit(x+3+12, y, (digit/100) % 10, color); 
  drawDigit(x+3+12*2, y, (digit/10) % 10, color); 
  drawDigit(x+3+12*3, y, (digit) % 10, color); 
}


/*  Draw the number of milliseconds in a xx:xx.xxx format.
 *  For this one, we assume it will be drawn in the same place
 *   every time.  We keep track of previously displayed digits
 *   and only update a digit if it has changed.
 */
int oldDigs[7] = {9,9,9,9,9,9,9};
bool didColons = 0;

void drawMillis_2_2_3(int x, int y, unsigned long timeToDisplay, int color)
{
  int digs[7];
  unsigned long ttd = timeToDisplay; 
  int i;
  int pOff = 0;

  /* Figure out each digit, starting from least significant. 
   *  Most of the time it's a divide by 10 operation, except
   *  for the 2nd digit of seconds.
   */
  for (i = 6; i >= 0; i--)
  {
    digs[i] = ttd % ((i == 2) ? 6 : 10);
    ttd /= ((i == 2) ? 6 : 10);
  }

  /* Now display each digit if needed */
  for (i = 0; i < 7; i++)
  {
    if (digs[i] != oldDigs[i])
    {
      drawDigit (x + pOff, y, digs[i], color);
      oldDigs[i] = digs[i];
    }
    
    pOff += 12;
    if (i == 1) pOff += 3;
    if (i == 3) pOff += 3;
  }

  if (didColons == 0)
  {
    tft.drawRect(x+24, y+4, 2, 2, color);
    tft.drawRect(x+24, y+11, 2, 2, color);
    tft.drawRect(x+51, y+4, 2, 2, color);
    tft.drawRect(x+51, y+11, 2, 2, color); 
    didColons = 1;
  }
}

/* This is the original one that works, but always overwrites
 *  every digit.
 */
void drawMillis_2_2_3_orig(int x, int y, long digit, int color)
{
  int mils = digit % 1000;
  int secs = (digit / 1000) % 60;
  int mins = ((digit / 60) / 1000) % 60;

  drawInt_2(x, y, mins, color);
  tft.drawRect(x+24, y+4, 2, 2, color);
  tft.drawRect(x+24, y+11, 2, 2, color);
  drawInt_2(x+27, y, secs, color);
  tft.drawRect(x+51, y+4, 2, 2, color);
  tft.drawRect(x+51, y+11, 2, 2, color);
  drawInt_3(x+54, y, mils, color);
}
