/*********
 * Slot Car Timer
 * 
 * H. Pinder     Original work done in Nov. 2016, updated Sept 2020.
 * 
 * This implements a race timer for a slot car track, using IR LEDs and IR photodiodes to detect movement, and
 *  a color TFT display to show the results.
 * This was designed for my Carrera GO! two-track layout, which is around 52 track pieces, and 30 feet in length.
 *   But it is adaptable for any scale and track length.
 * It supports a race on one track only (i.e. time trial), or both tracks in a classic head-to-head race.
 * It supports racing in either direction, and detects this direction automatically. 
 * It supports the two tracks racing in opposite directions.
 * It saves the last lap, best lap for current race, and record lap across all races since power-up
 *   (TODO: add support for saving record times in the eeprom)
 * Uses a custom font for the numbers.
 * 
 * I highly recommend adding a track timer to your slot car setup!
 * This has increased our enjoyment of the track tremendously. 
 * It is truly worth the effort to get set up, you will be rewarded with a lot of terrific racing fun!
 *  
 * Please feel free to use this software for your own personal non-commercial use.
 * If you'd like, send me a photo of your track!  pinderh at gmail dot com
 */
 
#include <Adafruit_GFX.h>            // Core graphics library
#include <Adafruit_ST7735.h>         // Hardware-specific library
#include <Fonts/FreeSans12pt7b.h>
#include <SPI.h>

// These are the wires connected to the color TFT display  (From Adafruit: https://www.adafruit.com/product/358)
#define TFT_CS     10
#define TFT_RST    9
#define TFT_DC     8
#define TFT_LITE   6

// Use hardware SPI pins to connect the display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

// The inputs pins from the IR detectors.  
// Low means no car, high means there's a car blocking the light
// "First" and "Second" refer to the order they will be hit when going clockwise.
#define TRACK_1_FIRST  3
#define TRACK_1_SECOND 2
#define TRACK_2_FIRST  1
#define TRACK_2_SECOND 0
#define BLOCKED 1
#define OPEN 0

// Store statistics here.  The _old construct is used to keep track
//  of when these values change, so that the screen is overwritten
//  only when needed.
// They all use an array of size 2, supporting 2 tracks.

int bestLap[2] = {9999,9999};
int bestLap_old[2] = {1,1};     // used to know if bestLap has changed
int totalLaps[2] = {0,0};
int totalLaps_old[2] = {1,1};   // used to know if totalLaps has changed
int lastLap[2] = {0,0};
int lastLap_old[2] = {1,1};     // to know if lastLap has changed
int recordLap[2][2] = {{9999,9999},{9999,9999}};   // first index: 0=CCW, 1=CW.  2nd index: track
int recordLap_old[2] = {1,1};
 
unsigned long lapStartTime[2];
unsigned long raceStartTime = 0;     // A race has only one start time, not 2
unsigned long pauseRaceTimeDisplayUntil = 0;

// These hold the times captured by the ISRs
volatile unsigned long t1f_millis;
volatile unsigned long t1s_millis;
volatile unsigned long t2f_millis;
volatile unsigned long t2s_millis; 

// This holds the race state information
int tSt[2] = {0,0};
unsigned long smTimer[2];
bool drawn_blank_time = 0;
bool goingCW[2] = {0,0};
int passesWithBackSensorOpen[2] = {0,0};
int passesWithFrontSensorOpen[2] = {0,0};
int passesWithBackSensorClosed[2] = {0,0};
int passesWithFrontSensorClosed[2] = {0,0};

//  Used for debug
//#define DEBUG_STATES 1
int reasonForGoingToIdle = 0;

void setup() {
  
  pinMode (TFT_LITE, OUTPUT);
  //  This simply turns on TFT full brightness.  Some time I could
  //   add a dimming function...
  digitalWrite (TFT_LITE, HIGH);

  pinMode (TRACK_1_FIRST, INPUT);
  pinMode (TRACK_1_SECOND, INPUT);
  pinMode (TRACK_2_FIRST, INPUT);
  pinMode (TRACK_2_SECOND, INPUT);
  
  attachInterrupt(digitalPinToInterrupt(TRACK_1_FIRST),  t1f_int, RISING);
  attachInterrupt(digitalPinToInterrupt(TRACK_1_SECOND), t1s_int, RISING);
  attachInterrupt(digitalPinToInterrupt(TRACK_2_FIRST),  t2f_int, RISING);
  attachInterrupt(digitalPinToInterrupt(TRACK_2_SECOND), t2s_int, RISING);
  
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(1);

  drawDisplayBase();
  
  // Last thing before loop, clear these out
  t1f_millis = t1s_millis = t2f_millis = t2s_millis = 0;
}

