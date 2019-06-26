/*
 * gimbal_tasks.c
 *
 *  Created on: Jun 24, 2019
 *      Author: ntron
 */
/*====================== INCLUDES ======================== */

/// Project Top-Level Headers ///
#include "main.h"
#include "gimbal.h"

/// FreeRTOS Headers ///
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

///  Module Headers ///
#include "imu.h"
/// others...

/*====================== DEFINES ========================= */
/// Task Priority Levels
/// TODO: these are fairly arbitrary right now, the ordering needs to be given more thought
#define PRIO_IMU					((UBaseType_t)10)
#define PRIO_TARGETSET				((UBaseType_t)9)
#define PRIO_UARTRX					((UBaseType_t)8)

/* ============= GLOBAL RESOURCE VARIABLES =============== */

static volatile IMU_t imu;

/* ============== SYNCHRONIZATION OBJECTS ================ */

QueueHandle_t xTargetQueue;
QueueHandle_t xUartTxQueue;

/* =================== TASK HANDLES ====================== */
/// IMU handler task handle
TaskHandle_t xTaskIMU;
TaskHandle_t xTaskUartRx;
TaskHandle_t xTaskGimbalControl;
TaskHandle_t xTaskTargetSet;

/* ================== INIT FUNCTIONS ===================== */
/// Top-level Init function
void Gimbal_Init(void)
{
	Gimbal_InitSensors();
	Gimbal_InitTasks();

	//TODO: whatever else needs to go in here
}

/// Init the peripheral sensor modules
void Gimbal_InitSensors(void)
{
	IMU_Init(&imu, AXIS_IMU);
	/// others... including any calibration required
}

/// Init the tasks
void Gimbal_InitTasks(void)
{
	xTaskCreate(vImuIRQHandler,"IMUHandler",configMINIMAL_STACK_SIZE, NULL, PRIO_IMU, &xTaskIMU);
	/// others...
}

/* ================== TASK FUNCTIONS ===================== */
/// IMU interrupt handler
void vImuIRQHandler(void* pvParameters)
{
	// initialize the IMU, this needs to go here to prevent the fifo from starting interrupts
	IMU_Start(&imu);

	while(true)
	{
		// wait for IMU interrupt
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		IMU_GetQuaternion(&imu);
		IMU_CalcEulerAngles(&imu);
		printf("%d, %d, %d\n", (int)(imu.pos.pitch), (int)(imu.pos.yaw), (int)(imu.pos.roll));
	}
}

void vUartRxIRQHandler(void* pvParameters)
{
	while(true)
	{
		vTaskDelay(1000);
	}
}

void vUartTxTask(void* pvParameters)
{
	while(true)
	{
		vTaskDelay(1000);
	}
}

void vGimbalControlLoopTask(void* pvParameters)
{
	while(true)
	{
		vTaskDelay(1000);
	}
}


void vTargetSettingTask(void* pvParameters)
{
	while(true)
	{
		vTaskDelay(1000);
	}
}



/* ============= HAL IRQ HANDLER CALLBACKS =============== */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	// Handoff to IMU handler task
	if (GPIO_Pin == AXIS_IMU_INT_Pin)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); /// TODO: temporary, remove this
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		vTaskNotifyGiveFromISR( xTaskIMU, &xHigherPriorityTaskWoken );
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}
