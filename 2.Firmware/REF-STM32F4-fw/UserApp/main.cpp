#include "common_inc.h"

/* User Variables ---------------------------------------------------------*/
BoardConfig_t boardConfig;

// On-board Screen, can choose from hi2c2 or hi2c0(soft i2c)
SSD1306 oled(&hi2c0);

// On-board Sensor, used hi2c1
MPU6050 mpu6050(&hi2c1);

// 5 User-Timers, can choose from htim7/htim10/htim11/htim13/htim14
Timer timerCtrlLoop(&htim7, 50);
Timer timerHeartBeat(&htim10, 1);

// 2x2-channel PWMs, used htim9 & htim12, each has 2-channel outputs
PWM pwm(21000, 21000);

// 4-channel 14bit ADC inputs
Analog analog;

DummyRobot dummy(&hcan1);

/* Thread Definitions -----------------------------------------------------*/
// 200Hz accurate period called by TIM7.
osThreadId_t controlLoopTaskHandle;
void ThreadControlLoop(void* argument)
{
    static float t = 0;
    for (;;)
    {
        // Suspended here until got Notification.
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        mpu6050.Update(true);

        dummy.UpdateJointPos();

//        motorJ1.SetSpeedSetPoint(mpu6050.data.gz * 10000);
    }
}

// Infinity display refresh loop.
osThreadId_t oledTaskHandle;
void ThreadOledUpdate(void* argument)
{
    uint32_t t = micros();

    for (;;)
    {
        oled.clearBuffer();
        oled.setFont(u8g2_font_5x8_tr);
        oled.setCursor(0, 10);
        oled.printf("IMU:%.3f,%.3f", mpu6050.data.az, mpu6050.data.gz);
        oled.setCursor(0, 20);
        oled.printf("ADC:%.2f,%.2f,%.2f,%.2f",
                    analog.GetVoltage(Analog::CH1), analog.GetVoltage(Analog::CH2),
                    analog.GetVoltage(Analog::CH3), analog.GetVoltage(Analog::CH4));
        oled.setCursor(85, 10);
        oled.printf("| FPS:%lu", 1000000 / (micros() - t));
        t = micros();

        oled.sendBuffer();
    }
}


/* Timer Callbacks -------------------------------------------------------*/
void OnTimer7Callback()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Wake & invoke thread IMMEDIATELY.
    vTaskNotifyGiveFromISR(TaskHandle_t(controlLoopTaskHandle), &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void OnTimer10Callback()
{
    if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin))
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

//    printf("[%ld] Hello from STM32F4.\n", micros());
}


/* Default Entry -------------------------------------------------------*/
void Main(void)
{
    // Load all configurations stored in NVM.
    // LoadConfiguration();

    // Init all communication staff, include USB-CDC/VCP/UART/CAN etc.
    InitCommunication();

    // Init IMU.
    do
    {
        mpu6050.Init();
        osDelay(100);
    } while (!mpu6050.testConnection());
    mpu6050.InitFilter(200, 100, 50);

    // Init OLED 128x80.
    oled.Init();

    // Init & Run User Threads.
    const osThreadAttr_t oledTask_attributes = {
        .name = "OledTask",
        .stack_size = 1000 * 4,
        .priority = (osPriority_t) osPriorityNormal,   // should >= Normal
    };
    oledTaskHandle = osThreadNew(ThreadOledUpdate, nullptr, &oledTask_attributes);

    const osThreadAttr_t controlLoopTask_attributes = {
        .name = "ControlLoopTask",
        .stack_size = 1000 * 4,
        .priority = (osPriority_t) osPriorityRealtime, // robot control thread is critical, should be the highest
    };
    controlLoopTaskHandle = osThreadNew(ThreadControlLoop, nullptr, &controlLoopTask_attributes);

    // Start Timer Callbacks.
    timerCtrlLoop.SetCallback(OnTimer7Callback);
    timerCtrlLoop.Start();
    timerHeartBeat.SetCallback(OnTimer10Callback);
    timerHeartBeat.Start();
}
