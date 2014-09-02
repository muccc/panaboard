/*************************\
*                         *
* Panaboard Arduino v0.1  *
*     Arduino Mega        *
* (c) andz 2014           *
* Licence: GPLv2          *
\*************************/

#include <SPI.h>
#define PANA_NEWLINE 3
#define PANA_HASPIC 2
byte buf [180];
volatile byte pos = 0;
volatile boolean process_it = 0; 
volatile boolean haspic = 0;
volatile boolean linefinished = 0;
volatile int lineNumber = 0;

void setup (void)
{
  Serial.begin (500000);          //We need speed
  pinMode(MISO, OUTPUT);          // Set Arduino to Slave
  pinMode(PANA_NEWLINE, INPUT);   // New Line Sigal
  pinMode(PANA_HASPIC, INPUT);    // Gets High somewhere in the Line when a Line is a Scanned line
  SPCR |= _BV(SPE);               // turn on SPI in slave mode
  pos = 0;                        // buffer empty
  process_it = false;             // No scanned Line
  SPI.attachInterrupt();          // turn on SPI interrupt
  attachInterrupt(1, int_newline, RISING); // turn on NewLine Interrupt

}  // end of setup

///////// NEWLINE interrupt routine

void int_newline(void)
{
  SPCR |= _BV(SPE); // Turn on SPI Interrupt again
  byte clr=SPDR;    // Flush SPI Data Register
  SPSR=0x00;        // Flush SPI Status Register
  pos = 0;          // Reset Buffer
  if(haspic){       // Line was a scanned Line
    lineNumber++;   // Increase Line Number
    haspic = false; // Reset scanned Line Flag
  }
  else
    lineNumber = 0; // Line had no Pic
}


///////// SPI interrupt routine

ISR (SPI_STC_vect)
{
  byte c = SPDR;  // grab byte from SPI Data Register
  if (pos < 180)  // Line is not full
    {
    buf [pos++] = c;  // Add byte to buffer     
    } 
  else
  {
    process_it = true; // Line is Finished, send it via UART
    SPCR &= ~_BV(SPE); // Disable SPI interrupt to avoid false data
  }
    if(!haspic) // if Line not yet flaged as scanned line
      haspic = digitalRead(PANA_HASPIC); // look if line is scanned
}

///////// main loop
void loop (void)
{
  if (process_it & haspic) // If line was a scanned line and line Buffer is full --> process
    {
    buf[0] = 0xFF; // Test Byte
    buf[2] = 0xFF; // Test Byte
    Serial.write (buf,180); // Write whole Line via UART
    byte clr=SPDR; // Flush SPI Data Register (again, don't know why, but it works)
    clr=SPSR;   // Flush SPI Status Register (again, don't know why, but it works)
    pos = 0; // Reset Buffer (again, don't know why, but it works)
    process_it = false; // Processing finished
    } 
    
} 


