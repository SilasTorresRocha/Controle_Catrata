#pragma once
#include "esp_err.h"
#include <stdbool.h>
#include <time.h>

/**
 * @brief Inicializa a interface Ethernet (W5500 via SPI).
 */
void network_manager_init(void);

/**
 * @brief Retorna true se a rede estiver conectada.
 */
bool is_network_connected(void);

/**
 * @brief Obtém o horário atual já sincronizado via SNTP.
 *
 * @param timeinfo Estrutura que receberá o horário.
 * @return esp_err_t ESP_OK em caso de sucesso.
 */
esp_err_t get_current_time(struct tm *timeinfo);
