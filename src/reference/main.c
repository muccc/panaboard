

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/wdt.h>
#include <avr/sleep.h>

//#include <stdio.h>
#include <math.h>
#include <avr/interrupt.h> 

#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#include "RTC1338.h"
#include "TWI_Master.h"

#include "usart.h"
#include "sdcard.h"

#include "sd/dos.h"

//#include "param.h"

#include "main.h"


#define ZEILENABSTAND 14 
#define ZEILE1 1
#define ZEILE2 ZEILE1+ZEILENABSTAND
#define ZEILE3 ZEILE2+ZEILENABSTAND
#define ZEILE4 ZEILE3+ZEILENABSTAND
#define ZEILE5 ZEILE4+ZEILENABSTAND
#define ZEILE6 ZEILE5+ZEILENABSTAND
#define ZEILE7 ZEILE6+ZEILENABSTAND
#define ZEILE8 ZEILE7+ZEILENABSTAND
#define ZEILE9 ZEILE8+ZEILENABSTAND
#define ZEILE10 ZEILE9+ZEILENABSTAND


#define DEBUG 1

#define LCD_DDR DDRB


#define STOP 0
#define START 1

#define ID1	'0'
#define ID2 '3'
#define TIME_OUT 60

#define MAX_WRITE_BUFFER 200		//Größe schreibbuffer
#define WRITE_NOW    MAX_WRITE_BUFFER - 25   //

#define PORT_PANA 	PORTG
#define PIN_PANA  	PING
#define DDR_PANA	DDRG
#define NEW_LINE	PG0
#define CLOCK 		PG1
#define DATA		PG2

#define N_ZEILE		1500	//Anzahl der Zeilen
#define SIZE_ZEILE	156		//Bytes pro Zeile

#define PULSE			163  // 14,7Mhz *11,3ms/(1024*1000)
#define PULSE_MIN		133	
#define PULSE_MAX		193

#define NEXT_LINE		200		//Zeit zwischen zwei Linien

void delay_ms(unsigned int period);
void vShowTime(uint8_t u8Stunde, uint8_t u8Minute, uint8_t u8Sekunde);
void vShowDate(uint8_t u8Tag, uint8_t u8Monat, uint8_t u8Jahr);
void cmd(void);
uint8_t menu1(void);

#include "glcd-Display3000-211.h"
extern const prog_uint8_t Font1[], Font2[];

extern const prog_uint16_t  *Bitmap;

volatile uint8_t auto_fahn=0,c_fahn=0;
volatile uint8_t flag_new_day;  
volatile uint16_t sec;


char tstring[20];	//wird in vShowTime verwendet

uint8_t header[]={	0x42, 0x4D, 0x3E, 0xEE, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x28, 0x00,
			 		0x00, 0x00, 0xE8, 0x03, 0x00, 0x00, 0xDC, 0x05, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x00, 0xEE, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00};



