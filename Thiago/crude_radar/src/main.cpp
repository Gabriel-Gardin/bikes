#include <Arduino.h>
#include "arduinoFFT.h"
 
#define SAMPLES 512            //Must be a power of 2
#define SAMPLING_FREQUENCY 40000 //Hz, must be less than 10000 due to ADC
 
arduinoFFT FFT = arduinoFFT();
 
unsigned int sampling_period_us; 
unsigned long microseconds;

TaskHandle_t fft;

QueueHandle_t fila1;

//*************function_declaration****************
void fft_func(void * parameter);

double speed;
double vReal[SAMPLES];  //Armazena os dados. Global para os dois processadores.
double *data;
//double vImag[SAMPLES] = {0};

unsigned long t0;

void setup() {
    Serial.begin(115200);
    sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));   
    fila1 = xQueueCreate(2, sizeof(*vReal));
    if(fila1 != 0)
    {
        xTaskCreatePinnedToCore(
            fft_func, /* Function to implement the task */
            "fft_fucn", /* Name of the task */
            20480,  /* Stack size in BYYTES! */
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
        t0 = micros();
        for(int i=0; i<SAMPLES; i++)
        {
            vReal[i] = analogRead(34);
            //while(micros() < (microseconds + sampling_period_us)){}
            //Serial.println(vReal[i]);
            delayMicroseconds(sampling_period_us);
        }
        data = &vReal[0]; //Aponta para o endereço do primeiro ítem do array. Equivalente a data = vReal

        xStatus = xQueueSend(fila1, (void *)&data, xTicksToWait);
       // Serial.println(micros() - t0);
    }
 //   vTaskDelay(1 / portTICK_0PERIOD_MS);
}

void fft_func(void * pvParameters)
{
    double *received_data;
    double copy_data[SAMPLES];
    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS(1);

    for(;;)
    {
        /*FFT*/
        xStatus = xQueueReceive(fila1, &received_data, xTicksToWait);
        if(xStatus == pdPASS){
            double vImag[SAMPLES] = {0};
            memcpy(copy_data, received_data, sizeof(vReal)); //Copio a data recebida pela queue, afim de liberar o uso da variável para task do loop...
            //double *data1 = received_data;
            
            FFT.Windowing(copy_data, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
            FFT.Compute(copy_data, vImag, SAMPLES, FFT_FORWARD);
            FFT.ComplexToMagnitude(copy_data, vImag, SAMPLES);
            double peak = FFT.MajorPeak(copy_data, SAMPLES, SAMPLING_FREQUENCY);
            speed = peak/44;
            Serial.println(speed);
        }
    }
}