#ifndef __DEBUG_H_
#define __DEBUG_H_

#include "esp_log.h"

/** * @brief Main switch for debug messages. 
 * Change this to 1 to see debug logs, or 0 to turn them off completely.
 */
#define DEBUG 1

#if DEBUG
	#define Debug(__info,...) ESP_LOGW(__func__, __info, ##__VA_ARGS__)
#else
	#define Debug(__info,...)  
#endif

#endif