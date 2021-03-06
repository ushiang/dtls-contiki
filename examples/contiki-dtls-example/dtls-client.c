/*
 * Example client application using DTLS library
 * TLS implementation for Contiki OS
 * Copyright (c) 2012, Vladislav Perelman <vladislav.perelman@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#include <contiki.h>
#include <contiki-net.h>
#include <contiki-lib.h>
#include <dtls.h>
#include <string.h>
#include "lib/mmem.h"
#define SEND_INTERVAL 1 * CLOCK_CONF_SECOND
#define CLOSE_INTERVAL 120 * CLOCK_CONF_SECOND // This will be the lenght of the connection


#if CONTIKI_TARGET_MINIMAL_NET

#ifndef DEBUG
#define DEBUG DEBUG_PRINT
#endif

#include "net/uip-debug.h"

#endif


PROCESS(dtls_client_test_process, "DTLS client");
AUTOSTART_PROCESSES(&dtls_client_test_process);
static Connection* connection;
static struct etimer et;
static struct etimer et2;
static char* hello_msg = "Hello World!";
static uip_ipaddr_t ipaddr;
static void dtls_handler(process_event_t ev, process_data_t data){
	
	if (ev == dtls_event){
		
		if (dtls_rehandshake()){
			etimer_stop(&et);
		} else
		if (dtls_connected()){
			#if CONTIKI_TARGET_MINIMAL_NET
			PRINTF("CONNECTED\n");
			#endif
			connection = (Connection*)data;
			etimer_set(&et, SEND_INTERVAL);
			DTLS_Write(connection, hello_msg, strlen(hello_msg));
		} else if (dtls_newdata()){
			dtls_appdata[dtls_applen] = 0;
			#if CONTIKI_TARGET_MINIMAL_NET
			PRINTF("GOT NEW DATA: %s\n", dtls_appdata);
			#endif
		}
		
		
	} else if (ev == PROCESS_EVENT_TIMER){
		if (etimer_expired(&et)){
			DTLS_Write(connection, hello_msg, strlen(hello_msg));
			etimer_reset(&et);
		}
		if (etimer_expired(&et2)){
			#if CONTIKI_TARGET_MINIMAL_NET
			PRINTF("Client Sending DTLS Close Connection\n\n");
			DTLS_Close(connection);
			#endif
			etimer_stop(&et);
			etimer_stop(&et2);
		}
	}


}
static void set_connection_address(uip_ipaddr_t *ipaddr)
{
	// use uip_ip6addr to set the address of the server to connect to
	uip_ip6addr(ipaddr, 0xfe80,0x0000,0x0000,0x0000,0x0000,0x00ff,0xfe02,0x00232); // Use Local-Link Adress if used prefix aaaa::/64 Contiki 2.7 has some bug on the neighbour table that makes it crash
}

PROCESS_THREAD(dtls_client_test_process, ev, data)
{
  PROCESS_BEGIN();
	
	set_connection_address(&ipaddr);
	etimer_set(&et, CLOCK_CONF_SECOND*10);
  	PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
  	etimer_set(&et2, CLOSE_INTERVAL);
	DTLS_Connect(&ipaddr, 4433);
	while(1){
		PROCESS_YIELD();
		
		#if CONTIKI_TARGET_MINIMAL_NET
		PRINTF("Client Awaken!\n");
		#endif
		
		dtls_handler(ev, data);
	}
  PROCESS_END();
}
