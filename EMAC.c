

#include "EMAC.h"

/*----------------------------------------------------------------------------
 *      EMAC 1 'EMAC_Name': Sample EMAC
 *---------------------------------------------------------------------------*/

SignalR *sigR;
osPoolDef (SignalR_pool, 1, SignalR);  // Declare memory pool
osPoolId  (SignalR_pool_id);                 // Memory pool ID

void EMAC (void const *argument);                             // EMAC function
osThreadId tid_EMAC;                                          // EMAC id
osThreadDef (EMAC, osPriorityNormal, 1, 0);                   // EMAC object


uint32_t tcp_cb_client (int32_t socket, netTCP_Event event,
                        const NET_ADDR *addr, const uint8_t *buf, uint32_t len);

int Init_EMAC (void) {

	sigR = SignalR_Create();
	netInitialize();
  tid_EMAC = osThreadCreate (osThread(EMAC), NULL);
  if (!tid_EMAC) return(-1);
  
  return(0);
}

void EMAC (void const *argument) {
	NET_ADDR addr = { NET_ADDR_IP4, 8000,
                    100, 0, 0, 1};	
	
	
	uint8_t *http_headers;
	uint32_t maxlen;
										
	int32_t tcp_sock = netTCP_GetSocket (tcp_cb_client);	
  netTCP_SetOption (tcp_sock, netTCP_OptionKeepAlive, 1);
	osDelay(100);
  while (1) {

		if (tcp_sock > 0) {
			switch (netTCP_GetState (tcp_sock)) {
				case netTCP_StateUNUSED:
				case netTCP_StateCLOSED:
					// Connect to TCP socket server on port 2000
					netTCP_Connect (tcp_sock, &addr, 0);
					break;
	 
				case netTCP_StateESTABLISHED:
					// Connected, send the data
					if (netTCP_SendReady (tcp_sock) == true) {							
						switch(SignalR_GetState(sigR)){						
							case SignalR_StateCLOSED:
								switch(SignalR_GetSubState(sigR)){
									case SignalR_SubStateNEGOTIATE:
										maxlen  = netTCP_GetMaxSegmentSize (tcp_sock);				
										http_headers = netTCP_GetBuffer (maxlen);
										sprintf((char*)http_headers, "GET /signalr/negotiate?clientProtocol=1.5&connectionData=[{\"name\":\"myHub\"}] HTTP/1.1\r\nHost: 192.168.90.90:8000\r\n\r\n");
										#ifdef DEBUG_PRINTF
											printf("%s",http_headers);
										#endif									
										netTCP_Send (tcp_sock, http_headers, maxlen);
										osDelay(1000);
										break;
									case SignalR_SubStateCONNECT:
										http_headers = netTCP_GetBuffer (maxlen);
									  sprintf((char*)http_headers, "GET /signalr/connect?transport=serverSentEvents&clientProtocol=1.5&connectionToken=%s&connectionData=[{\"name\":\"myHub\"}] HTTP/1.1\r\nHost: 192.168.90.90:8000\r\n\r\n",sigR->connectionToken);
										#ifdef DEBUG_PRINTF
											printf("%s/n",http_headers);
										#endif						
										netTCP_Send (tcp_sock, http_headers, maxlen);
										osDelay(1000);
										break;
									case SignalR_SubStateSTART:
										http_headers = netTCP_GetBuffer (maxlen);
									  sprintf((char*)http_headers, "GET /signalr/start?transport=serverSentEvents&clientProtocol=1.5&connectionToken=%s&connectionData=[{\"name\":\"myHub\"}] HTTP/1.1\r\nHost: 192.168.90.90:8000\r\n\r\n",sigR->connectionToken);
										#ifdef DEBUG_PRINTF
											printf("%s/n",http_headers);
										#endif						
										netTCP_Send (tcp_sock, http_headers, maxlen);
										osDelay(1000);
										break;
									case SignalR_SubStateNULLSTATE:
										break;
								}
								break;
							case SignalR_StateESTABLISHED:
								osDelay(1000);
								http_headers = netTCP_GetBuffer (maxlen);
								sprintf((char*)http_headers, "POST /signalr/send?transport=serverSentEvents&clientProtocol=1.5&connectionToken=%s&connectionData=[{\"name\":\"myHub\"}] HTTP/1.1\r\nHost: 192.168.90.90:8000\r\nContent-Length: 58\r\n\r\n data={\"H\":\"myHub\",\"M\":\"Send\",\"A\":[\"a\",\"test msg\"],\"I\":0}",sigR->connectionToken);
								#ifdef DEBUG_PRINTF
									printf("%s/n",http_headers);
								#endif						
								netTCP_Send (tcp_sock, http_headers, maxlen);		
								break;
						}
					}
					
					break;
	 

				default:
					break;
			}
		}
    osThreadYield ();                                           // suspend EMAC
  }
}

