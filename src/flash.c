// vim: tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab
/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2013 Damian Miller <damian.m.miller@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/flash.h>

#include "flash.h"
#include "hid.h"


#define FLASH_PAGE_NUM_MAX 127
#define FLASH_PAGE_SIZE 0x800

// thanks to: https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f1/stm32-h107/flash_rw_example/flash_rw_example.c

uint32_t flash_program_data(uint32_t start_address, uint8_t *input_data, uint16_t num_elements)
{
	uint16_t iter;
	uint32_t current_address = start_address;
	uint32_t page_address = start_address;
	uint32_t flash_status = 0;

	/*check if start_address is in proper range*/
	//if((start_address - FLASH_BASE) >= (FLASH_PAGE_SIZE * (FLASH_PAGE_NUM_MAX+1)))
	//	return 1;

	/*calculate current page address*/
	if(start_address % FLASH_PAGE_SIZE)
		page_address -= (start_address % FLASH_PAGE_SIZE);

	flash_unlock();

	/*Erasing page*/
	flash_erase_page(page_address);
	flash_status = flash_get_status_flags();
	if(flash_status != FLASH_SR_EOP)
		return flash_status;

	/*programming flash memory*/
	for(iter=0; iter<num_elements; iter += 4)
	{
		/*programming word data*/
		flash_program_word(current_address+iter, *((uint32_t*)(input_data + iter)));
		flash_status = flash_get_status_flags();
		if(flash_status != FLASH_SR_EOP)
			return flash_status;

		/*verify if correct data is programmed*/
		if(*((uint32_t*)(current_address+iter)) != *((uint32_t*)(input_data + iter)))
			return FLASH_WRONG_DATA_WRITTEN;
	}

	return 0;
}

void flash_read_data(uint32_t start_address, uint16_t num_elements, uint8_t *output_data)
{
	uint16_t iter;
	uint32_t *memory_ptr= (uint32_t*)start_address;

	for(iter=0; iter<num_elements/4; iter++)
	{
		*(uint32_t*)output_data = *(memory_ptr + iter);
		output_data += 4;
	}
}