int main()
{
		
	char dat_name[13];		//Dateiname
	uint8_t write_buff[MAX_WRITE_BUFFER];
	uint16_t pos_write_buff=0;
	uint8_t byt,c_temp;
	uint8_t n_datei=0;
	uint16_t zeilen_c=0;
	uint16_t z;	

	static char tempStr[20];

	wdt_disable();

	
	
	//Portinitialisierung
  	
	DDRC &=0x00;		//c0-c6 als Input
	PORTC |=0x7f; 		//c0-c6 Pull up
	
	DDRD |=1<< PD7 | 1<<PD6 | 1<<PD5;//   | PD6=3V Regler CE |

	DDRB |= (1<<PB7); //Display Port

	PORTD &= ~( 1<<PD6 );// 3V Regler ein
	PORTB |= (1<<PB7); //B7 to high = switch display lighting on

	DDR_PANA &= ~(1<<NEW_LINE | 1<<CLOCK | 1<<DATA);
	PORT_PANA |= 1<<NEW_LINE | 1<<CLOCK | 1<<DATA;
	
	SD_Init();		//Pins SD-Karte initialisiere	

	LCD_Init();
	delay_ms(1000);

	LCD_Cls(yellow);
	
	Orientation = Portrait180; 

		
	/*
	
	TWIM_Init (100000); //f

	vSetDate(16, 4, 13);
	vSetTime(0, 48, 17);
	
	TWIM_Stop();
	
	*/
	
	
	//ASSR   = (1<< EXCLK | 1<<AS2);  //  externer Takt von Ds1338          // Timer2 asynchron takten
              
    TCCR2A =  0; // normal mode
    TCCR2B = 1<<CS22 |1<<CS21 | 1<<CS20;	//vorteiler 1024                // Vorteiler 1 -> 7,8ms Überlaufperiode
                      
     
    	
	
	sei();


	
	U1_Init(MYUBRR1);
	
	while(1)
	{
	
		

		LCD_Cls(yellow);		
	
		
		
		TWIM_Init (100000); // Initiate TWI Master with bit rate of 100 kHz
		
		vGetTimeDate(); //Zeit von der RTC holen

		TWIM_Stop();
	

		vShowDate(_Tag,_Monat,_Jahr);
		vShowTime(_Stunde,_Min,_Sec);

			
		
LOOP1:	


		
		switch(SD_Present())
		{
			case NO_SD:
		
				LCD_Print("NO SD-CARD   ", 1, ZEILE1, 2, 1, 1, blue, yellow);
				
				delay_ms(1000);
				
				goto LOOP1;		
			break;

			case SD_WE:
			
			LCD_Print("SD-CARD WR EN", 1, ZEILE1, 2, 1, 1, blue, yellow);	//karte vorhanden kein Schreibschutz
			
			break;

			case SD_WD:
		
			LCD_Print("SD-CARD WR DI", 1, ZEILE1, 2, 1, 1, blue, yellow);
			
			delay_ms(1000);
				
			
			
			goto LOOP1;	
			break;
				
		}
				
					
		SD_Tristate(1);   //power on sd-card
	

						
		#ifdef MMC_CARD_SPI //SD_CARD_SPI too !
			MMC_IO_Init();
		#endif
	
	

		if(GetDriveInformation()==F_OK) 	
		{
			LCD_Print("Open SD Ok", 1, ZEILE2, 2, 1, 1, blue, yellow);
		}
		else 	
		{
			LCD_Print("Error Open SD", 1, ZEILE2, 2, 1, 1, blue, yellow);
			
			delay_ms(1000);
			goto LOOP1;	
		}

		//Dateiname generieren 
		//TODO prüfen ob vorhanden

		dat_name[0]=0x30+ _Tag/10;
		dat_name[1]=0x30+ _Tag%10;
		dat_name[2]='_';
		dat_name[3]=0x30+ _Monat/10;
		dat_name[4]=0x30+ _Monat%10;;
		dat_name[5]='_';
		dat_name[6]= 0x30 ;
		dat_name[7]= 0x30 ;
		dat_name[8]='.';
		dat_name[9]='b';
		dat_name[10]='m';
		dat_name[11]='p';
		dat_name[12]='\0';

		
		
		while(1)
		{
			if(n_datei>99)
			{ 
				LCD_Print("SD full", 1, ZEILE3, 2, 1, 1, blue, yellow);
				goto LOOP1;
			}
			dat_name[6]=0x30+n_datei/10;
			dat_name[7]=0x30+n_datei%10;
			LCD_Print(dat_name, 1, ZEILE3, 2, 1, 1, blue, yellow);
			if(FindName(dat_name)==FULL_MATCH) n_datei++;
			else break;
	
		}
		//LCD_Print(dat_name, ZEILE3, 121, 2, 1, 1, blue, yellow);

		
		while(!(PIN_PANA & (1<<NEW_LINE)));    	//warten auf ersten Puls
		while( (PIN_PANA & (1<<NEW_LINE)));    	//warten auf ersten Puls
				
		//LCD_Print("1.Pulse", 1, ZEILE4, 2, 1, 1, blue, yellow);						
		
		
		if(Fopen(dat_name,'w')==F_OK)
		{
		
			LCD_Print("open", 1, ZEILE4, 2, 1, 1, blue, yellow);

			Fwrite(header,61);
				
				

			
		}   
		else 
		{	
		
			LCD_Print("Error opening", 1, ZEILE4, 2, 1, 1, blue, yellow);
			delay_ms(1000);
					
			goto LOOP1;	
		
		}

		while(!(PIN_PANA & (1<<NEW_LINE)));    	//warten auf  Puls
		while( (PIN_PANA & (1<<NEW_LINE)));    	//wegen Zeit für öffnen Datei
		
		LCD_Print("1.pulse", 1, ZEILE5, 2, 1, 1, blue, yellow);
		
		TCNT2=0;
		
		
		while(1)   //warten auf 13ms Puls
		{

			while( !(PIN_PANA & (1<<NEW_LINE)) );    	//L-H
			
			
			c_temp=TCNT2;
			
			

			//U1_Send_Char(PIN_PANA);
			U1_Send_Char(c_temp);
			U1_Send_Char(13);
			
			//itoa(c_temp,tempStr,10);
			//LCD_Print(tempStr, 1, ZEILE6, 2, 1, 1, blue, yellow);

			if(c_temp>PULSE_MIN) 
			{
				if (c_temp < PULSE_MAX) break;		//Pulsbreite im Fenster
			}
			
			while(  (PIN_PANA & (1<<NEW_LINE)) );    	//H-L
			
			TCNT2=0;

		}

		
		
		
		for( z=0;z<N_ZEILE;z++)
		{

			for(pos_write_buff=0;pos_write_buff<154;pos_write_buff++)
			{
				register uint8_t byte=0;

				while(!(PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
				while( (PIN_PANA & 1<<CLOCK));   	//warten auf nächste Taktflanke
		
				if(PIN_PANA & (1<<DATA))  	byt |=   1<<0;
				else						byt &= ~(1<<0);
			
				while(!(PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
				while( (PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
		
				if(PIN_PANA & (1<<DATA)) 	byt |=   1<<1;
				else						byt &= ~(1<<1);
			
				while(!(PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
				while( (PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
		
				if(PIN_PANA & (1<<DATA)) 	byt |=   1<<2;
				else						byt &= ~(1<<2);
			
				while(!(PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
				while( (PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
		
				if(PIN_PANA & (1<<DATA)) 	byt |=   1<<3;
				else						byt &= ~(1<<3);
			
				while(!(PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
				while( (PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
		
				if(PIN_PANA & (1<<DATA)) 	byt |=   1<<4;
				else						byt &= ~(1<<4);
			
				while(!(PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
				while( (PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
		
				if(PIN_PANA & (1<<DATA)) 	byt |=   1<<5;
				else						byt &= ~(1<<5);
			
				while(!(PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
				while( (PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
		
				if(PIN_PANA & (1<<DATA)) 	byt |=   1<<6;
				else						byt &= ~(1<<6);
			
				while(!(PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
				while( (PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
		
				if(PIN_PANA & (1<<DATA)) 	byt |=   1<<7;
				else						byt &= ~(1<<7);						
		
					
				if(pos_write_buff>=19) write_buff[(124+19)-(pos_write_buff)]=byt;		//SIZE_ZEILE

			
			}

			/*

			while(!(PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
			while( (PIN_PANA & 1<<CLOCK));   	//warten auf nächste Taktflanke
	
			if(PIN_PANA & (1<<DATA))  	byt |=   1<<7;
			else						byt &= ~(1<<7);
		
			while(!(PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
			while( (PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
	
			if(PIN_PANA & (1<<DATA)) 	byt |=   1<<6;
			else						byt &= ~(1<<6);
		
			while(!(PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
			while( (PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
	
			if(PIN_PANA & (1<<DATA)) 	byt |=   1<<5;
			else						byt &= ~(1<<5);
		
			
			
			while(!(PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
			while( (PIN_PANA & 1<<CLOCK));    	//warten auf nächste Taktflanke
	
			if(PIN_PANA & (1<<DATA)) 	byt |=   1<<4;
			else						byt &= ~(1<<4);

			*/

			write_buff[125]=0;
			write_buff[126]=0;
			write_buff[127]=0;


			Fwrite(write_buff,128);

			TCNT2=0;
			
			while( !(PIN_PANA & (1<<NEW_LINE)) ) 
			{
			//	c_temp=TCNT2;
			//	if(c_temp>=NEXT_LINE) break;	//Übertragung beendet
					
					
			}   	



			
		//	if(c_temp>=NEXT_LINE) break;	//Übertragung beendet

		
			
		}
		
		/*

		for( ;z<N_ZEILE;z++)
		{
			write_buff[0]=0xff;	
			write_buff[1]=0xff;
			write_buff[2]=0xff;
			write_buff[3]=0xff;
			write_buff[4]=0xff;

			Fwrite(write_buff,156);
		
		
		
		}
		*/


			
		Fclose();

		LCD_Print("Fertig", 1, ZEILE6, 2, 1, 1, blue, yellow);

		delay_ms(2000);

	}
	
	/*	
		

		
		
		
		
		
		
		
		
		
		}				
		
		
			


		
	

		
		

		//interne Uhr starten
		TIFR2  |= (1<<TOV2);           // Interrupts löschen
    	TIMSK2 |= (1<<TOIE2);           // Timer overflow Interrupt freischalten
		
			

		if(flag.pc)
		{
			
			PORTB |= (1<<PB7); //B7 to high = switch display lighting on
			LCD_Init();
			delay_ms(1000);

			switch(menu1())
			{
				case 1:	//transparent Modus

				LCD_Print("Transp. Modus", 1, ZEILE1, 2, 1, 1, blue, yellow);
				LCD_Print("Right  Button", 1, ZEILE2, 2, 1, 1, blue, yellow);
				LCD_Print("to exit", 1, ZEILE3, 2, 1, 1, blue, yellow);

				flag.trans=1;

				U1_Init(MYUBRR1);
				U0_Init(MYUBRR0);

				
				while((PINC & (1<<PC0)));

				flag.trans=0;
		
				goto SLEEP;


				break;

				

				case 2:	

				
				
				U0_Init(MYUBRR0);

				LCD_Print("Push Up   ", 1, ZEILE1, 2, 1, 1, blue, yellow);
				LCD_Print("or Down   ", 1, ZEILE2, 2, 1, 1, blue, yellow);
				LCD_Print("Right  Button", 1, ZEILE3, 2, 1, 1, blue, yellow);
				LCD_Print("to exit", 1, ZEILE4, 2, 1, 1, blue, yellow);
			
				
				
				while(1)
				{
					
					
					if(!(PINC & (1<<PC3))) 
					{
						if(!taste1)
						{
							U0_Send_String("ser\r");
							delay_ms(100);
							U0_Send_String("motd\r");
						}
						taste1=1;
						
					}
					else taste1=0;

					if(!(PINC & (1<<PC4))) 
					{
						if(!taste2)
						{
							U0_Send_String("ser\r");
							delay_ms(100);
							U0_Send_String("motu\r");
						}
						taste2=1;
						
					}
					else taste2=0;
				
				
					if(!(PINC & (1<<PC0))) goto SLEEP;
		
				
				}


				
					
				break;

				case 3:	//beleuchtung ein

				flag.display=1;
				flag.first_time=0;

				//while((PINC & (1<<PC0)));
		
				goto SLEEP;


				break;

				case 4://beleuchtung ein 

				
				flag.display=0;

				//while((PINC & (1<<PC0)));
		
				goto SLEEP;

				
				break;

				
				case 5:

				LCD_Print("Push Up   ", 1, ZEILE1, 2, 1, 1, blue, yellow);
				LCD_Print("or Down   ", 1, ZEILE2, 2, 1, 1, blue, yellow);
				LCD_Print("Right  Button", 1, ZEILE3, 2, 1, 1, blue, yellow);
				LCD_Print("to exit", 1, ZEILE4, 2, 1, 1, blue, yellow);
				itoa(auto_fahn,tempStr,10);
				LCD_Print("    x Fahren", 1, ZEILE6, 2, 1, 1, blue, yellow);
				LCD_Print(tempStr, 1, ZEILE6, 2, 1, 1, blue, yellow);
				

				while(1)
				{
				
					if(!(PINC & (1<<PC3))) 
					{
						if(!taste1)
						{
							if(auto_fahn) auto_fahn--;
							itoa(auto_fahn,tempStr,10);
							LCD_Print("    x Fahren", 1, ZEILE6, 2, 1, 1, blue, yellow);
							LCD_Print(tempStr, 1, ZEILE6, 2, 1, 1, blue, yellow);
						}
						taste1=1;
						
					}
					else taste1=0;

					if(!(PINC & (1<<PC4))) 
					{
						if(!taste2)
						{
							if(auto_fahn<255) auto_fahn++;
							itoa(auto_fahn,tempStr,10);
							LCD_Print("    x Fahren", 1, ZEILE6, 2, 1, 1, blue, yellow);
							LCD_Print(tempStr, 1, ZEILE6, 2, 1, 1, blue, yellow);

						}
						taste2=1;
						
					}
					else taste2=0;
				
					if(!(PINC & (1<<PC0))) goto SLEEP;
				}

				break;
				
				
				case 6:

				wdt_enable(1);

				break;
				
				case 7:

				goto SLEEP;

				break;




				
			
			}

		
		
		



		}

		
		if(flag.uart1)
		{
			flag.trans=1;
			
			U1_Init(MYUBRR1);
			U0_Init(MYUBRR0);

			PORTB |= (1<<PB7); //B7 to high = switch display lighting on
			LCD_Init();
			delay_ms(1000);

			LCD_Cls(yellow);
	
			Orientation = Portrait180; 

			LCD_Print("Transp. Modus", 1, ZEILE1, 2, 1, 1, blue, yellow);
			LCD_Print("Right  Button", 1, ZEILE2, 2, 1, 1, blue, yellow);
			LCD_Print("to exit", 1, ZEILE3, 2, 1, 1, blue, yellow);

		

			while((PINC & (1<<PC0)));

			flag.trans=0;
		
			goto SLEEP;

			

		
		
		}
		
		
	
		
		if(flag.uart0)	//Daten vom Charger
		{
			U0_Init(MYUBRR0);

			if(flag.display)
			{
			
				PORTB |= (1<<PB7); //B7 to high = switch display lighting on
				
				if(!flag.first_time)
				{
					LCD_Init();
					delay_ms(1000);

					LCD_Cls(yellow);
	
					Orientation = Portrait180; 
					flag.first_time=1;
				}
				LCD_Cls(yellow);

				vShowTime(_Stunde, _Min, _Sec);
				vShowDate(_Tag,_Monat,_Jahr);

			}	

		

			
			
			if(flag.new_day)  //prüfen ob neuer Tag-> neue Datei
			{
				
				//Buffer in alte Datei schreiben

				if(pos_write_buff) //Daten im Buffer
				{
					
					switch(SD_Present())
					{
						case NO_SD:
						if(flag.display) 
						{
							LCD_Print("NO SD-CARD   ", 1, ZEILE1, 2, 1, 1, blue, yellow);
							delay_ms(1000);
							
						}
						
						pos_write_buff=0;
						goto SLEEP;		
						break;

						case SD_WE:
						if(flag.display) LCD_Print("SD-CARD WR EN", 1, ZEILE1, 2, 1, 1, blue, yellow);
						
						break;

						case SD_WD:
						if(flag.display) 
						{
							LCD_Print("SD-CARD WR DI", 1, ZEILE1, 2, 1, 1, blue, yellow);
							delay_ms(1000);
							
						}
						pos_write_buff=0;
						goto SLEEP;	
						break;
				
					}
				
					SD_Tristate(1);   //power on sd-card
	

						
					#ifdef MMC_CARD_SPI //SD_CARD_SPI too !
						MMC_IO_Init();
					#endif
	
	

					if(GetDriveInformation()==F_OK) 	
					{
						if(flag.display) LCD_Print("Open SD Ok", 1, ZEILE2, 2, 1, 1, blue, yellow);
					}
					else 	
					{
						if(flag.display) 
						{
							LCD_Print("Error Open SD", 1, ZEILE2, 2, 1, 1, blue, yellow);
							delay_ms(1000);
						}

						pos_write_buff=0;
						goto SLEEP;	
					}
				
								
					if(Fopen(dat_name,'a')==F_OK)
					{
					
						if(flag.display) 
						{	
							LCD_Print(dat_name, 1, ZEILE3, 2, 1, 1, blue, yellow);
							delay_ms(1000);
						}
						Fwrite(write_buff,pos_write_buff);
			
						Fclose();
					}   
					else 
					{	
						if(flag.display)
						{
							LCD_Print("Error opening", 1, ZEILE3, 2, 1, 1, blue, yellow);
							delay_ms(1000);
						}
						pos_write_buff=0;
						
						goto SLEEP;	
					
					}
				
				}
								
				dat_name[0]=0x30+ _Tag/10;
				dat_name[1]=0x30+ _Tag%10;
				dat_name[2]='_';
				dat_name[3]=0x30+ _Monat/10;
				dat_name[4]=0x30+ _Monat%10;;
				dat_name[5]='_';
				dat_name[6]= ID1 ;
				dat_name[7]= ID2 ;
				dat_name[8]='.';
				dat_name[9]='c';
				dat_name[10]='s';
				dat_name[11]='v';
				dat_name[12]='\0';

				if(flag.display) LCD_Print(dat_name, 1, ZEILE3, 2, 1, 1, blue, yellow);

		
				pos_write_buff=0;	//buffer ist leer
				flag.new_day=0;
			}

			
			

			//schreibe Datum

			write_buff[pos_write_buff++]=0x30+ _Tag/10;
			write_buff[pos_write_buff++]=0x30+ _Tag%10;
			write_buff[pos_write_buff++]='.';
			write_buff[pos_write_buff++]=0x30+ _Monat/10;
			write_buff[pos_write_buff++]=0x30+ _Monat%10;;
			write_buff[pos_write_buff++]='.';
			write_buff[pos_write_buff++]='2';
			write_buff[pos_write_buff++]='0';
			write_buff[pos_write_buff++]=0x30+ _Jahr/10;
			write_buff[pos_write_buff++]=0x30+ _Jahr%10;
			write_buff[pos_write_buff++]=' ';
		
			//schreibe Datum Uhrzeit

			write_buff[pos_write_buff++]=0x30+ _Stunde/10;
			write_buff[pos_write_buff++]=0x30+ _Stunde%10;
			write_buff[pos_write_buff++]=':';	
			write_buff[pos_write_buff++]=0x30+ _Min/10;
			write_buff[pos_write_buff++]=0x30+ _Min%10;
			write_buff[pos_write_buff++]=':';
			write_buff[pos_write_buff++]=0x30+ _Sec/10;
			write_buff[pos_write_buff++]=0x30+ _Sec%10;
		
				
			write_buff[pos_write_buff++]=';';		
			
			x_pos=1;
			y_pos=	 ZEILE5;	

			while(1)
			{
				if((ct[0]=U0_Receive_char()))	//ZEICHEN EMFANGEN
				{
					
					if(flag.display)
					{
						LCD_Print(ct,  x_pos, y_pos, 2, 1, 1, blue, yellow);
						x_pos+=8;
						if(x_pos>121)
						{
							x_pos=1;
							y_pos+=ZEILENABSTAND;
						}
					}
					
					if(ct[0]==' ') 
					{
						write_buff[pos_write_buff++]=';'; //ersetze leerzeichen mit semikolon
						x_pos=1;
						y_pos+=ZEILENABSTAND;
					
					}
					else
					{ 
						if(ct[0]==13)
						{
							write_buff[pos_write_buff++]=';';
							write_buff[pos_write_buff++]=ct[0];
							break;
						}
						else
						{ 
							write_buff[pos_write_buff++]=ct[0];	//default
							
						
						}
					}
					
				

				}
		
			}

			if(pos_write_buff<WRITE_NOW)
			{
				if(flag.display)	
				{
					
					delay_ms(1000);
						
				}

				goto SLEEP;

			
			}
			else		//Daten schreiben
			{
				
				switch(SD_Present())
				{
					case NO_SD:
					if(flag.display) 
					{
						LCD_Print("NO SD-CARD   ", 1, ZEILE1, 2, 1, 1, blue, yellow);
						delay_ms(1000);
						
					}
					pos_write_buff=0;
					goto SLEEP;	
				
					break;

					case SD_WE:
					if(flag.display) LCD_Print("SD-CARD WR EN", 1, ZEILE1, 2, 1, 1, blue, yellow);
					
					break;

					case SD_WD:
					if(flag.display) 
					{
						LCD_Print("SD-CARD WR DI", 1, ZEILE1, 2, 1, 1, blue, yellow);
						delay_ms(1000);
						
					}

					pos_write_buff=0;
					goto SLEEP;	
					break;
			
				}
				
				SD_Tristate(1);   //power on sd-card
	

						
				#ifdef MMC_CARD_SPI //SD_CARD_SPI too !
					MMC_IO_Init();
				#endif
	
	

				if(GetDriveInformation()==F_OK) 	
				{
					if(flag.display) LCD_Print("Open SD Ok", 1, ZEILE2, 2, 1, 1, blue, yellow);
				}
				else 	
				{
					if(flag.display) 
					{
						LCD_Print("Error Open SD", 1, ZEILE2, 2, 1, 1, blue, yellow);
						delay_ms(1000);
					}
					pos_write_buff=0;
					goto SLEEP;	
				}
				
								
				if(Fopen(dat_name,'a')==F_OK)
				{
				
					if(flag.display) 
					{
						LCD_Print(dat_name, 1, ZEILE3, 2, 1, 1, blue, yellow);
						delay_ms(1000);					
					}
					Fwrite(write_buff,pos_write_buff);
		
					Fclose();
				}   
				else 
				{	
					if(flag.display)
					{
						LCD_Print("Error opening", 1, ZEILE3, 2, 1, 1, blue, yellow);
						delay_ms(1000);
					}
					pos_write_buff=0;
					goto SLEEP;	
				
				}
			
			
				pos_write_buff=0;	//keine Daten mehr

				
				
				goto SLEEP;
			}


			

	
		
		}

		




	}

	*/
	
}
void delay_ms(uint16_t period)	 //delay routine (milliseconds)
{
	for(unsigned int i=0; i<=period; i++)
		_delay_ms(1);
}



// ----------------------------------------------------------------------------
// ---  Format & Show Time on Display -----------------------------------------
// ----------------------------------------------------------------------------
void vShowTime(uint8_t u8Stunde, uint8_t u8Minute, uint8_t u8Sekunde) 
{
	static char tempStr[20];
	

		itoa(u8Stunde, tempStr, 10);
		if(u8Stunde < 10)
			strcat(tstring, "0");
				
		strcat(tstring,tempStr);
		strcat(tstring, ":");
				
		itoa(u8Minute, tempStr, 10);
		if(u8Minute < 10)
			strcat(tstring, "0");
				
		strcat(tstring,tempStr);
		strcat(tstring, ":");
				
		itoa(u8Sekunde, tempStr, 10);
		if(u8Sekunde < 10)
			strcat(tstring, "0");
				
		strcat(tstring,tempStr);
								
		LCD_Print(tstring, 2, 165, 1, 1, 1, yellow, blue);
				
		// Char-Array to empty!!!
		for(int x = 0; x < 20; x++)
			tstring[x] = 0;
	
}


// ----------------------------------------------------------------------------
// ---  Format & Show Time on Display -----------------------------------------
// ----------------------------------------------------------------------------
void vShowDate(uint8_t u8Tag, uint8_t u8Monat, uint8_t u8Jahr)
{
	static char tempStr[20];
	

		itoa(u8Tag, tempStr, 10);
		if(u8Tag < 10)
			strcat(tstring, "0");
				
		strcat(tstring,tempStr);
		strcat(tstring, ".");
				
		itoa(u8Monat, tempStr, 10);
		if(u8Monat < 10)
			strcat(tstring, "0");
				
		strcat(tstring,tempStr);
		strcat(tstring, ".");
				
		itoa(u8Jahr, tempStr, 10);
		if(u8Jahr < 10)
			strcat(tstring, "0");
				
		strcat(tstring,tempStr);
								
		LCD_Print(tstring, 80, 165, 1, 1, 1, yellow, blue);
				
		// Char-Array to empty!!!
		for(int x = 0; x < 20; x++)
			tstring[x] = 0;
	
}


void cmd(void)
{}

uint8_t menu1(void)
{
	uint8_t pos=1, taste=1;
	
	LCD_Cls(yellow);
	
	Orientation = Portrait180; 
			
	while(1)
	{
		if (pos==1) LCD_Print("Transpar.  Mod", 1,0*17, 2, 1, 1, yellow, blue);
		else 		LCD_Print("Transpar.  Mod", 1,0*17, 2, 1, 1, blue, yellow);

		if (pos==2) LCD_Print("Motor fahren  ", 1,1*17, 2, 1, 1, yellow, blue);
		else 		LCD_Print("Motor fahren  ", 1,1*17, 2, 1, 1, blue, yellow);

		if (pos==3) LCD_Print("Display ein   ", 1,2*17, 2, 1, 1, yellow, blue);
		else 		LCD_Print("Display ein   ", 1,2*17, 2, 1, 1, blue, yellow);

		if (pos==4) LCD_Print("Display aus   ", 1,3*17, 2, 1, 1, yellow, blue);
		else 		LCD_Print("Display aus   ", 1,3*17, 2, 1, 1, blue, yellow);

		if (pos==5) LCD_Print("Autom Fahren  ", 1,4*17, 2, 1, 1, yellow, blue);
		else 		LCD_Print("Autom Fahren  ", 1,4*17, 2, 1, 1, blue, yellow);
		
		if (pos==6) LCD_Print("Reset         ", 1,5*17, 2, 1, 1, yellow, blue);
		else 		LCD_Print("Reset         ", 1,5*17, 2, 1, 1, blue, yellow);

		if (pos==7) LCD_Print("Ende          ", 1,6*17, 2, 1, 1, yellow, blue);
		else 		LCD_Print("Ende          ", 1,6*17, 2, 1, 1, blue, yellow);
	

		if(taste) taste=0;
		else
		{
			if (!(PINC & (1<<PC4)))
			{
				taste=1;
				if(pos>1) pos--;

			}
			if (!(PINC & (1<<PC3)))
			{
				taste=1;
				if(pos<7) pos++;

			}

			if (!(PINC & (1<<PC6))) break;

		}

		
		delay_ms(100);
		vShowTime(_Stunde, _Min, _Sec);
		vShowDate(_Tag,_Monat,_Jahr);

	}

	LCD_Cls(yellow);

	return pos;

}


