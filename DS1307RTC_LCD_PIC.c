
// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = ON        // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = ON         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#define _XTAL_FREQ 20000000UL

#define RS PORTBbits.RB0
#define EN PORTBbits.RB1
#define RW PORTBbits.RB2

 // RTC macros for DS1307
#define DS1307_address 0xD0
#define second            0x00
#define minute            0x01
#define hr                0x02
#define day1              0x03
#define date1             0x04
#define month1            0x05
#define year1             0x06
#define control_reg       0x07 

void DS1307_WRITE(char sec,char min,char hour,char day,char date,char month,char year);
void DS1307_READ(char slave_address,char register_address);
//void delay(unsigned int delay);
void bcd_to_ascii(unsigned char value);
int decimal_to_bcd(unsigned char value);
void lcd_cmd(unsigned char cmd);
void lcd_data(unsigned char data);

void lcd_word(unsigned char*words);
unsigned char _sec,_min,_hour,_day,_date,_month,_year,_con;
void main(void)
{
    //all I2C port pins as a input
    TRISC=0xFF;
    // how 49 ,   clock = (Fosc/((4*SSPADD) +1))
    //100 = 20MHZ/(4*SSPADD +1)
    SSPADD = 49;
    //PCON register of pic16f877a  8-bit register
    // FOR MASTER MODE AND ENABLE SSPEN , PCON = 00101000=0x28
   SSPCON = 0x28;
   PORTD = 0x00;
   //define direction
   TRISD = 0x00;
   TRISB=0x00;
   PORTB =0x00;
  
  //lcd cmd
   lcd_cmd(0x38);
   lcd_cmd(0x02);
   lcd_cmd(0x01);
   lcd_cmd(0x0c);
   lcd_cmd(0x06);
   lcd_cmd(0x80);   // LCD CMD of first row
   lcd_word("clock :");
   lcd_cmd(0xc0);   // LCD CMD of second row
   lcd_word("date:");
   DS1307_WRITE(45,11,22,1,31,3,24);
   __delay_ms(300);
   while(1)
   {
       __delay_ms(20);
       DS1307_READ(DS1307_address,0x00);
   }
}
  void bcd_to_ascii(unsigned char value)
  {
      unsigned char bcd;
      bcd=value;
      bcd=bcd&0xf0;
      bcd=bcd>>4;
      bcd=bcd|0x30;
      lcd_data(bcd);
      bcd=value;
      bcd=bcd&0x0f;
      bcd=bcd|0x30;
      lcd_data(bcd);
  }
  int decimal_to_bcd(unsigned char value)
  {
      unsigned char msb,lsb,hex;
      msb=value/10;
      lsb=value%10;
      hex = (msb<<4)+lsb;
      return hex;
  }
  void DS1307_WRITE(char sec,char min,char hour,char day,char date,char month,char year)
  {
      //start bit
      SSPCON2bits.SEN=1;
      //initialize start condition
      while(SEN); //wait for start condition to complete
      PIR1bits.SSPIF=0; //clear SSPIF flag
      //slave address bits
      SSPBUF = DS1307_address;
      //send the slave address high r/w = 0 for write
      while(!SSPIF);  //wait for acknowledge SSPIF is set for every 9th clock
      PIR1bits.SSPIF = 0;
      if(SSPCON2bits.ACKSTAT)
      {
          SSPCON2bits.PEN = 1; //STOP BITS
          while(PEN);
          return ;
      }
      //for second
       /*SSPBUF = second;
      //send the slave address high r/w = 0 for write
      while(!SSPIF);  //wait for acknowledge SSPIF is set for every 9th clock
      PIR1bits.SSPIF = 0;
      if(SSPCON2bits.ACKSTAT)
      {
          SSPCON2bits.PEN = 1; //STOP BITS
          while(PEN);
          return ;
      }*/
      SSPBUF = decimal_to_bcd(sec); // send the data
      while(!SSPIF);  //acknowledge
      PIR1bits.SSPIF =0 ; //clear SSPIF flag
      // for minute
     
      SSPBUF = decimal_to_bcd(min);
      while(!SSPIF);  //acknowledge
      PIR1bits.SSPIF=0; //clear flag
      // for hour
      
      SSPBUF = decimal_to_bcd(hour);
      while(!SSPIF);               //acknowledge
      PIR1bits.SSPIF =0;
      
      // for day
     
      SSPBUF = decimal_to_bcd(day);
      while(!SSPIF);
      PIR1bits.SSPIF =0;
      // for date
      
      SSPBUF = decimal_to_bcd(date);
      while(!SSPIF);
      PIR1bits.SSPIF =0;
      //for month
      
      SSPBUF = decimal_to_bcd(month);
      while(!SSPIF);
      PIR1bits.SSPIF =0;
      // for year
     
      SSPBUF = decimal_to_bcd(year);
      while(!SSPIF);
      PIR1bits.SSPIF =0;
      //send the data
      SSPBUF = 0x00;
      while(!SSPIF);
      PIR1bits.SSPIF=0; //clear SSPIF flag
      //stop bit
      SSPCON2bits.PEN = 1;
      while(PEN); //complete stop(writing is finished)
    }
  void DS1307_READ(char slave_address,char register_address)
  {
      //start reading
      SSPCON2bits.SEN = 1;
      while(SEN);
      PIR1bits.SSPIF =0;
      //slave address bit
      SSPBUF = slave_address;
      while(!SSPIF);  // ACKNOWLEDGE
      PIR1bits.SSPIF =0;
      if(SSPCON2bits.ACKSTAT) 
      {
          SSPCON2bits.PEN = 1;
          while(PEN);
          return;
      }
      //slave address
      SSPBUF = register_address;
      while(!SSPIF);
      PIR1bits.SSPIF =0;
      if(SSPCON2bits.ACKSTAT) 
      {
          SSPCON2bits.PEN =1 ;
          while(PEN);
          return;
      }
      //repeated start
      SSPCON2bits.RSEN = 1;
      while(RSEN);
      PIR1bits.SSPIF = 0;
      //slave address bits
      SSPBUF = (slave_address +1);
      while(!SSPIF);
      PIR1bits.SSPIF = 0;
      if(SSPCON2bits.ACKSTAT)
      {
          SSPCON2bits.PEN =1;
          while(PEN);
          return;
      }
      //receive enable
      SSPCON2bits.RCEN=1;
      while(!SSPSTATbits.BF);
      _sec = SSPBUF;
      SSPCON2bits.ACKDT=0;   // SEND NECK
      SSPCON2bits.ACKEN = 1; 
      while(ACKEN);
       //receive enable MINUTE
      SSPCON2bits.RCEN=1;
      while(!SSPSTATbits.BF);
      _min = SSPBUF;
      SSPCON2bits.ACKDT=0;   // SEND NECK
      SSPCON2bits.ACKEN = 1;
      while(ACKEN);
    
      //receive enable HOUR
      SSPCON2bits.RCEN=1;
      while(!SSPSTATbits.BF);
      _hour = SSPBUF;
      SSPCON2bits.ACKDT=0;   // SEND NECK
      SSPCON2bits.ACKEN = 1; 
      while(ACKEN);
      
      // for DAY
     //receive enable
      SSPCON2bits.RCEN=1;
      while(!SSPSTATbits.BF);
      _day = SSPBUF;
      SSPCON2bits.ACKDT=0;   // SEND NECK
      SSPCON2bits.ACKEN = 1;
      while(ACKEN);
      // for DATE
     //receive enable
      SSPCON2bits.RCEN=1;
      while(!SSPSTATbits.BF);
      _date = SSPBUF;
      SSPCON2bits.ACKDT=0;   // SEND NECK
      SSPCON2bits.ACKEN = 1; 
      while(ACKEN);
     // for MONTH
     //receive enable
      SSPCON2bits.RCEN=1;
      while(!SSPSTATbits.BF);
      _month = SSPBUF;
      SSPCON2bits.ACKDT=0;   // SEND NECK
      SSPCON2bits.ACKEN = 1;
      while(ACKEN);
      
      // for YEAR
      //receive enable
      SSPCON2bits.RCEN=1;
      while(!SSPSTATbits.BF);
      _year = SSPBUF;
      SSPCON2bits.ACKDT=0;   // SEND NECK
      SSPCON2bits.ACKEN = 1;  
      while(ACKEN);
      
        // for con
      //receive enable
      SSPCON2bits.RCEN=1;
      while(!SSPSTATbits.BF);
      _con = SSPBUF;
      SSPCON2bits.ACKDT=1;   // SEND NECK
      SSPCON2bits.ACKEN = 1;  
      while(ACKEN);
      
      // stop bit
      SSPCON2bits.PEN = 1;
      while(PEN);
      // LCD 
      lcd_cmd(0x87);
      bcd_to_ascii(_hour);
      lcd_word(":");
      bcd_to_ascii(_min);
      lcd_word(":");
      bcd_to_ascii(_sec);
      lcd_cmd(0xc5);
      bcd_to_ascii(_date);
      lcd_word("/");
      bcd_to_ascii(_month);
      lcd_word("/");
      bcd_to_ascii(_year);
    }
void lcd_cmd(unsigned char cmd)
{
   PORTD = cmd;
   RS =0;
   EN=1;
   __delay_ms(1);
   EN=0;
 }
void lcd_data(unsigned char data)
{
   PORTD = data;
   RS =1;
   EN=1;
   __delay_ms(1);
   EN=0; 
  
}
void lcd_word(unsigned char*words)
{
    unsigned int loop =0;
    for(loop=0;words[loop]!= '\0';loop++)
    {
        lcd_data(words[loop]);
    }
}
