/*
 * FILE: lab14-2-solution.c
 *
 * Push buttons B0 to B3 are connected to port C, pins 0 to 3.
 * Button B0 (pin C0) increases the frequency
 * Button B1 (pin C1) decreases the frequency
 * Button B2 (pin C2) increases the duty cycle
 * Button B3 (pin C3) decreases the duty cycle
 *
 * A piezo buzzer and an LED should both be connected to the OC1B pin
 */
#include <avr/io.h>
#define F_CPU 8000000UL	// 8MHz
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>


uint16_t freq_to_clock_period(uint16_t freq) {
	return (1000000UL / freq);	// UL ensures 32-bit arithmetic
}


uint16_t duty_cycle_to_pulse_width(float dutycycle, uint16_t clockperiod) {
	return (dutycycle * clockperiod) / 100;
}

void make_sound(uint16_t frequency) { // Changed return type to void
	float dutycycle = 50.0;	// % Duty Cycle for a clear tone
	uint16_t clockperiod = freq_to_clock_period(frequency);
	uint16_t pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);
	
	OCR1A = clockperiod - 1;

	// Set the count compare value based on the pulse widths
	if(pulsewidth == 0) {
		OCR1B = 0;
		} else {
		OCR1B = pulsewidth - 1;
	}

	// Set up timer/counter 1 for Fast PWM, counting from 0 to OCR1A
	// Configure output OC1B to be clear on compare match and set on timer/counter overflow (non-inverting mode)
	TCCR1A = (1 << COM1B1) | (0 << COM1B0) | (1 << WGM11) | (1 << WGM10);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (0 << CS12) | (1 << CS11) | (0 << CS10);
	
}

void end_music(void) {
	TCCR1B &= ~(1 << CS12);
	TCCR1B &= ~(1 << CS11);
	TCCR1B &= ~(1 << CS10);
	
	PORTD &= ~(1 << 4);
}