void loop() {
  updateDisplay();
  doRaceStateMachine(0);
  doRaceStateMachine(1);
  delay(5);
}

// The interrups.  All they do is capture when the light beam was broken.
void t1f_int() { t1f_millis = millis(); }
void t1s_int() { t1s_millis = millis(); }
void t2f_int() { t2f_millis = millis(); }
void t2s_int() { t2s_millis = millis(); }


/* 
 *  This section implements a state machine that controls the race.
 *  Below are the state definitions.
 *  Each track has its own state machine.
 *  The car starts out on the 'back' sensor. The actual race is timed from when cars hit the 'front' sensor.
 */
#define TST_IDLE            1         // Idle, doing nothing, waiting for someone to place a car at the back position
#define TST_WAIT_RACE_PREP  2         // Car is placed, make sure it stays on the back position for long enough
#define TST_RACE_PREP       3         // Waiting for the car to start going, i.e. leaving the back sensor
#define TST_WAIT_RACING     4         // Waiting for car to trip the front sensor after leaving the back sensor
#define TST_RACING_1        5         // Racing, doing a lap, waiting for back sensor to be tripped
#define TST_RACING_2        6         // Racing, back sensor was tripped, waiting for it to go away
#define TST_RACING_3        7         // Racing, back sensor was tripped and gone away, now waiting for front sensor

// Timeout values are in milliseconds
#define RACE_PREP_TIMEOUT      3000   // A car must be on the back position this long before we acknowledge they want to race
#define RACE_PENDING_TIMEOUT   1000   // A car must hit front sensor within this time for the race to be declared as started.
#define RACE_LAP_TIMEOUT      40000   // If a car takes longer than this to make a lap, we assume human has given up
#define RACE_TRIGS_DELTA_MAX   1000   // If a car shows up on a sensor longer than this during a race, assume a new race is desired
#define RACE_SHORTEST_POSSIBLE_LAP  4000  // Any lap shorter than this is assumed to be cheating (or a glitchy sensor)


