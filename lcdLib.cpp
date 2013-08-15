#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>

#define setE        PORTC |= 0x10
#define clrE        PORTC &= ~0x10
#define setRS        PORTC |= 0x20
#define clrRS        PORTC &= ~0x20

void lcdInit();
void lcdChr(char ch);
void lcdStr(char *str);
void nextbyte();
void sendbyte(unsigned char ch);
void send4bit(unsigned char ch);
void sendPulse();

struct {
        struct {
                unsigned h:1;
                unsigned c:1;
                unsigned p:1;
        }flags;
        unsigned char sbyte;
        char buf[48];
        int inx;
        int count;
}myLcd;

void lcdInit()
{
        DDRC = 0x3f;
        PORTC = 0;
     TCCR0A = 3;
     TCCR0B = 8;
     OCR0A = 40;
     OCR0B = 2;   
     TIMSK0 = 6;
     myLcd.inx = 0;
     myLcd.count = 0;
     _delay_ms(50);
     send4bit(3);
     _delay_ms(5);
     send4bit(3);
     _delay_ms(5);
     send4bit(3);
     _delay_us(200);
     send4bit(2);
     _delay_us(200);
     myLcd.flags.c = 1;
     sendbyte(0x28);
     _delay_ms(2);
     myLcd.flags.c = 1;
     sendbyte(0x0F);
     _delay_ms(2);
     myLcd.flags.c = 1;
     sendbyte(0x06);
     _delay_ms(2);
     myLcd.flags.c = 1;
     sendbyte(0x01);
     _delay_ms(2);
     myLcd.flags.c = 1;
     sendbyte(0x80);
     _delay_ms(2);
     setRS;
}
void lcdChr(char ch)
{
        if (myLcd.count >= 47) return;
        myLcd.buf[myLcd.inx] = ch;
        myLcd.inx = (myLcd.inx < 47)?myLcd.inx + 1:0;
        myLcd.count++;
        if (myLcd.flags.p == 0) nextbyte();
}
void lcdStr(char *str)
{        
        while((myLcd.count < 47) && (*str != 0)) {
                lcdChr(*str);
                str++;
        }
}

void nextbyte()
{
int sbf;
char ch1;

        sbf = (myLcd.count <= myLcd.inx)? myLcd.inx - myLcd.count:(myLcd.inx + 48) - myLcd.count;
        ch1 = myLcd.buf[sbf];
        if (ch1 == 27) {
                myLcd.flags.c = 1;
                myLcd.count--;
                if (myLcd.count > 0) nextbyte();
        } else {
                myLcd.count--;
                sendbyte(ch1);
        }

}
void sendbyte(unsigned char ch)
{
        if (myLcd.flags.c) clrRS; else setRS;
        myLcd.sbyte = ch;
        myLcd.flags.h = 1;
        send4bit(ch >> 4);
}
void send4bit(unsigned char ch)
{
        ch &= 0x0f;
        PORTC &= 0xF0;
        PORTC |= ch;
        sendPulse();
}
void sendPulse()
{
        myLcd.flags.p = 1;
        setE;
        TCCR0A = 3;
        OCR0A = 40;
        OCR0B = 2;   
        TIMSK0 = 6;
        TCNT0 = 0;
        TCCR0B = 11;                // старт таймера
}
ISR(TIMER0_COMPB_vect)
{
        clrE;
}
ISR(TIMER0_COMPA_vect)
{
        TCCR0B &= 0xF8;
        myLcd.flags.p = 0;
        if (myLcd.flags.h) {
                myLcd.flags.h = 0;
                if (myLcd.flags.c) myLcd.flags.c = 0;
                send4bit(myLcd.sbyte);
                return;
        }
        if (myLcd.count > 0) nextbyte();
}

