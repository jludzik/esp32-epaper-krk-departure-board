#ifndef __DEBUG_H_
#define __DEBUG_H_

#include "esp_log.h"

#define DEBUG 1

#if DEBUG
	#define Debug(__info,...) ESP_LOGW(__func__, __info, ##__VA_ARGS__)
#else
	#define Debug(__info,...)  
#endif

#endif