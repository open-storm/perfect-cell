/**
 * @file valve.h
 * @brief Declares functions for operating valve
 * @author Brandon Wong
 * @version TODO
 * @date 2017-06-19
 */

#include <project.h>
#include "misc.h"

/**
 * @brief Move valve to the position indicated by @p valve
 *
 * @param Position to move the valve:
 * - 0: Completely open
 * - 100: Completely closed
 * - 1-99: Partially open
 *
 * @return Number of iterations needed to close valve. TODO: Unclear.
 */
int move_valve(int valve);

/**
 * @brief Test the valve by opening and closing it.
 *
 * @return 1
 */
int test_valve();

/**
 * @brief Read the current position of the valve
 *
 * @return Position of the valve as a percent, with 0 meaning completely open
 * and 100 meaning completely closed.
 */
float32 read_Valve_POS();

/**
 * @brief Moves first valve and inserts current value of valve_trigger into labels and readings arrays.
 *
 * @param labels Array to store labels corresponding to each trigger result
 * @param readings Array to store trigger results as floating point values
 * @param array_ix Array index to label and readings
 * @param max_size Maximum size of label and reading arrays (number of entries)
 *
 * @return (*array_ix) + number of entries filled
 */
uint8 zip_valve(char *labels[], float readings[], uint8 *array_ix, int *valve_trigger, uint8 max_size);

/**
 * @brief Moves second valve and inserts current value of valve_2_trigger into labels and readings arrays.
 *
 * @param labels Array to store labels corresponding to each trigger result
 * @param readings Array to store trigger results as floating point values
 * @param array_ix Array index to label and readings
 * @param max_size Maximum size of label and reading arrays (number of entries)
 *
 * @return (*array_ix) + number of entries filled
 */

uint8 zip_valve_2(char *labels[], float readings[], uint8 *array_ix, int *valve_2_trigger, uint8 max_size);

/* [] END OF FILE */
