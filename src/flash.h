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

#define FLASH_WRONG_DATA_WRITTEN 0x80
#define RESULT_OK 0

uint32_t flash_program_data(uint32_t start_address, uint8_t *input_data, uint16_t num_elements);
void flash_read_data(uint32_t start_address, uint16_t num_elements, uint8_t *output_data);

