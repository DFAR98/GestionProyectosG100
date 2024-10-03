#include "pico/stdlib.h"
#include <stdio.h>

#define SIGNAL_A_PIN 2  // Pin de la señal A
#define SIGNAL_B_PIN 3  // Pin de la señal B

// Variables para almacenar el estado actual y anterior de las señales
volatile uint8_t signal_a_last_state = 0;
volatile uint8_t signal_b_last_state = 0;
volatile int32_t position = 0;  // Variable para almacenar la posición

// Variables para medir velocidad
volatile absolute_time_t last_time;
volatile float speed = 0.0;  // Velocidad en pulsos por segundo (PPS)

// Interrupción para manejar los cambios en las señales
void gpio_callback(uint gpio, uint32_t events) {
    uint8_t signal_a_state = gpio_get(SIGNAL_A_PIN);
    uint8_t signal_b_state = gpio_get(SIGNAL_B_PIN);
    
    // Calcular el tiempo actual y el tiempo desde el último cambio
    absolute_time_t current_time = get_absolute_time();
    int64_t time_diff_us = absolute_time_diff_us(last_time, current_time);  // Diferencia en microsegundos
    
    // Calcular velocidad (pulsos por segundo)
    if (time_diff_us > 0) {
        speed = 1000000.0f / time_diff_us;  // Velocidad en PPS (pulsos por segundo)
    }
    
    // Actualizar el último tiempo
    last_time = current_time;

    // Si las señales cambian, determinar la dirección del giro
    if (signal_a_state != signal_a_last_state || signal_b_state != signal_b_last_state) {
        if (signal_a_state == signal_b_state) {
            position++;  // Gira en una dirección
        } else {
            position--;  // Gira en la otra dirección
        }

        // Actualizar el estado anterior
        signal_a_last_state = signal_a_state;
        signal_b_last_state = signal_b_state;
    }
}

int main() {
    // Inicialización de los pines
    stdio_init_all();
    gpio_init(SIGNAL_A_PIN);
    gpio_init(SIGNAL_B_PIN);

    // Configurar los pines como entrada
    gpio_set_dir(SIGNAL_A_PIN, GPIO_IN);
    gpio_set_dir(SIGNAL_B_PIN, GPIO_IN);

    // Habilitar interrupciones en ambos pines para detectar cambios de señal
    gpio_set_irq_enabled_with_callback(SIGNAL_A_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(SIGNAL_B_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // Inicializar el tiempo
    last_time = get_absolute_time();

    // Bucle principal
    while (true) {
        // Determinar la dirección basándose en la posición
        const char* direction = (position >= 0) ? "Forward" : "Reverse";

        // Imprimir la dirección y la velocidad
        printf("Dirección: %s, Velocidad: %.2f PPS, Posición: %d\n", direction, speed, position);
        
        sleep_ms(500);  // Pausa para actualizar
    }

    return 0;
}
