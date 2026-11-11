#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* Demonstration LPG alarm profile in estimated ppm; not ADC counts or a
 * conversion of the old ADC thresholds. Validate after sensor calibration.
 * Defaults apply when Flash does not contain valid saved PPM thresholds. */
/* Temporary bench thresholds for the current uncalibrated 0..91 display. */
#define GAS_WARNING_PPM_DEFAULT   50U /* Enter WARNING at or above this. */
#define GAS_DANGER_PPM_DEFAULT    80U /* Enter DANGER at or above this. */
#define GAS_HYSTERESIS_PPM         5U /* Clear only below threshold - 5. */
#define GAS_THRESHOLD_STEP_PPM     5U /* One button adjustment. */
#define GAS_THRESHOLD_MIN_PPM     5U /* Bench setting; not a rated MQ-2 limit. */
#define GAS_MIN_GAP_PPM          10U /* Minimum danger - warning. */
#define GAS_PPM_MAX            10000U /* Upper PPM/threshold limit. */

/* ADC acquisition limit, separate from the PPM alarm settings above. */
#define GAS_ADC_MAX             4095U

/* MQ-2 module powered at 5 V, AO wired directly to PA0: AO/ADC gain = 1.
 * PA0 analog input must stay within the ADC voltage range (0..Vref).
 * A 5 V module can exceed this range; software gain does not limit voltage.
 * RL is the sensor-circuit load (usually fitted on the module).
 * R0 is the sensor resistance measured in clean air, not an added resistor.
 * RL/R0 are required by the PPM model even without an external divider.
 * Both 10 kohm values below are unmeasured placeholders; replace them with
 * the actual load/calibration. See docs/MQ2_PPM.md. */
#define MQ2_ADC_VREF_MV     3300.0f
#define MQ2_CIRCUIT_MV      5000.0f
#define MQ2_AO_TO_ADC_GAIN  1.0f
#define MQ2_RL_KOHM         10.0f
#define MQ2_R0_KOHM         10.0f

/* Approximate LPG/propane curve, using clean-air R0. These are a prototype
 * fit to Winsen v1.6 Fig.3: about (300 ppm, 0.22) and (10000 ppm, 0.018).
 * These are not manufacturer-specified accuracy coefficients. */
#define MQ2_LPG_REFERENCE_PPM   300.0f
#define MQ2_LPG_REFERENCE_RATIO 0.22f
#define MQ2_LPG_EXPONENT        (-1.4f)

/* Shared capacity for the ADC history and its OLED graph. */
#define APP_HISTORY_COUNT     64U

#define SENSOR_PERIOD_MS      100U
#define ALARM_PERIOD_MS        20U
#define OLED_PERIOD_MS        200U
#define BUTTON_DEBOUNCE_MS     30U
#define BUTTON_LONG_MS       2000U
#define SENSOR_TIMEOUT_MS    2000U
#define MUTE_TIME_MS        30000U
#define TEST_TIME_MS         1000U

/* Example SSD1306 module address on the I2C bus. */
#define SSD1306_I2C_ADDR 0x3CU

#endif
