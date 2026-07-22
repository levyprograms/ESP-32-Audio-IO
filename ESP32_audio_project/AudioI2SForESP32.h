#ifndef AUDIOI2SFORESP32_H
#define AUDIOI2SFORESP32_H

#include <driver/i2s.h>
#include <soc/io_mux_reg.h> // <-- ADICIONADO: Necessário para rotear o MCLK

#define VERSAO_LIB 1.0

// ============================================================
// FUNÇÕES DE INICIALIZAÇÃO DE ÁUDIO I2S
// ============================================================

void initI2S_TX() {
  i2s_config_t cfg = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate          = SAMPLE_RATE,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 8,
    .dma_buf_len          = 64,
    .use_apll             = true,
    .tx_desc_auto_clear   = true,
    .fixed_mclk           = 0
  };
  i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL);

  i2s_pin_config_t pins = {
    .bck_io_num   = I2S_TX_BCK,
    .ws_io_num    = I2S_TX_WS,
    .data_out_num = I2S_TX_DOUT,
    .data_in_num  = I2S_PIN_NO_CHANGE
  };
  i2s_set_pin(I2S_NUM_0, &pins);
  i2s_zero_dma_buffer(I2S_NUM_0);
}

void initI2S_RX() {
  i2s_config_t cfg = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate          = SAMPLE_RATE,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 8,
    .dma_buf_len          = 64,
    .use_apll             = true,
    .tx_desc_auto_clear   = false,
    .fixed_mclk           = 0
  };
  i2s_driver_install(I2S_NUM_1, &cfg, 0, NULL);

  i2s_pin_config_t pins = {
    .bck_io_num   = I2S_RX_BCK,
    .ws_io_num    = I2S_RX_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num  = I2S_RX_DIN
  };
  i2s_set_pin(I2S_NUM_1, &pins);

  // ==============================================================
  // HACK HARDWARE: Roteia o Master Clock (MCLK) do I2S1 para o pino RX0 (GPIO 3)
  // Obrigatório para o PCM1808 gerar amostras de áudio.
  // ==============================================================
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD_CLK_OUT2);
  REG_WRITE(PIN_CTRL, (REG_READ(PIN_CTRL) & 0xFFFFFF0F) | (0x10)); 
  // ==============================================================
}

#endif // AUDIOI2SFORESP32_H