#include <Arduino.h>
#include "arduinoFFT.h"
#include "Wire.h"
 
#define SAMPLES 1024            //Deve ser um número na base 2
#define SAMPLING_FREQUENCY 40000 //Frequencia da amostragem do radar em Hz
 
arduinoFFT FFT = arduinoFFT();

QueueHandle_t fila1;

//*************function_declaration****************
void fft_func(void * parameter);

double vReal[SAMPLES];  //Buffer de dados amostragem do analogRead. Deve ser global para não overflow o a task do loop....

void setup() {
    Serial.begin(115200);
    Wire.begin();
    fila1 = xQueueCreate(2, sizeof(&vReal)); //Cria a queue com o tamanho de dois ponteiros do array vReal....
    if(fila1 != 0)
    {
        xTaskCreatePinnedToCore(
            fft_func, /* Function to implement the task */
            "fft_fucn", /* Name of the task */
            20480,  /* Stack size in BYTES!!! (20 bytes)*/
            NULL,  /* Task input parameter */
            0,  /* Priority of the task */
            NULL,  /* Task handle. */
            0); /* Core where the task should run */
    }
}
 
void loop() {
    /* keep the status of sending data */
    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS(1);
    double *data;
    unsigned int sampling_period_us; 
    unsigned long microseconds;
    sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));   
    for(;;)
    {
        for(int i=0; i<SAMPLES; i++)
        {
            vReal[i] = analogRead(34);
            //while(micros() < (microseconds + sampling_period_us)){}
            delayMicroseconds(sampling_period_us);
        }
        data = &vReal[0]; //Aponta para o endereço do primeiro ítem do array. Equivalente a data = vReal
        //Por mais que as queues do freertos passem os dados por cópia e não pro referencia, isto não é viável em grandes buffers, como é o caso
        xStatus = xQueueSend(fila1, (void *)&data, xTicksToWait);
    }
 //   vTaskDelay(1 / portTICK_0PERIOD_MS);
}

void fft_func(void * pvParameters)
{
    double *received_data;
    double copy_data[SAMPLES];
    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS(1);
    Wire.begin();

    for(;;)
    {
        /*FFT*/
        double speed;
        xStatus = xQueueReceive(fila1, &received_data, xTicksToWait);

        if(xStatus == pdPASS){
            double vImag[SAMPLES] = {0};
            memcpy(copy_data, received_data, sizeof(vReal)); //Copio a data recebida pela queue, afim de liberar o uso da variável para task do loop..
            //double *data1 = received_data;
            
            FFT.Windowing(copy_data, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
            FFT.Compute(copy_data, vImag, SAMPLES, FFT_FORWARD);
            FFT.ComplexToMagnitude(copy_data, vImag, SAMPLES);
            double peak = FFT.MajorPeak(copy_data, SAMPLES, SAMPLING_FREQUENCY);
            speed = peak/44;
            
           // Serial.println(speed);
            if(speed > 15){
            //    Serial.println(speed);
                Wire.beginTransmission(8);                       // start transmit to slave arduino (8)
                Wire.write((uint8_t) speed);                          // sends one byte converted POT value to slave
                Wire.endTransmission();
            }
        }
    }
}
