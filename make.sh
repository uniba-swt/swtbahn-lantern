avr-gcc -g -O1 -mmcu=attiny84 -c src/lantern.c

avr-gcc -g -mmcu=attiny84 -o lantern.elf lantern.o

avr-objcopy -j .text -j .data -O ihex lantern.elf lantern.hex

avrdude -c stk500v2 -P /dev/ttyACM0 -p attiny84 -U flash:w:lantern.hex
