#define F_CPU 8000000
#include <avr/io.h>
#include <util/delay.h>


void readState(void);

void lanternConstant(void);
void lanternSoftBlink(void);
void lanternHardBlink(void);
void lanternFastBlink(void);

volatile enum {NORMAL, REVERSE, MOVING, ERROR} state;

int main(void){

  DDRB = (1 << PB2);  //Output: LED
  DDRA = (0 << PA0);  //Input: Reverse
  DDRA = (0 << PA1);  //Input: Normal
  //Input: CLK PB1

  while(1){
    readState();

    switch (state) {
       case (NORMAL):           lanternConstant();
                                break;
       case (REVERSE):          lanternSoftBlink();
                                break;
       case (MOVING):           lanternHardBlink();
                                break;
       case (ERROR):
       default:                 lanternFastBlink();
                                break;
    }
  }

}

void readState(void) {
  if (((PINA >> PINA0) & 0x01) && !((PINA >> PINA1) & 0x01)) {
    state = REVERSE;
  } else if (!((PINA >> PINA0) & 0x01) && ((PINA >> PINA1) & 0x01)) {
    state = NORMAL;
  } else if (!((PINA >> PINA0) & 0x1) && !((PINA >> PINA1) & 0x01)) {
    state = MOVING;
  } else {
    state = ERROR;
  }
}

void lanternConstant(void) {
  PORTB |= (1 << PB2);
}

void lanternSoftBlink(void) {
  PORTB |= (1 << PB2);
  _delay_ms(20);
  PORTB ^= (1 << PB2);
  _delay_ms(180);
}

void lanternHardBlink(void) {
  PORTB ^= (1 << PB2);
  _delay_ms(100);
}

void lanternFastBlink(void) {
  PORTB ^= (1 << PB2);
  _delay_ms(10);
}
