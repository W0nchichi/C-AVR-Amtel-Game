// FileSaver.c

#include "FileSaver.h"
#include "game.h"
#include <avr/eeprom.h>           


void save_game(uint8_t level, uint16_t num_steps, uint32_t level_time)
{
	uint16_t address = 0;
	eeprom_update_word((uint16_t*) address, EEPROM_SIG_VALUE);
	address += sizeof(uint16_t);
	
	eeprom_update_byte((uint8_t*) address, level);
	address += sizeof(uint8_t);
	
	eeprom_update_byte((uint8_t*) address, player_row);
	address += sizeof(uint8_t);
	
	eeprom_update_byte((uint8_t*) address, player_col);
	address += sizeof(uint8_t);
	
	eeprom_update_block(&num_steps, (void*)address, sizeof(num_steps));
	address += sizeof(num_steps);
	
	// Save level_time
	eeprom_update_block(&level_time, (void*)address, sizeof(level_time));
	address += sizeof(level_time);
	
	eeprom_update_block((const void*)board, (void*)(address), sizeof(board));
}



bool load_game(uint8_t *level, uint16_t* num_steps, uint32_t* level_time)
{
	uint16_t address = 0;
	uint16_t sig = eeprom_read_word((uint16_t*)address);
	address += sizeof(uint16_t);
	
	if(sig != EEPROM_SIG_VALUE){
		return false;
	}
	
	*level = eeprom_read_byte((uint8_t*) address);
	address += sizeof(uint8_t);
	
	player_row = eeprom_read_byte((uint8_t*) address);
	address += sizeof(uint8_t);
	
	player_col = eeprom_read_byte((uint8_t*) address);
	address += sizeof(uint8_t);
	
	 eeprom_read_block(num_steps, (const void*)address, sizeof(*num_steps));
	 address += sizeof(*num_steps);
	 
	 // Read level_time
	 eeprom_read_block(level_time, (const void*)address, sizeof(*level_time));
	 address += sizeof(*level_time);
	
	eeprom_read_block((void*)board, (const void*)(address), sizeof(board));
	
	return true;
}


void delete_game(void)
{
	uint16_t invalid_sig = 0xFF;
	eeprom_update_word((uint16_t*)0, invalid_sig);
}
