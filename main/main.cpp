#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

// Очередь gpio прерываний
static QueueHandle_t gpio_btn_queue = NULL;

void gpio_pin_init() {
    // Пустая конфигурация GPIO
    gpio_config_t io_conf = {};
    // Разрешаем прерывания по спаду.
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    // Битовая маска пина к которому применяются настройки
    // 1ULL<<GPIO_NUM_14 == 0000000000000000000000000100000000000000
    io_conf.pin_bit_mask = (1ULL<<GPIO_NUM_14);
    // В какую сторону будетработать пин (вход или выход)
    // Кнопка работает на вход
    io_conf.mode = GPIO_MODE_INPUT;
    // Подтяжка к верху
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    // Устанавливаем конфигурацию
    ESP_ERROR_CHECK(gpio_config(&io_conf));

}

static void IRAM_ATTR gpio_btn_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_btn_queue, &gpio_num, NULL);
}

void gpio_task_btn(void* arg) {
    gpio_num_t io_num;
    for(;;) {
        if (xQueueReceive(gpio_btn_queue, &io_num, portMAX_DELAY)) {
            ESP_LOGI("isr_task", "GPIO[%d] intr, val: %d", io_num, gpio_get_level(io_num));
        }
    }
}

extern "C" void app_main(void) {
    // Инициализация GPIO пинов
    gpio_pin_init();
    // Регистрируем службу очередей для прерывания по кнопке
    gpio_btn_queue = xQueueCreate(5, sizeof(uint32_t));
    // Задача обрабатывает очередь нажатия на кнопки
    xTaskCreate(gpio_task_btn, "gpio_task_btn", 2048, NULL, 10, NULL);
    // Запускаем сервис обработки прерываний
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_14, gpio_btn_isr_handler, (void*) GPIO_NUM_14);
}
