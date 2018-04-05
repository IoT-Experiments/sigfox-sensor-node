#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "USART2313.h"

#define PIN_SENSOR_MOTOR 9 //PB0
#define PCIE_SENSOR_MOTOR PCINT0
#define PIN_SENSOR_BUTTON 10 // PB1
#define PCIE_SENSOR_BUTTON PCINT1
#define PIN_SIGFOX_EMITTER_RESET 8 // PD6
#define PIN_DEBUG_LED 7 // PD5

#define DEBOUNCE_SENSOR_MS 2000L
#define BUTTON_RESET_COUNTER_MS 1500L

volatile unsigned long g_lastCountMillis = 0;
volatile uint16_t g_counter = 0;

USART2313 mySerial;

void sendSigfoxMessage(uint16_t counterValue);
void sleep();

//////////

void setup() {
  pinMode(PIN_SENSOR_MOTOR, INPUT);
  digitalWrite(PIN_SENSOR_MOTOR, HIGH); // Enable pullup resistor
  pinMode(PIN_SENSOR_BUTTON, INPUT);
  digitalWrite(PIN_SENSOR_BUTTON, HIGH); // Enable pullup resistor

  pinMode(PIN_SIGFOX_EMITTER_RESET, INPUT); // - To let it float
  pinMode(PIN_DEBUG_LED, OUTPUT);

  mySerial.begin(9600, true);
  mySerial.println("AT");
  mySerial.println("AT");
  sendSigfoxMessage(0);
}

void loop() {
  // - In SLEEP_MODE_PWR_DOWN, the timer is stopped.
  // We cannot go to sleep before the debounce timer
  if(millis() > g_lastCountMillis + DEBOUNCE_SENSOR_MS + 100) {
    digitalWrite(PIN_DEBUG_LED, LOW);
    sleep();
  }

  if(digitalRead(PIN_SENSOR_BUTTON) == LOW) {
    delay(BUTTON_RESET_COUNTER_MS);

    // - Sends the last counted value
    if(g_counter != 0) { // - Avoid multiple messages when the sensor is long pressed
      sendSigfoxMessage(g_counter);
    }

    // - Reset the counter on long press
    if(digitalRead(PIN_SENSOR_BUTTON) == LOW) {
      if(g_counter != 0) { // - Avoid multiple messages when the sensor is long pressed
        sendSigfoxMessage(0);
      }
      g_counter = 0;
      g_lastCountMillis = millis();
    }
  }
}

void sleep() {
  //ADCSRA &= ~_BV(ADEN); //Disable ADC ==> Not available on ATTiny2313

  // Enable interrupts
  // INT0 and INT1 cannot be used to wake up from power down on level change
  // Only PCINT7..0 can wake up from power down
  GIMSK |= _BV(PCIE);
  PCMSK |= _BV(PCIE_SENSOR_MOTOR);
  PCMSK |= _BV(PCIE_SENSOR_BUTTON);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sei();
  sleep_cpu();
  cli();
  sleep_disable();
  sei();
}

void sendSigfoxMessage(uint16_t counterValue) {
  static char table[] = "0123456789ABCDEF";
  // Reset Sigfox module
  pinMode(PIN_SIGFOX_EMITTER_RESET, OUTPUT);
  digitalWrite(PIN_SIGFOX_EMITTER_RESET, LOW);
  delay(500);
  pinMode(PIN_SIGFOX_EMITTER_RESET, INPUT); // - To let it float
  digitalWrite(PIN_SIGFOX_EMITTER_RESET, HIGH); // Enable pullup resistor
  delay(1000);
  // - Avoiding sprintf(buffer, "%s%d", "AT$SF=", g_counter) : too large program size
  // Moreover, the data should be in hexa with left padding '0'
  if(counterValue == 0) {
    mySerial.println("AT$SF=00");
  } else {
    char bufferNumberStr[24];
    uint16_t value = counterValue;
    int i = 0;
    while(value) {
      bufferNumberStr[i++] = table[value % 16];
      value /= 16;
    }
    mySerial.print("AT$SF=");
    if(i % 2 != 0) { // Left padding '0' to have bytes in hex
      mySerial.write('0');
    }
    // - Reverse the result to send it
    while(i > 0) {
      mySerial.write(bufferNumberStr[i - 1]);
      i--;
    }
    // CR
    mySerial.write(0x0D);
    mySerial.write(0x0A);
  }

  delay(5000); // wait for message departure
  mySerial.println("AT$P=2"); // Deep sleep mode
  delay(200); // wait for all data to be sent
}

ISR(PCINT_vect) {
  digitalWrite(PIN_DEBUG_LED, HIGH);

  if ( (PINB & _BV(PCIE_SENSOR_BUTTON)) == LOW ) {
    // - Managed in loop
  } else if ( (PINB & _BV(PCIE_SENSOR_MOTOR)) == LOW ) {
    if(millis() > g_lastCountMillis + DEBOUNCE_SENSOR_MS) {
      g_counter++;
      g_lastCountMillis = millis();
    }
  }
}
