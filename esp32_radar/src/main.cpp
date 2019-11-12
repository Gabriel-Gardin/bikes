3#include <Arduino.h>
#include "arduinoFFT.h"
#include<Wire.h>
 
#define SAMPLES 1024            //Must be a power of 2
#define SAMPLING_FREQUENCY 40000 //Hz, must be less than 10000 due to ADC
 
arduinoFFT FFT = arduinoFFT();
 
unsigned int sampling_period_us; 
unsigned long microseconds;

TaskHandle_t fft;

QueueHandle_t fila1;

//*************function_declaration****************
void fft_func(void * parameter);

double copy_data[SAMPLES];
double vImag[SAMPLES];
double speed;
double vReal[SAMPLES];  //Armazena os dados. Global para os dois processadores.
double *data;

uint32_t debugFlag = 0x001F0000;

void setup() {
    Serial.begin(115200);
    Wire.begin();
   // Wire.setDebugFlags(debugFlag,0);// resetBits=0 says leave all current setBits as is.
    sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));   
    fila1 = xQueueCreate(2, sizeof(struct adc_data*));
    if(fila1 != 0)
    {
        xTaskCreatePinnedToCore(
            fft_func, /* Function to implement the task */
            "fft_fucn", /* Name of the task */
            100000,  /* Stack size in words */
            NULL,  /* Task input parameter */
            0,  /* Priority of the task */
            &fft,  /* Task handle. */
            0); /* Core where the task should run */
    }
}
 
void loop() {
    /* keep the status of sending data */
    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS(1);
    for(;;)
    {
        for(int i=0; i<SAMPLES; i++)
        {
            microseconds = micros();
            vReal[i] = analogRead(34);
            while(micros() < (microseconds + sampling_period_us)){}
        }
        data = vReal;
        xStatus = xQueueSend(fila1,(void *) &data, xTicksToWait);
    }
 //   vTaskDelay(1 / portTICK_PERIOD_MS);
}

void fft_func(void * pvParameters)
{
    double *received_data;
    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS(1);
    Wire.begin();

    for(;;)
    {
        /*FFT*/
        xStatus = xQueueReceive(fila1, &received_data, xTicksToWait);

        if(xStatus == pdPASS){
            for(int i = 0; i < SAMPLES; i++)
            {
                vImag[i] = 0;
            }
            memcpy(copy_data, received_data, sizeof(vReal));
            
            FFT.Windowing(copy_data, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
            FFT.Compute(copy_data, vImag, SAMPLES, FFT_FORWARD);
            FFT.ComplexToMagnitude(copy_data, vImag, SAMPLES);
            double peak = FFT.MajorPeak(copy_data, SAMPLES, SAMPLING_FREQUENCY);
            speed = peak/44;
            //

           if(speed > 15){
                Serial.println(speed);
                Wire.beginTransmission(8);                       // start transmit to slave arduino (8)
                Wire.write((uint8_t) speed);                          // sends one byte converted POT value to slave
                Serial.println(Wire.endTransmission());
        //        delay(1);
            }
        }
    //vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}