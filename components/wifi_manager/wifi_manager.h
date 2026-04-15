#ifndef _WIFI_MANAGER_H_
#define _WIFI_MANAGER_H_

#include <stdbool.h>

// Pin number for the physical reset button
#define WIFI_RESET_KEY 12

//PIN code for the smartphone app to connect and configure Wi-Fi
#define WIFI_CONNECTION_PIN "1234567"

//Name of the network created by ESP32 when there is no saved password
#define WIFI_CONNECTION_SSID "krk-departure-board"

/**
 * @brief Setups the GPIO pin for the Wi-Fi reset button.
 */
void wifi_reset_key_config(void);

/**
 * @brief Checks if user is pressing the reset button.
 * If the button is pressed it clears saved Wi-Fi data from memory.
 * @return true if button is pressed and memory is cleared.
 * @return false if button is not pressed.
 */
bool wifi_manager_check_reset(void);

/**
 * @brief Starts the Wi-Fi and connects to the network.
 * If there is no saved password it opens its own Wi-Fi network 
 * (Access Point) so the user can configure it via smartphone app.
 */
void wifi_manager_init_sta(void);

/**
 * @brief Checks if device is currently connected to Wi-Fi.
 * @return true if connected to the router.
 * @return false if there is no connection.
 */
bool wifi_manager_is_connected(void);

/**
 * @brief Tries to connect to Wi-Fi again after an error.
 * It disconnects first to be safe, and then connects again. 
 * It blocks the program and waits until the connection is ready.
 */
void wifi_manager_reconnect(void);

#endif