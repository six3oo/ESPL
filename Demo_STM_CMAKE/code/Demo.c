/**
 * This is the main file of the ESPLaboratory Demo project.
 * It implements simple sample functions for the usage of UART,
 * writing to the display and processing user inputs.
 *
 * @author: Jonathan Müller-Boruttau,
 * 			Tobias Fuchs tobias.fuchs@tum.de
 * 			Nadja Peters nadja.peters@tum.de (RCS, TUM)
 *
 */
#include "includes.h"

// start and stop bytes for the UART protocol
static const uint8_t startByte = 0xAA,
					 stopByte  = 0x55;

static const uint16_t displaySizeX = 320,
					  displaySizeY = 240;

QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
SemaphoreHandle_t ESPL_DisplayReady;

// Stores lines to be drawn
QueueHandle_t JoystickQueue;

// Persistent buffers for static circle task
static StackType_t xCircleTaskStack[1000];
static StaticTask_t xCircleTaskTCB;
void vApplicationGetCircleTaskMemory( StaticTask_t **ppxCircleTaskTCBBuffer,
                                     StackType_t **ppxCircleTaskStackBuffer,
                                     uint32_t *pulCircleTaskStackSize ) {
*ppxCircleTaskTCBBuffer = &xCircleTaskTCB;
*ppxCircleTaskStackBuffer = xCircleTaskStack;
*pulCircleTaskStackSize = 1000;
}


int main() {

	// Initialize Board functions and graphics
	ESPL_SystemInit();

	// Initializes Draw Queue with 100 lines buffer
	JoystickQueue = xQueueCreate(100, 2 * sizeof(char));

	// Initializes Tasks with their respective priority
	xTaskCreate(circleBlink1, "circleBlink1", 300, NULL, 1, NULL);
	xTaskCreateStatic(circleBlink2, "circleBlink2", 300, NULL, 2, xCircleTaskStack,
			&xCircleTaskTCB);
	xTaskCreate(checkJoystick, "checkJoystick", 1000, NULL, 3, NULL);
	xTaskCreate(drawTask, "drawTask", 1000, NULL, 4, NULL);

	// Start FreeRTOS Scheduler
	vTaskStartScheduler();

}

/**
 * Exercise 2
 */
