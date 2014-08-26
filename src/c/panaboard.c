/********
panaboard.c
(c) andz 2014
GPLv2
26.08.2014
***********/

#include <stdio.h>
#include <pigpio.h>

#DEFINE DATA 2
#DEFINE CLK 3
#DEFINE NEWLINE 4
#DEFINE PICINDATA 17

/*
cc -o LDR LDR.c -lpigpio -lrt -lpthread
*/

#define LDR 11 /* GPIO 11 as interrupt input */

/* forward declaration */

void alert(int pin, int level, uint32_t tick);

int main (int argc, char *argv[])
{
   if (gpioInitialise()<0) return 1;

   gpioSetAlertFunc(LDR, alert); /* call alert when LDR changes state */
   gpioSetMode(LDR, PI_INPUT);

   while (1)
   {
      gpioDelay(1); /* nominal 1000000 readings per second */
   }

   gpioTerminate();
}

void alert(int pin, int level, uint32_t tick)
{
        fprintf(stderr, "Sensor Alarm!\n");
}