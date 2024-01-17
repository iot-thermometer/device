#define DEFAULT_VREF 1100
#define ADC_CHANNEL ADC_CHANNEL_0

#define ADC_MIN 62000
#define ADC_MAX 262080

void initialize_adc() {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11);
}

float read_soil_moisture_sensor() {
    uint32_t reading = 0;
    for (int i = 0; i < 64; ++i) {
        reading += adc1_get_raw(ADC_CHANNEL);
    }
    float percentage = (ADC_MAX - reading) * 100 / (ADC_MAX - ADC_MIN);
    return percentage;
}