#define S826_CM_XS_CH0     2
#define printout 0
#define TSETTLE 5;

#include "826api.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h> 

// TODO: convert individual configurations into a struct


/*
 * Function:  AdcHandler 
 * --------------------
 *  Runs the S826_AdcRead!
 */
void AdcHandler(void)
{
    int errcode;
    int slotval[16]; // buffer must be sized for 16 slots

    uint slotlist = 0x0007; //0007;//
    errcode = S826_AdcRead(0, slotval, NULL, &slotlist, S826_WAIT_INFINITE); 
    if (errcode != S826_ERR_OK)
        printf("ADC Read Failed. %d", errcode);

    //uint read_status;
    //S826_AdcEnableRead(0, &read_status);
    //fprintf(stdout, "Check that read status is on: %d; \n", read_status);

    //uint conversion_status;
    //S826_AdcStatusRead(0, &conversion_status);
    //fprintf(stdout, "What is ready to be converted: 0x%04x; \n", conversion_status);

// add logging - level, sender, message  

    int slot;
    for (slot = 0; slot < 3; slot++)    // Display all samples.
    {
        int adcdata;
        unsigned int burstnum;
    float voltage;
        
        //fprintf(stdout, "Raw Channel %d Data = %d; \t", slot, slotval[slot]);
        
        // extract adcdata, burstnum, and bufoverflowflag from buf
        adcdata = (int)((slotval[slot] & 0xFFFF));
        burstnum = ((unsigned int)slotval[slot] >> 24);
    voltage = (float)((float)adcdata / (float)(0x7FFF)) * 10;
        printf("Slot: %d \t Voltage: %f \t ADCData: %d \t BurstNum %d \n", slot, voltage, adcdata, burstnum);   
    };
    printf("\n");   
//    S826_AdcStatusRead(0, &conversion_status);
//    fprintf(stdout, "\nADC conversion status = 0x%04x; \n\n", conversion_status);

}


char t[200];
int main(int argc, char **argv){

    if( argc < 2 ) {
        printf("Use: ./housekeeping duration\n");
        printf("     duration = how long experiment is (seconds)\n");
        exit(0);
    }
    printf("%d %d %d \n", (short)0x7FFF - (short)0x8000, (short)0x7FFF, (short)0x8000);

    //// Preparation
    // Set timer counter interval: 
    // Determines how often count data is stored.
    int duration = atoi(argv[1]); // seconds

    int board = 0;   // only one board
    int runmode = 0; // use runmode

    // ------ Begin Configurations for Housekeeping Sensors ------
    /* 
     * Power Output for Sensors
     *  Function: Analog output channel 1
     *    - Pin = 44 
     *    - Name: AOUT1
     *  Range: 0 to +5 V
     */
    int pwr_output_channel = 1; 
    int pwr_output_range = 0;
    int pwr_output_setpoint = 0xFFFF;

    /*
    S826_ADC_GAIN_1         0           // -10V to +10V
    S826_ADC_GAIN_2         1           // -5V to +5V
    S826_ADC_GAIN_5         2           // -2V to +2V
    S826_ADC_GAIN_10    3       // -1V to +1V
     */
    /* 
     * Temperature Sensor 1 Input
     *  Function: Analog input (+) channel 0
     *    - Pin = 4
     *    - Name: +AIN0
     *  Function: Analog input (-) channel 0
     *    - Pin = 3 
     *    - Name: -AIN0
     */
    int temp_1_channel = 0;
    int temp_1_settling = TSETTLE;
    int temp_1_range = S826_ADC_GAIN_1;
    int temp_1_timeslot = 0;

    /* 
     * Temperature Sensor 2 Input
     *  Function: Analog input (+) channel 1
     *    - Pin = 6
     *    - Name: +AIN1
     *  Function: Analog input (-) channel 1
     *    - Pin = 5
     *    - Name: -AIN1
     */
    int temp_2_channel = 1;
    int temp_2_settling = TSETTLE;
    int temp_2_range = S826_ADC_GAIN_1;
    int temp_2_timeslot = 1;

    /* 
     * Humidity Sensor Input 1
     *  Function: Analog input (+) channel 2
     *    - Pin = 8
     *    - Name: +AIN2
     *  Function: Analog input (-) channel 2
     *    - Pin = 7
     *    - Name: -AIN2
     */
    int hum_1_channel = 2;
    int hum_1_settling = TSETTLE;
    int hum_1_range = S826_ADC_GAIN_1;//0;
    int hum_1_timeslot = 2;

    // ------ End Configurations for Housekeeping Sensors

    // ------ Begin S826 API
    int flags = S826_SystemOpen();
    if (flags < 0)
    printf("S826_SystemOpen returned error code %d", flags);

    // configure power output for sensors
    int pwr = S826_DacRangeWrite(board, pwr_output_channel, pwr_output_range, runmode);
    if (pwr < 0)
        printf("Configure power error code %d", pwr);

    int pwr_set = S826_DacDataWrite(board, pwr_output_channel, pwr_output_setpoint, runmode);
    if (pwr_set < 0)
        printf("Configure power data error code %d", pwr);


    // configure power input for temperature 1 sensor
    int temp_1 = S826_AdcSlotConfigWrite(board, 
        temp_1_timeslot, 
        temp_1_channel, 
        temp_1_settling,
        temp_1_range
    );
    if (temp_1 < 0)
        printf("Configure error %d", temp_1);

    // configure power input for temperature 2 sensor
    int temp_2 = S826_AdcSlotConfigWrite(board, 
        temp_2_timeslot, 
        temp_2_channel, 
        temp_2_settling,
        temp_2_range
    );

    // configure power input for humidity sensor 1
    int hum_1 = S826_AdcSlotConfigWrite(board, 
        hum_1_timeslot, 
        hum_1_channel, 
        hum_1_settling,
        hum_1_range
    );

    // enable the slots in the slot list
    // 0xE000 = is the first three 
    // trigger mode = continuous
    // enable the board 

    int err_AdcSlotlistWrite = S826_AdcSlotlistWrite(board, 0x0007, S826_BITWRITE);
    if (err_AdcSlotlistWrite < 0)
        printf("S826_AdcSlotlistWrite error code %d", err_AdcSlotlistWrite);

    int err_AdcTrigModeWrite = S826_AdcTrigModeWrite(board, 0); 
    if (err_AdcTrigModeWrite < 0)
        printf("S826_AdcTrigModeWrite error code %d", err_AdcTrigModeWrite);

    int err_AdcEnableWrite = S826_AdcEnableWrite(board, 1);  
    if (err_AdcEnableWrite < 0)
        printf("S826_AdcEnableWrite error code %d", err_AdcEnableWrite);    

    nanosleep((const struct timespec[]){{0, 10000000000L}}, NULL);

    // time keeping
    time_t rawtime, startime;
    time(&rawtime);
    startime = rawtime;
    
    while (rawtime - startime < duration){
    
        AdcHandler();

        // update time and sleep time
        time(&rawtime);
        nanosleep((const struct timespec[]){{0, 500000000L}}, NULL);
    }

    // explicitly set power to zero
    int pwr_set_off = S826_DacDataWrite(board, pwr_output_channel, 0x0000, runmode);
    if (pwr_set_off < 0)
        printf("Configure power data off code %d", pwr);

    // ------ Close S826 API ------
    S826_SystemClose();

};