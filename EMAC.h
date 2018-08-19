

#ifndef __EMAC_H
#define __EMAC_H

#include <stdio.h>
#include <string.h>
#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "rl_net.h"
#include "jsmn.h"

#ifdef DEBUG
//#include "EventRecorder.h"
#endif

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0])) //returns the number os elements in byte array

int Init_EMAC (void);

typedef enum {
	SignalR_StateESTABLISHED,
	SignalR_StateCLOSED
}SignalR_State;

typedef enum {
	SignalR_SubStateNEGOTIATE,
	SignalR_SubStateCONNECT,
	SignalR_SubStateSTART,
	SignalR_SubStateNULLSTATE
}SignalR_SubState;

typedef struct {
	SignalR_State stateID;	
	SignalR_SubState subStateID;
	char connectionToken[450];
}SignalR;


SignalR* SignalR_Create(void);
int32_t SignalR_Init(SignalR *me);
SignalR_State SignalR_GetState(SignalR *me);
SignalR_SubState SignalR_GetSubState(SignalR *me);

#define SIGNALR_EVENT_NEGOTIATE (1 << 0)
#define SIGNALR_EVENT_CONNECT 	(1 << 1)
#define SIGNALR_EVENT_START 		(1 << 2)



//Event Recorder Components
#ifdef DEBUG
#define EvtEMACManager_No 0x08  
#define EvtEMACManager_TCPevents     	      		EventID (EventLevelOp,       EvtEMACManager_No, 0x00U)

#endif

#endif

