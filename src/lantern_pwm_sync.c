#define F_CPU 8000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile enum {NORMAL, REVERSE, ERROR} state;
volatile int counter = 0;
volatile int pos = 0;

int sine[16] = { 50, 60, 70, 80, 90, 80, 70, 60, 50, 40, 30, 20, 10, 20, 30, 40};
int straight[16] = { 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90};
int pulse[16] = { 90, 90, 90, 90, 0, 0, 0, 0, 90, 90, 90, 90, 0, 0, 0, 0};

int waveFormSelect();
void clearSyncTimer();

int main(void){
//DataDirections - I/O
  DDRB |= (1 << PB2);  //Output: LED
  DDRA |= (0 << PA0);  //Input: Reverse
  DDRA |= (0 << PA1);  //Input: Normal
  DDRA |= (0 << PB1); //Input: CLK PB1

//Interrupts
  GIMSK |= (1 << PCIE1) | (1 << PCIE0); //General Interrupt Mask
  PCMSK1 |= (1 << PCINT9);//Pin Change Interrupt
  PCMSK0 |= (1 << PCINT0) | (1 << PCINT1);

//PWM
  TCCR0A |= (1 << COM0A1) | (1 << COM0A0) | (1 << WGM00) | (1 << WGM01); //Timer/Counter Control
  TCCR0B |= (1 << WGM02) |
  TIMSK0 |= (1 << OCF0A); //Timer Interrupt Mask

//Sync timer
  //TCCR1A = ;
  TCCR1B |= (1 << WGM02);
  TIMSK1 |= (1 << TOIE1);


  sei();
  // Prescaler: after setting bit timer starts
  TCCR0B |= (1 << CS00); //8-bit-timer prescaler
  TCCR1B |= (1<<CS11) | (1<<CS10);//16-bit-timer prescaler 64


  while(1){
  }

}

ISR(TIMER0_COMPA_vect){
  //PWM timer
  OCR0A = (waveFormSelect() / 100) * 255;
}

ISR(TIMER1_OVF_vect){
  //PWM timer
  if(pos <= 16){
      pos++;
  }
  else{
    pos = 0;
  }
}

ISR(PCINT0_vect){
  //Servo input (pin-change)
  if(PINA & (1 << PA0)){
    state = REVERSE;
  }
  else if(PINA & (1 << PA1)) {
    state = NORMAL;
  }
  else{
    state = ERROR;
  }
}
ISR(PCINT1_vect){
  //Clock input (pin-change)
  if(PINB & (1 << PB1){
      pos = 0;
      clearSyncTimer();
      }
}

int waveFormSelect(){
  switch(state){
    case (REVERSE): return sine[pos];
      break;
    case (NORMAL): return straight[pos];
      break;
    case (ERROR):
    default: return pulse[pos];
      break;
  }
}

void clearSyncTimer(){
  //16-bit timer
  unsigned char sreg = SREG;
  _cli();
  TCNT1H = 0x00;
  TCNT1L = 0x00;
  SREG = sreg;
  sei();
}
