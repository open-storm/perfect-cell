/**
 * @file sensors_uart_control.h
 * @brief A collection of common patterns used to interface with the sensors
 * UART.
 * @author Ivan Mondragon
 * @version TODO
 * @date 2017-07-16
 */
#ifndef SENSORS_UART_CONTROL_H
#define SENSORS_UART_CONTROL_H

#include <project.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/**
 * @brief Starts the sensors UART with generic interrupt service.
 */
void sensors_uart_start();

/**
 * @brief Stops the sensors UART and generic interrupt service.
 */
void sensors_uart_stop();

/**
 * @brief Sets the baud rate for the sensors uart.
 *
 * @param baud One of the standard baud rates. [1200, 2400, 4800, 9600, 14400,
 * 19200, 38400, 57600, 115200, 128000, or 256000]
 *
 * @return 1u on success, 0u otherwise.
 */
uint8_t sensors_uart_set_baud(const uint32_t baud);

/**
 * @brief Get the uart received string.
 *
 * @return Pointer to the string in the buffer.
 */
char* sensors_uart_get_string();

/**
 * @brief Get the size of the string in the buffer.
 *
 * @return Size of string in buffer.
 */
size_t sensors_uart_get_string_size();

/**
 * @brief Clears internal buffer.
 */
void sensors_uart_clear_string();

CY_ISR_PROTO(isr_sensors_uart_rx);

#endif
