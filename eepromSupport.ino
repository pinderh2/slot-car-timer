/*
 * TODO: The EEPROM feature has not been implemented yet.
 * This file is just a starting point for some ideas.
 * 
 * This will store race values in the EEPROM.
 * 
 * The EEPROM data will be stored at offset zero.
 * 
 */
#include <EEPROM.h>

#define EEPROM_MAP_VERSION 1

#define NVM_ADDR 0
#define NUM_HIGH_SCORES 3

struct highScore
{
  unsigned long lapTime;
  char inits[3];
};

struct trackScore
{
  struct highScore high[3];
};
/* The ordering of the tracks:
 *  0-Track 1 CW
 *  1-Track 1 CCW
 *  2-Track 2 CW
 *  3-Track 2 CCW
 */
struct nvmStg
{
  int vers;
  struct trackScore track[4];
};


struct nvmStg myNvm;

struct nvmStg defaultNvmStg = 
{
  EEPROM_MAP_VERSION,
  { 
    { 
      { 
        { 5000, 'B', 'O', 'B'},
        { 5200, 'S', 'L', 'O'}, 
        { 5500, 'M', 'P', 'G'} 
      }
    },
    {
      {
        { 5000, 'B', 'O', 'B'},
        { 5200, 'S', 'L', 'O'}, 
        { 5500, 'M', 'P', 'G'} 
      }
    },
    {
      {
        { 5000, 'B', 'O', 'B'},
        { 5200, 'S', 'L', 'O'}, 
        { 5500, 'M', 'P', 'G'} 
      },
    },
    {
      {
        { 5000, 'B', 'O', 'B'},
        { 5200, 'S', 'L', 'O'}, 
        { 5500, 'M', 'P', 'G'} 
      },
    }   
  }
};

void init_EEPROM()
{
  int vers;
  
  EEPROM.get(NVM_ADDR, myNvm);
  if (myNvm.vers != EEPROM_MAP_VERSION)
  {
    EEPROM.put(NVM_ADDR, defaultNvmStg);
  }

  EEPROM.get(NVM_ADDR, myNvm);
}

void storeScore (unsigned long mSec, int track, char inits[3])
{
  
}


 
