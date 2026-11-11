#include "task_sensor.h"
#include "app_ipc.h"
#include "bsp_board.h"
#include "app_config.h"
#include <math.h>

#define MQ2_PPM_INVALID 0xFFFFU

/* Estimated LPG/propane ppm, limited to GAS_PPM_MAX. Full-scale ADC is
 * converted numerically (about 91 with current settings), even if clipped.
 * This display convention cannot measure beyond the ADC range. */
static uint16_t MQ2_ConvertToPPM(uint16_t adc)
{
    float outputMv;
    float resistanceRatio;
    float ppm;

    if (adc == 0U || adc > GAS_ADC_MAX ||
        !(MQ2_ADC_VREF_MV > 0.0f) || !(MQ2_CIRCUIT_MV > 0.0f) ||
        !(MQ2_AO_TO_ADC_GAIN > 0.0f) || !(MQ2_RL_KOHM > 0.0f) ||
        !(MQ2_R0_KOHM > 0.0f) || !(MQ2_LPG_REFERENCE_PPM > 0.0f) ||
        !(MQ2_LPG_REFERENCE_RATIO > 0.0f) || !(MQ2_LPG_EXPONENT < 0.0f))
    {
        return MQ2_PPM_INVALID;
    }

    outputMv = ((float)adc * MQ2_ADC_VREF_MV / (float)GAS_ADC_MAX) *
               MQ2_AO_TO_ADC_GAIN;
    if (outputMv >= MQ2_CIRCUIT_MV)
    {
        return SENSOR_ADC_OVERRANGE;
    }
    if (!(outputMv > 0.0f))
    {
        return MQ2_PPM_INVALID;
    }

    /* Rs = RL * (Vc / Vout - 1); R0 is measured in clean air.
     * See docs/MQ2_PPM.md for the curve reference and calibration. */
    resistanceRatio = (MQ2_RL_KOHM / MQ2_R0_KOHM) *
                      (MQ2_CIRCUIT_MV / outputMv - 1.0f);
    ppm = MQ2_LPG_REFERENCE_PPM * powf(
        resistanceRatio / MQ2_LPG_REFERENCE_RATIO, MQ2_LPG_EXPONENT);

    if (!(ppm >= 0.0f))
    {
        return MQ2_PPM_INVALID;
    }
    if (ppm >= (float)GAS_PPM_MAX)
    {
        return GAS_PPM_MAX;
    }

    return (uint16_t)(ppm + 0.5f);
}

static void prvSensorTask(void *unused)
{
    TickType_t last = xTaskGetTickCount();
    uint16_t raw;
    uint16_t adc = 0U;
    uint16_t ppm;
    uint32_t sum;
    SensorSample_t sample;
    uint8_t index;
    uint8_t valid;
    uint8_t clipped;
    uint8_t hasSample = 0U;

    (void)unused;

    for (;;)
    {
        sum = 0UL;
        valid = 1U;
        clipped = 0U;

        for (index = 0U; index < 8U; ++index)
        {
            raw = bsp_adc_read();
            if (raw == 0U || raw > GAS_ADC_MAX)
            {
                valid = 0U;
                break;
            }
            if (raw == GAS_ADC_MAX)
            {
                clipped = 1U;
            }
            sum += raw;
        }

        if (valid != 0U)
        {
            if (clipped != 0U)
            {
                /* Show the full-scale estimate immediately, without IIR lag. */
                ppm = MQ2_ConvertToPPM(GAS_ADC_MAX);
            }
            else
            {
                raw = (uint16_t)(sum / 8UL);
                if (hasSample == 0U)
                {
                    adc = raw;
                    hasSample = 1U;
                }
                else
                {
                    adc = (uint16_t)((3UL * adc + raw + 2UL) / 4UL);
                }

                ppm = MQ2_ConvertToPPM(adc);
            }

            if (ppm != MQ2_PPM_INVALID)
            {
                /* Resume from a fresh sample after a full-scale batch. */
                if (clipped != 0U || ppm == SENSOR_ADC_OVERRANGE)
                {
                    hasSample = 0U;
                }
                sample.ppm = ppm;
                sample.adc = clipped != 0U ? GAS_ADC_MAX : adc;
                xQueueOverwrite(xSensorQueue, &sample);
                if (ppm != SENSOR_ADC_OVERRANGE)
                {
                    app_history_push(ppm);
                }
            }
        }

        vTaskDelayUntil(&last, pdMS_TO_TICKS(SENSOR_PERIOD_MS));
    }
}

BaseType_t task_sensor_create(void)
{
    return xTaskCreate(
        prvSensorTask,
        "SENSOR",
        192U,
        NULL,
        tskIDLE_PRIORITY + 3U,
        NULL);
}