// Notify the user application about TCP socket events.
uint32_t tcp_cb_client (int32_t socket, netTCP_Event event,
                        const NET_ADDR *addr, const uint8_t *buf, uint32_t len) {
	char *js;
	volatile uint8_t got_http_version = 0, got_http_status_ok = 0, i;		
	uint32_t  j;													
	jsmn_parser parser;
	jsmntok_t tokens[25];
	jsmn_init(&parser);
	
	
  switch (event) {
    case netTCP_EventConnect:
      // Connect request received
      break;
 
    case netTCP_EventEstablished:
      // Connection established
      break;
 
    case netTCP_EventClosed:
      // Connection was properly closed
      break;
 
    case netTCP_EventAborted:
      // Connection is for some reason aborted
      break;
 
    case netTCP_EventACK:
      // Previously sent data acknowledged
      break;
 
    case netTCP_EventData:
			#ifdef DEBUG_PRINTF
				printf("%s/n/n",buf);
			#endif
			switch(SignalR_GetState(sigR)){
				case SignalR_StateCLOSED:
					switch(SignalR_GetSubState(sigR)){
						case SignalR_SubStateNEGOTIATE:
							js = strchr((char*)buf, '{');
							if(js!=NULL) jsmn_parse(&parser, js, strlen(js), tokens,25);
							for(i=0;i < NELEMS(tokens); i++){
								if(tokens[i].type != JSMN_STRING)continue;
								if(memcmp("ConnectionToken", js+tokens[i].start, tokens[i].end - tokens[i].start)==0){
									for(j=0;j<tokens[i+1].end - tokens[i+1].start;j++){
										if(	*(js + tokens[i+1].start + j) == '+'){
											strcat(sigR->connectionToken,"%2B");
											continue;
										}
										if( *(js + tokens[i+1].start + j) == '/'){
											strcat(sigR->connectionToken,"%2F");
											continue;
										}
										if( *(js + tokens[i+1].start + j) == '='){
											strcat(sigR->connectionToken,"%3D");
											continue;
										}
										strncat(sigR->connectionToken,(js + tokens[i+1].start + j),1);				
									}			
									#ifdef DEBUG_PRINTF
										printf("%s",sigR->connectionToken);
									#endif
									sigR->subStateID = SignalR_SubStateCONNECT; 
									osSignalSet (tid_EMAC, SIGNALR_EVENT_NEGOTIATE);  
								}
							}
							break;
						case SignalR_SubStateCONNECT:
							js = strchr((char*)buf, '{');
							if(js!=NULL){
								jsmn_parse(&parser, js, strlen(js), tokens,25);
								for(i=0;i < NELEMS(tokens); i++){
									if(tokens[i].type != JSMN_STRING)continue;
									if(memcmp("S", js+tokens[i].start, tokens[i].end - tokens[i].start)==0){
										if(	*(js + tokens[i+1].start) == '1'){
											sigR->subStateID = SignalR_SubStateSTART; 
											osSignalSet (tid_EMAC, SIGNALR_EVENT_CONNECT); 
											break;
										}												
									}
								}
							}					
							break;
						case SignalR_SubStateSTART:
							js = strstr((char*)buf, "{ \"Response\": \"started\" }" );
							if(js!=NULL){
								sigR->stateID = SignalR_StateESTABLISHED;
								sigR->subStateID = SignalR_SubStateNULLSTATE;
								osSignalSet (tid_EMAC, SIGNALR_EVENT_START); 
							}
							break;
						case SignalR_SubStateNULLSTATE:
							break;	 
					}
				case SignalR_StateESTABLISHED:	
					
				break;
			}
  }
  return (0);
}

SignalR* SignalR_Create(void){
	SignalR* me;
	SignalR_pool_id = osPoolCreate(osPool(SignalR_pool));
	me = (SignalR *)  osPoolAlloc(SignalR_pool_id);
	if(me != NULL)
	{
		SignalR_Init(me);
	}
	return me;
}

int32_t SignalR_Init(SignalR *me){
	if(me == NULL) return -1;
	me->stateID = SignalR_StateCLOSED;
	me->subStateID = SignalR_SubStateNEGOTIATE;
	return 0;
}
	
SignalR_State SignalR_GetState(SignalR *me){
	return me->stateID;
}

SignalR_SubState SignalR_GetSubState(SignalR *me){
	return me->subStateID;
}