// Run the race state machine on the given track
void doRaceStateMachine(int track)
{
  int frontSensor, backSensor;
  volatile unsigned long *frontTrig, *rearTrig;
  unsigned long now;
  unsigned long lap;
  int lastTst;
    
  now = millis();

  // Fetch a few values, depending on which track, and which direction
  if (goingCW[track] == 0)
  {
    frontSensor = digitalRead(track ? TRACK_2_FIRST : TRACK_1_FIRST);
    backSensor = digitalRead(track ? TRACK_2_SECOND : TRACK_1_SECOND);
    frontTrig = track ? &t2f_millis : &t1f_millis;
    rearTrig = track ? &t2s_millis : &t1s_millis;
  }
  else
  {
    frontSensor = digitalRead(track ? TRACK_2_SECOND : TRACK_1_SECOND);
    backSensor = digitalRead(track ? TRACK_2_FIRST : TRACK_1_FIRST);
    frontTrig = track ? &t2s_millis : &t1s_millis;
    rearTrig = track ? &t2f_millis : &t1f_millis;
  }

  // Update an accumulator of the number of consecutive times the loop has
  //  been called, with the sensor either open or closed.
  if (backSensor == 0) passesWithBackSensorOpen[track]++;
  else passesWithBackSensorOpen[track] = 0;

  if (frontSensor == 0) passesWithFrontSensorOpen[track]++;
  else passesWithFrontSensorOpen[track] = 0;

  if (backSensor == 1) passesWithBackSensorClosed[track]++;
  else passesWithBackSensorClosed[track] = 0;

  if (frontSensor == 1) passesWithFrontSensorClosed[track]++;
  else passesWithFrontSensorClosed[track] = 0;

  // Keep track of the last track state, before updating to possibly a new state  
  lastTst = tSt[track];

  // Implement the track state machine
  switch(tSt[track])
  {
    case 0:
      // this is an unitialized state, always go to Idle state
      tSt[track] = TST_IDLE;
      break;
      
    case TST_IDLE:
      // Default state, nothing happening.
      // Things get interesting when car is put on back sensor.
      // Until then - nothing.
      lapStartTime[track] = 0;
      passesWithFrontSensorClosed[track] = 0;
      passesWithFrontSensorOpen[track] = 0;
      passesWithBackSensorClosed[track] = 0;
      passesWithBackSensorOpen[track] = 0;
      
      if (backSensor == BLOCKED)   
      {
        smTimer[track] = now;             // capture current time, for use later
        tSt[track] = TST_WAIT_RACE_PREP;  // go to next state if back sensor is blocked
      }
      if (frontSensor == BLOCKED)  
      {
        smTimer[track] = now;
        goingCW[track] = 1 - goingCW[track];  // if front sensor is blocked, assume a race in the opposite                      
        tSt[track] = TST_WAIT_RACE_PREP;      // direction is desired
      }
      break;

    case TST_WAIT_RACE_PREP:
      // We've detected a car on back sensor, but need to wait to make sure it
      //  wasn't just a glitch or something - they really want to race.
      // The wait time is RACE_PREP_TIMEOUT
      
      if (((now - smTimer[track]) > RACE_PREP_TIMEOUT) &&
          (lapStartTime[1-track] == 0))
      {
        // Yes, they kept the car there long enough, go to race prep mode.
        // Also, the other track is not in the middle of a race.
        tSt[track] = TST_RACE_PREP;
        lastLap[track] = 0;
        totalLaps[track] = 0;                // Reset the lap counter
        *frontTrig = 0;
      }
      else if (passesWithBackSensorOpen[track] > 50)
      {
        // We have a glitchy back sensor, but assume no glitch will last longer than 50 passes...
        // Well for some reason they decided not to race.  Hmmm.
        reasonForGoingToIdle = 1;
        tSt[track] = TST_IDLE;
      }
      break;
      
    case TST_RACE_PREP:
      // In this mode we are waiting for the race to start.  We will detect this
      //   by watching the back sensor - when it goes OPEN, something is happening!
      // Until then, just wait.  We don't know how long those humans will chat prior
      //   to actually racing (need time for trash talk).
      if (backSensor == OPEN)
      {
        tSt[track] = TST_WAIT_RACING;
        smTimer[track] = now;       // start timeout again
        // clear out all interrupt-driven timer values
        if (track == 0)
        {
          t1f_millis = t1s_millis = 0;
        }
        else
        {
          t2f_millis = t2s_millis = 0;
        }
      }
      break;
      
    case TST_WAIT_RACING:
      // Now we *might* be starting the race, or maybe the human just moved the car off the track.
      //  We will know the answer by what happens with the front sensor.  If it is blocked in short order,
      //  the race is on!  If not, human took the car off and doesn't want to race.
      // Instead of just checking the front sensor, we will actually check the value that should be set
      //  via the interrupt.
          
      if ((now - smTimer[track]) > RACE_PENDING_TIMEOUT)
      {
        // No race - human took car off track.  Too bad!
        reasonForGoingToIdle = 2;
        tSt[track] = TST_IDLE;
      } 
      else if (*frontTrig != 0)
      {
        //  YES!!!   RACE IS ON!!
          
        tSt[track] = TST_RACING_1;
        
        smTimer[track] = now;           // start our state machine timer again
        lapStartTime[track] = *frontTrig;  // Capture the start time of this lap
          
        if (lapStartTime[1-track] == 0)
        {
            // Means that this track was the first to get started.
            //  This means that *now* is the overall race start time.
            raceStartTime = *frontTrig;
        }

        // Reset the 'best' lap, which is the best lap of this race
        bestLap[track] = 9999;

        *frontTrig = 0;                    // Reset the trigger for next time
        *rearTrig = 0;
      }
      else if (backSensor == BLOCKED)
      {
        /* Thought the race was going to start but it was just a glitchy back sensor, so go back */
        tSt[track] = TST_RACE_PREP;
      }
      break;
      
    case TST_RACING_1:
      /* 
       * Each lap will alternate between three states - racing1,2,3.  The purpose of these states
       *  is to make sure the car actually hits both sensors in a reasonable amount of time.
       * We also have an overall timeout that says, hey the human has obviously stopped racing.
       *
       * This first state should be the state during most of the lap.  We expect the car to be
       *  driving around, and will exit this state when the rear sensor is hit.
       *
       * However, looks like we were getting some false rear sensor triggers left over from the 
       *  last lap, so make sure to check for that case.
       */
      if ((now - smTimer[track]) > RACE_LAP_TIMEOUT)
      {
        /* It has been too long, race is over.  Too bad! */
        reasonForGoingToIdle = 3;
        tSt[track] = TST_IDLE;
      }
      if (backSensor == BLOCKED)
      {
        if ((now - smTimer[track]) < RACE_SHORTEST_POSSIBLE_LAP)
        {
          /* oops - must be a mistake - trigger is too early.
           * Just reset the trigger and ignore it.
           */
          *rearTrig = 0;
        }
        else 
        {
          tSt[track] = TST_RACING_2;
          smTimer[track] = now;
        }
      }
      if (passesWithFrontSensorClosed[track] > 50)
      {
        /* This shouldn't happen.  We've been a long time with the front sensor closed, without hitting back sensor.
         *  This means the human has picked up the car and wants to start racing the other
         *  direction
         */
         tSt[track] = TST_IDLE;
         reasonForGoingToIdle = 8;
      }
      break;
      
      case TST_RACING_2:
         /* We can get to this state two ways, either it's a valid lap, or the human has given up this
          *  race, picked up the car, and put it back on the starting block.
          * To tell the difference, it all depends on how long the rear sensor is active:
          *  - short time means a valid lap
          *  - longer time means start over again
          * Stay in this state until the rear sensor either goes
          *  away (meaning it looks like a good lap) or it stays on too long (human wants to
          *  restart the race.  
          */
          if ((now - smTimer[track]) > RACE_TRIGS_DELTA_MAX)
          {
              /* Human is restaring the race  */
              reasonForGoingToIdle = 4;
              tSt[track] = TST_IDLE;
          }
          if (backSensor == OPEN)
          {
              smTimer[track] = now;
              tSt[track] = TST_RACING_3;
          }
          break;
          
    case TST_RACING_3:
      // Looks like this was a valid lap.  However there's a timeout in case not.
      lap = *frontTrig - lapStartTime[track];
      lap = min(lap, 9999); 
          
      if ((frontSensor == BLOCKED) &&
          (lap > RACE_SHORTEST_POSSIBLE_LAP))
      {
        
        // This means it was a good lap.
        totalLaps[track]++;
        lastLap[track] = lap;
        lapStartTime[track] = *frontTrig;
        pauseRaceTimeDisplayUntil = now + 2000;   // pause the display
 
        // The all-important point a which we determine if this was the best lap in this race!
        if (lap < bestLap[track])
        {
          bestLap[track] = lap;
        }

        // And equally important, was a record shattered???
        if (lap < recordLap[goingCW[track]][track])
        {
          recordLap[goingCW[track]][track] = lap;
        }
        
        *frontTrig = 0;
        *rearTrig = 0;

        smTimer[track] = now;
        tSt[track] = TST_RACING_1;
      }
      
      if (now - smTimer[track] > RACE_TRIGS_DELTA_MAX)
      {
        reasonForGoingToIdle = 6;
        tSt[track] = TST_IDLE;
      }
   
      break;
    
    default:
      // This is a bad state somehow
      reasonForGoingToIdle = 7;
      tSt[track] = TST_IDLE;
      break;
  }

#if DEBUG_STATES
  // I was having issues with going back to Idle state for unknown reasons, so I wrote this code to 
  //  display the state, and display a reason code on the TFT display for going back to Idle.
  if (lastTst != tSt[track])
  {
     tft.fillRect(71 + track * 13, 58, 5, 7, ST7735_BLACK);
     tft.setCursor(71 + track * 13, 58);
     tft.print(tSt[track]); 
  }

  if ((tSt[track] == TST_IDLE) && (lastTst != TST_IDLE))
  {
    tft.fillRect(76, 78, 5, 7, ST7735_BLACK);
    tft.setCursor(76,78);
    tft.print(reasonForGoingToIdle);
  }
#endif

}


