#include "bsp_board.h"
#include "bsp_pins.h"
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "app_config.h"
#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_rcc.h"

#define CONFIG_FLASH_ADDR 0x0800FC00UL
#define CONFIG_MAGIC      0x4E13U
/* Versions 1..3 belong to previous ADC/PPM profiles.
 * Version 4 starts with temporary 50/80 display thresholds after upgrading.
 * Settings saved by this version are loaded normally on later boots. */
#define CONFIG_VERSION    4U

uint32_t SystemCoreClock = 8000000UL;
static uint8_t adcReady = 0U;

void SystemInit(void)
{
    RCC_DeInit();
    RCC_HSICmd(ENABLE);

    while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET)
    {
    }

    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    RCC_ADCCLKConfig(RCC_PCLK2_Div2);
    FLASH_SetLatency(FLASH_Latency_0);

    SystemCoreClock = 8000000UL;
}

void SystemCoreClockUpdate(void)
{
    SystemCoreClock = 8000000UL;
}

static uint8_t adc_init(void)
{
    ADC_InitTypeDef adc;
    volatile uint32_t delay;
    uint32_t timeout;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div2);

    adc.ADC_Mode = ADC_Mode_Independent;
    adc.ADC_ScanConvMode = DISABLE;
    adc.ADC_ContinuousConvMode = DISABLE;
    adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc.ADC_DataAlign = ADC_DataAlign_Right;
    adc.ADC_NbrOfChannel = 1U;
    ADC_Init(ADC1, &adc);

    ADC_RegularChannelConfig(
        ADC1,
        BSP_SENSOR_ADC_CHANNEL,
        1U,
        ADC_SampleTime_239Cycles5);
    ADC_Cmd(ADC1, ENABLE);

    for (delay = 0U; delay < 10000UL; ++delay)
    {
    }

    ADC_ResetCalibration(ADC1);
    timeout = 100000UL;
    while (ADC_GetResetCalibrationStatus(ADC1) != RESET && --timeout > 0U)
    {
    }
    if (timeout == 0U)
    {
        return 0U;
    }

    ADC_StartCalibration(ADC1);
    timeout = 100000UL;
    while (ADC_GetCalibrationStatus(ADC1) != RESET && --timeout > 0U)
    {
    }

    return (timeout > 0U) ? 1U : 0U;
}

uint16_t bsp_adc_read(void)
{
    uint32_t timeout = 100000UL;

    if (adcReady == 0U)
    {
        return BSP_ADC_INVALID;
    }

    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET && --timeout > 0U)
    {
    }
    if (timeout == 0U)
    {
        return BSP_ADC_INVALID;
    }

    return ADC_GetConversionValue(ADC1);
}

void bsp_board_init(void)
{
    HAL_GPIO_InitBoard();
    adcReady = adc_init();
    HAL_I2C_Init();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
}

void bsp_buttons_irq_enable(void)
{
    HAL_GPIO_ButtonsIRQEnable();
}

uint16_t bsp_buttons_pressed_mask(void)
{
    return HAL_GPIO_ButtonsPressedMask();
}

void bsp_buzzer_write(uint8_t on)
{
    HAL_GPIO_BuzzerWrite(on);
}

uint8_t bsp_i2c1_write(
    uint8_t address7,
    uint8_t control,
    const uint8_t *bytes,
    uint16_t length)
{
    return HAL_I2C_Write(address7, control, bytes, length);
}

uint8_t bsp_flash_load_thresholds(uint16_t *warning, uint16_t *danger)
{
    volatile const uint16_t *stored =
        (volatile const uint16_t *)CONFIG_FLASH_ADDR;
    uint16_t stored_warning;
    uint16_t stored_danger;
    uint16_t checksum;

    if (stored[0] != CONFIG_MAGIC || stored[1] != CONFIG_VERSION)
    {
        return 0U;
    }

    stored_warning = stored[2];
    stored_danger = stored[3];
    checksum = (uint16_t)(
        CONFIG_MAGIC ^ CONFIG_VERSION ^ stored_warning ^ stored_danger ^ 0xA55AU);

    if (stored[4] != checksum ||
        stored_warning < GAS_THRESHOLD_MIN_PPM ||
        stored_danger > GAS_PPM_MAX ||
        (uint32_t)stored_danger < (uint32_t)stored_warning + GAS_MIN_GAP_PPM)
    {
        return 0U;
    }

    *warning = stored_warning;
    *danger = stored_danger;
    return 1U;
}

static uint8_t flash_wait_idle(void)
{
    uint32_t timeout = 4000000UL;

    while (FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET && --timeout > 0U)
    {
    }

    /* Previous operation errors are cleared after Flash becomes idle. */
    return (timeout > 0U) ? 1U : 0U;
}

uint8_t bsp_flash_save_thresholds(uint16_t warning, uint16_t danger)
{
    uint16_t values[5];
    uint32_t index;

    if (warning < GAS_THRESHOLD_MIN_PPM ||
        danger > GAS_PPM_MAX ||
        (uint32_t)danger < (uint32_t)warning + GAS_MIN_GAP_PPM)
    {
        return 0U;
    }

    values[0] = CONFIG_MAGIC;
    values[1] = CONFIG_VERSION;
    values[2] = warning;
    values[3] = danger;
    values[4] = (uint16_t)(
        CONFIG_MAGIC ^ CONFIG_VERSION ^ warning ^ danger ^ 0xA55AU);

    if (flash_wait_idle() == 0U)
    {
        return 0U;
    }

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    if (FLASH_ErasePage(CONFIG_FLASH_ADDR) != FLASH_COMPLETE)
    {
        goto error;
    }

    for (index = 0U; index < 5U; ++index)
    {
        if (FLASH_ProgramHalfWord(CONFIG_FLASH_ADDR + 2U * index, values[index]) !=
            FLASH_COMPLETE)
        {
            goto error;
        }
    }

    FLASH_Lock();
    return 1U;

error:
    FLASH_Lock();
    return 0U;
}
