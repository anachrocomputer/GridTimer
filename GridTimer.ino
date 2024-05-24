/* GridTimer --- show a timer on a 16x16 pixel display      2018-06-17 */
/* Copyright (c) 2018 John Honniball. All rights reserved.             */

/* Released under the GNU Public Licence (GPL) */

#include <SPI.h>

// Direct port I/O defines for Arduino with ATmega328
// Change these if running on Mega Arduino
#define LEDOUT PORTB
#define CS     0x04
#define SDA    0x08
#define SCLK   0x20

// Connections to MAX7219 via SPI port on AVR chip
// I'm actually using a ready-made MAX7219 board and red LED
// matrix display, Deal Extreme (dx.com) SKU #184854
#define slaveSelectPin 10  // CS pin
#define SDAPin 11          // DIN pin
#define SCLKPin 13         // CLK pin

// Registers in MAX7219
#define NOOP_REG        (0x00)
// Display registers 1 to 8
#define DECODEMODE_REG  (0x09)
#define INTENSITY_REG   (0x0A)
#define SCANLIMIT_REG   (0x0B)
#define SHUTDOWN_REG    (0x0C)
#define DISPLAYTEST_REG (0x0F)

// Size of LED matrix
#define MAXX 16
#define MAXY 16
#define MAXROWS 16


// The pixel buffer, 32 bytes
unsigned short FrameBuffer[MAXY];


int Brightness = 7; // LED matrix display brightness


void setup(void)
{
  Serial.begin(9600);
  
  Serial.println("GridTimer");
  Serial.println("John Honniball, Jun 2018");

  // Initialise LED matrix controller chips
  max7219_begin();

  // Clear frame buffer and LED matrix (all pixels off)
  clrFrame();
  
  updscreen();
  
  // Wait one second
  delay(1000);
}


void loop(void)
{
  int x, y;
  int i, j, tmp;
  int buf[16];

  clrFrame();

  for (y = 0; y < 16; y++) {
    for (x = 0; x < 16; x++) {
      buf[x] = x;
    }

    for (x = 0; x < 16; x++) {
      i = random(16);
      j = random(16);

      if (i != j) {
        tmp = buf[i];
        buf[i] = buf[j];
        buf[j] = tmp;
      }
    }

    for (x = 0; x < 16; x++) {
      setPixel(buf[x], y);
  
      updscreen();

      delay(50);
    }
  }
}


/* clrFrame --- clear the entire frame (all LEDs off) */

void clrFrame(void)
{
  memset(FrameBuffer, 0, sizeof (FrameBuffer));
}


/* setPixel --- set a single pixel in the frame buffer */

void setPixel(const int x, const int y)
{
  FrameBuffer[y] |= (1 << x);
}


/* clrPixel --- clear a single pixel in the frame buffer */

void clrPixel(const int x, const int y)
{
  FrameBuffer[y] &= ~(1 << x);
}


/* updscreen --- update the physical screen from the frame buffer */

void updscreen(void)
{
// About 40us on 16MHz Arduino
//  unsigned long int before, after;
  int r;
  
//  before = micros();
  
  for (r = 0; r < 8; r++) {
    LEDOUT &= ~CS;    //  digitalWrite(slaveSelectPin, LOW);
    SPI.transfer(r + 1);
    SPI.transfer(FrameBuffer[r + 8] >> 8);
    SPI.transfer(r + 1);
    SPI.transfer(FrameBuffer[r + 8] & 0xff);
    SPI.transfer(r + 1);
    SPI.transfer(FrameBuffer[r] >> 8);
    SPI.transfer(r + 1);
    SPI.transfer(FrameBuffer[r] & 0xff);
    LEDOUT |= CS;     //  digitalWrite(slaveSelectPin, HIGH);
  }

//  after = micros();
  
//  Serial.print(after - before);
//  Serial.println("us updscreen");
}


/* max7219_begin --- initialise the MAX219 LED driver */

void max7219_begin(void)
{
  int i;

  /* Configure I/O pins on Arduino */
  pinMode(slaveSelectPin, OUTPUT);
  pinMode(SDAPin, OUTPUT);
  pinMode(SCLKPin, OUTPUT);
  
  digitalWrite(slaveSelectPin, HIGH);
  digitalWrite(SDAPin, HIGH);
  digitalWrite(SCLKPin, HIGH);

  SPI.begin();
  // The following line fails on arduino-0021 due to a bug in the SPI library
  // Compile with arduino-0022 or later
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  
  /* Start configuring the MAX7219 LED controller */
  max7219write(DISPLAYTEST_REG, 0); // Switch off display test mode
  
  max7219write(SHUTDOWN_REG, 1 | (1 << 8));    // Exit shutdown

  max7219write(INTENSITY_REG, 7 | (7 << 8));   // Brightness half

  max7219write(DECODEMODE_REG, 0);  // No decoding; we don't have a 7-seg display

  max7219write(SCANLIMIT_REG, 7 | (7 << 8));   // Scan limit 7 to scan entire display

  for (i = 0; i < 8; i++) {
    max7219write(i + 1, 0);
  }
}


/* max7219write --- write a command to the MAX7219 */

void max7219write(const unsigned char reg, const unsigned short val)
{
// Use direct port I/O and hardware SPI for speed

  LEDOUT &= ~CS;    //  digitalWrite(slaveSelectPin, LOW);
  SPI.transfer(reg);
  SPI.transfer(val >> 8);
  SPI.transfer(reg);
  SPI.transfer(val & 0xff);
  LEDOUT |= CS;     //  digitalWrite(slaveSelectPin, HIGH);
}
