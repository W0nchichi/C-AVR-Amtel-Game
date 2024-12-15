// FileSaver.h

#ifndef FILESAVER_H
#define FILESAVER_H

#include <stdint.h>
#include <stdbool.h>

// Define EEPROM signature value
#define EEPROM_SIG_VALUE 0xA  // Choose a unique signature

// Declare EEPROM variables with EEMEM attribute
#include <avr/eeprom.h>

// Function prototypes
void save_game(uint8_t level, uint16_t num_steps, uint32_t level_time);
bool load_game(uint8_t *level, uint16_t* num_steps, uint32_t* level_time);
void delete_game(void);

#endif // FILESAVER_H