// Update all the fields on the display that are updateable.
// The idea is to avoid refreshing the display too much, which is slow, so 
//   only update what is needed.
//
void updateDisplay()
{
  int i;
    
  for (i = 0; i < 2; i++)
  {
    if (totalLaps_old[i] != totalLaps[i])
    {
      drawInt_2(34 + i*96 - 12, 46, totalLaps[i], ST7735_GREEN);
      totalLaps_old[i] = totalLaps[i];
    }

    if (lastLap_old[i] != lastLap[i])
    {
      drawMillis_1_3(31 + i*98 - 25, 68, lastLap[i], ST7735_BLUE);
      lastLap_old[i] = lastLap[i];
    }
 
    if (bestLap_old[i] != bestLap[i])
    {
      drawMillis_1_3(31 + i*98 - 25, 88, bestLap[i], ST7735_BLUE);
      bestLap_old[i] = bestLap[i];
    }

    if (recordLap_old[i] != recordLap[goingCW[i]][i])
    {
      drawMillis_1_3(31 + i*98 - 25, 108, recordLap[goingCW[i]][i], ST7735_BLUE);
      recordLap_old[i] = recordLap[goingCW[i]][i];
    }     
  }

  // See if the race is on or off.  At least one of the cars must be in a race state
  //  in order for it to be on.
  if ((tSt[0] == TST_RACING_1) || (tSt[0] == TST_RACING_2) || (tSt[0] == TST_RACING_3) ||
      (tSt[1] == TST_RACING_1) || (tSt[1] == TST_RACING_2) || (tSt[1] == TST_RACING_3))
  {
    /* This pauses the race time display for awhile to allow people to see the race time after
     *  a lap has been completed.  Just check to see if current time has gone past the saved
     *  time point, and if so clear it out which will allow the display to work again.
     */
    if ((pauseRaceTimeDisplayUntil != 0) && (millis() > pauseRaceTimeDisplayUntil))
    {
      pauseRaceTimeDisplayUntil = 0;
    }
    
    if ((raceStartTime != 0) && (pauseRaceTimeDisplayUntil == 0))
    {
      drawMillis_2_2_3(60, 2, millis() - raceStartTime , ST7735_RED);
      drawn_blank_time = 0;
    }
  }
  else if (drawn_blank_time == 0)
  {
    drawMillis_2_2_3(60, 2, 0 , ST7735_RED);
    drawn_blank_time = 1;
  }

  /* This draws colored boxes around Lap1 or Lap2 areas depending on race states. */
  drawRaceState(0, tSt[0]);
  drawRaceState(1, tSt[1]);

  /* This puts up little red dots indicating the state of IR detectors. Useful for debugging, especially if
   *  your LEDs get knocked over!
   */
  tft.fillRect(76,32,4,4,(digitalRead(TRACK_1_FIRST) == BLOCKED) ? ST7735_RED : ST7735_BLACK);
  tft.fillRect(84,32,4,4,(digitalRead(TRACK_1_SECOND) == BLOCKED) ? ST7735_RED : ST7735_BLACK);
  tft.fillRect(76,38,4,4,(digitalRead(TRACK_2_FIRST) == BLOCKED) ? ST7735_RED : ST7735_BLACK);
  tft.fillRect(84,38,4,4,(digitalRead(TRACK_2_SECOND) == BLOCKED) ? ST7735_RED : ST7735_BLACK);
}

