#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>

#define HIGH 0x1
#define LOW 0x0
#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define A0 18
#define A1 19
#define A2 20
#define A3 21
#define A4 22
#define A5 23

#define constrain(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))
#define F(x) (x)

static inline int abs(int x) { return x < 0 ? -x : x; }

extern "C" {
void delay(unsigned long ms);
unsigned long millis(void);
unsigned long micros(void);
int analogRead(uint8_t pin);
int digitalRead(uint8_t pin);
void digitalWrite(uint8_t pin, uint8_t value);
void analogWrite(uint8_t pin, int value);
void pinMode(uint8_t pin, uint8_t mode);
}

struct SerialSim {
    void begin(unsigned long) {}
    template <typename T> void print(T) {}
    template <typename T> void println(T) {}
};
extern SerialSim Serial;

#endif
