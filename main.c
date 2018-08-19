/**************************************************************************//**
 * @file     main.c
 * @brief    
 * @version  V1.0
 * @date     20. June 2018
 * @author   
 * @note     Copyright (C) CataCompany. All rights reserved.

******************************************************************************/

#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os.h"
#include "EMAC.h"


#ifdef RTE_Compiler_EventRecorder
#include "EventRecorder.h"
#endif

int main (void) {

  // System Initialization
 SystemCoreClockUpdate();
#ifdef DEBUG
  // Initialize and start Event Recorder
  EventRecorderInitialize(EventRecordError, 1U);
#endif
  // ...
	
  osKernelInitialize();                 // Initialize CMSIS-RTOS
	Init_EMAC();
	
  osKernelStart();                      // Start thread execution

  //for (;;) {}
}