void drawTask() {
	char str[100]; // buffer for messages to draw to display
	struct coord joystickPosition; // joystick queue input buffer

	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");

	// Moving string position
	uint16_t stringPositionX = 0;
	uint16_t stringPositionY = 30;

	// Triangle position
	uint16_t trianglePositionX1 = displaySizeX/2;
	uint16_t trianglePositionY1 = displaySizeY/2-20;
	uint16_t trianglePositionX2 = displaySizeX/2+20;
	uint16_t trianglePositionY2 = displaySizeY/2+20;
	uint16_t trianglePositionX3 = displaySizeX/2-20;
	uint16_t trianglePositionY3 = displaySizeY/2+20;

	// Square position
	uint16_t squareX = displaySizeX/2+30;
	uint16_t squareY = displaySizeY/2-20;

	// Square path iterator
	float phiSquare = acos(-1.0);

	// Circle position
	uint16_t circleX = displaySizeX/2-50;
	uint16_t circleY = displaySizeY/2;
	// Circle path iterator
	float phiCircle = 0;

	// Declare triangle
	point triangle[] = {
			{ trianglePositionX1, trianglePositionY1 },
			{ trianglePositionX2, trianglePositionY2 },
			{ trianglePositionX3, trianglePositionY3 }
	};
	uint16_t triangleOffsetX = 0,
			triangleOffsetY = 0;

	// Static text position
	uint16_t staticTextX = 40;
	uint16_t staticTextY = displaySizeY-30;

	// Button press counter
	int btnA = 0, btnB = 0, btnC = 0, btnD = 0;

	// Start endless loop
	while(TRUE) {
		// wait for buffer swap
		while(xQueueReceive(JoystickQueue, &joystickPosition, 0) == pdTRUE)
			;

		// Clear background
		gdispClear(White);

		// Draw triangle
		gdispFillConvexPoly(triangleOffsetX, triangleOffsetY, triangle, 3, Green);

		// Draw circle
		gdispFillCircle(circleX, circleY, 20, Blue);
		// Iterate circle movement
		phiCircle += 0.1;
		circleX = 50 * cos(phiCircle) + displaySizeX/2;
		circleY = 50 * sin(phiCircle) + displaySizeY/2;

		// Draw square
		gdispFillArea(squareX, squareY, 40, 40, Red);
		// Iterate square movement
		phiSquare += 0.1;
		squareX = 50 * cos(phiSquare) + displaySizeX/2 - 20;
		squareY = 50 * sin(phiSquare) + displaySizeY/2 - 20;

		// Generate string with static text
		sprintf(str, "The quick brown fox jumps over the lazy dog.");
		// Print static text
		gdispDrawString(staticTextX, staticTextY, str, font1, Black);

		// Generate string with current joystick values
		sprintf( str, "Axis 1: %5d|Axis 2: %5d",
				 ADC_GetConversionValue(ESPL_ADC_Joystick_1),
				 ADC_GetConversionValue(ESPL_ADC_Joystick_2));
		// Print string of joystick values
		gdispDrawString(0, 0, str, font1, Black);

		// Iterate moving string movement
		stringPositionX += 1;
		if (stringPositionX == displaySizeX) {
			stringPositionX = 0;
		}
		// Generate moving string text
		sprintf(str, "Whee!!");
		// Print moving string
		gdispDrawString(stringPositionX, stringPositionY, str, font1, Black);

		// Generate string with current button values
		/*
		sprintf( str, "A: %d|B: %d|C %d|D: %d|E: %d|K: %d",
				 GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A),
				 GPIO_ReadInputDataBit(ESPL_Register_Button_B, ESPL_Pin_Button_B),
				 GPIO_ReadInputDataBit(ESPL_Register_Button_C, ESPL_Pin_Button_C),
				 GPIO_ReadInputDataBit(ESPL_Register_Button_D, ESPL_Pin_Button_D),
				 GPIO_ReadInputDataBit(ESPL_Register_Button_E, ESPL_Pin_Button_E),
				 GPIO_ReadInputDataBit(ESPL_Register_Button_K, ESPL_Pin_Button_K) );
		// Print string of button values
		gdispDrawString(0, 22, str, font1, Black);
		*/

		// Button count logic
		if (GPIO_ReadInputDataBit(ESPL_Register_Button_K, ESPL_Pin_Button_K) == 0){
			btnA = 0, btnB = 0, btnC = 0, btnD = 0;
		}
		else if (GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A) == 0){
			btnA += 1;
		}
		else if (GPIO_ReadInputDataBit(ESPL_Register_Button_B, ESPL_Pin_Button_B) == 0){
			btnB += 1;
		}
		else if (GPIO_ReadInputDataBit(ESPL_Register_Button_C, ESPL_Pin_Button_C) == 0){
			btnC += 1;
		}
		else if (GPIO_ReadInputDataBit(ESPL_Register_Button_D, ESPL_Pin_Button_D) == 0){
			btnD += 1;
		}
		// Generate string with button count
		sprintf( str, "A: %d|B: %d|C %d|D: %d", btnA, btnB, btnC, btnD);
		// Print string of joystick values
		gdispDrawString(0, 11, str, font1, Black);

		// Draw Circle in center of square, add joystick movement
		//circlePositionX = caveX + joystickPosition.x/2;
		//circlePositionY = caveY + joystickPosition.y/2;
		//gdispFillCircle(circlePositionX, circlePositionY, 10, Green);

		// Move entire screen except ADC output and button counters according to joystick
		//stringPositionX = -64 + joystickPosition.x/2;
		stringPositionY = -34 + joystickPosition.y/2;

		triangleOffsetX = -64+joystickPosition.x/2;
		triangleOffsetY = -64+joystickPosition.y/2;

		staticTextX = -24 + joystickPosition.x/2;
		staticTextY = displaySizeY-94 + joystickPosition.y/2;

		squareX -= 64;
		circleX -= 64;
		squareY -= 64;
		circleY -= 64;
		squareX += joystickPosition.x/2;
		circleX += joystickPosition.x/2;
		squareY += joystickPosition.y/2;
		circleY += joystickPosition.y/2;

		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();

	}
}