// We are using a 160x128 display in landscape mode, meaning X is [0,159] and Y is [0,127]
//  Using the custom font, this gives 6 lines.
//
//   Race    00:00.000
//    time
//
//     TRACK 1        TRACK 2        (y=35)
//        00    Lap      00          (y=46)
//      0.000   Last    0.000         (y=68)
//      0.000   Best    0.000         (y=88)
//      0.000  Record   0.000         (y=108)
//      15             95
//

void drawDisplayBase()
{
  tft.setFont(0);
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(0, 6);
  tft.print("Race time:");
  tft.setCursor(14, 36);
  tft.print("TRACK 1");
  tft.setCursor(106, 36);
  tft.print("TRACK 2");
  tft.setCursor(80-8, 46+4);
  tft.print("Lap");
  tft.setCursor(80-11, 68+4);
  tft.print("Last");  
  tft.setCursor(80-11, 88+4);
  tft.print("Best");
  tft.setCursor(80-17, 108+4);
  tft.print("Record");
}

int last_color[2] = {ST7735_BLACK, ST7735_BLACK};

/*
 * Draws a box around the race times, in red if the car has just been placed in the start position,
 *  in yellow when the race is ready to start, and in green if the race is active.
 */
void drawRaceState(int track, int state)
{
  int xpos = 2 + 97 * track;
  int ypos = 22;
  int xlen = 58;
  int ylen = 10;
  int color = ST7735_BLACK;
  
  switch(state)
  {
    case TST_WAIT_RACE_PREP:
      color = ST7735_RED;
      break;
    case TST_RACING_1:
    case TST_RACING_2:
    case TST_RACING_3:
      color = ST7735_GREEN;
      break;
    case TST_RACE_PREP:
    case TST_WAIT_RACING:
      color = ST7735_YELLOW;
      break;
  }
  if (color != last_color[track])
  {
    tft.fillRect(xpos, ypos, xlen, ylen, color);
    tft.drawRect(xpos, ypos, xlen, 106, color);
    last_color[track] = color;
  }
}