void circleBlink1() {
	printf("CIRCLE BLINK 1 RUNS!!!!!!!!!!!!!!!!!!!!!");
	struct coord joystickPosition;
	const TickType_t xTicksToDelay = 50;

	// enter endless loop
	while (TRUE) {
		// wait for buffer swap
		while(xQueueReceive(JoystickQueue, &joystickPosition, 0) == pdTRUE);

		gdispClear(White);
		gdispDrawCircle(0,0,80,Black);
		vTaskDelay(xTicksToDelay);
		gdispClear(White);
		vTaskDelay(1000-xTicksToDelay);

		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();
	}
}


void circleBlink2() {
	printf("CIRCLE BLINK 2 RUNS!!!!!!!!!!!!!!!!!!!!!");
	struct coord joystickPosition;
	const TickType_t xTicksToDelay = 50;

	// enter endless loop
	while (TRUE) {
		// wait for buffer swap
		while(xQueueReceive(JoystickQueue, &joystickPosition, 0) == pdTRUE);

		gdispClear(White);
		gdispDrawCircle(0,0,80,Red);
		vTaskDelay(xTicksToDelay);
		gdispClear(White);
		vTaskDelay(1000-xTicksToDelay);

		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();
	}
}

/**
 * This task polls the joystick value every 20 ticks
 */
void checkJoystick() {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	struct coord joystickPosition = {0, 0};
	const TickType_t tickFramerate = 20;

	while (TRUE) {
		// Remember last joystick values
		joystickPosition.x =
					(uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4);
		joystickPosition.y = (uint8_t) 255 -
						 (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4);

		xQueueSend(JoystickQueue, &joystickPosition, 100);

		// Execute every 20 Ticks
		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
	}
}

/**
 * Example function to send data over UART
 *
 * Sends coordinates of a given position via UART.
 * Structure of a package:
 *  8 bit start byte
 *  8 bit x-coordinate
 *  8 bit y-coordinate
 *  8 bit checksum (= x-coord XOR y-coord)
 *  8 bit stop byte
 */
void sendPosition(struct coord position) {
	const uint8_t checksum = position.x^position.y;

	UART_SendData(startByte);
	UART_SendData(position.x);
	UART_SendData(position.y);
	UART_SendData(checksum);
	UART_SendData(stopByte);
}

/**
 * Example how to receive data over UART (see protocol above)
 */
void uartReceive() {
	char input;
	uint8_t pos = 0;
	char checksum;
	char buffer[5]; // Start byte,4* line byte, checksum (all xor), End byte
	struct coord position = {0, 0};
	while (TRUE) {
		// wait for data in queue
		xQueueReceive(ESPL_RxQueue, &input, portMAX_DELAY);

		// decode package by buffer position
		switch(pos) {
		// start byte
		case 0:
			if(input != startByte)
				break;
		case 1:
		case 2:
		case 3:
			// read received data in buffer
			buffer[pos] = input;
			pos++;
			break;
		case 4:
			// Check if package is corrupted
			checksum = buffer[1]^buffer[2];
			if(input == stopByte || checksum == buffer[3]) {
				// pass position to Joystick Queue
				position.x = buffer[1];
				position.y = buffer[2];
				xQueueSend(JoystickQueue, &position, 100);
			}
			pos = 0;
		}
	}
}


/*
 *  Hook definitions needed for FreeRTOS to function.
 */
void vApplicationIdleHook() {
	while (TRUE) {
	};
}

void vApplicationMallocFailedHook() {
	while(TRUE) {
	};
}

// Macro to use CCM (Core Coupled Memory) in STM32F4
#define CCM_RAM __attribute__((section(".ccmram")))

StaticTask_t xIdleTaskTCB CCM_RAM;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE] CCM_RAM;

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
  /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
  state will be stored. */
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  /* Pass out the array that will be used as the Idle task's stack. */
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
  Note that, as the array is necessarily of type StackType_t,
  configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTaskTCB CCM_RAM;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH] CCM_RAM;

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
		StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
