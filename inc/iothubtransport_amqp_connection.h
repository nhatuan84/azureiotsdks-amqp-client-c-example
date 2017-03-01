// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef IOTHUBTRANSPORTAMQP_AMQP_CONNECTION_H
#define IOTHUBTRANSPORTAMQP_AMQP_CONNECTION_H

#include "umock_c_prod.h"
#include "xio.h"
#include "azure_uamqp_c/session.h"
#include "azure_uamqp_c/cbs.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum AMQP_CONNECTION_STATE_TAG
{
	AMQP_CONNECTION_STATE_OPENED,
	AMQP_CONNECTION_STATE_CLOSED,
	AMQP_CONNECTION_STATE_ERROR
} AMQP_CONNECTION_STATE;

typedef void(*ON_AMQP_CONNECTION_STATE_CHANGED)(const void* context, AMQP_CONNECTION_STATE old_state, AMQP_CONNECTION_STATE new_state);

typedef struct AMQP_CONNECTION_CONFIG_TAG
{
	const char* iothub_host_fqdn;
	XIO_HANDLE underlying_io_transport;
	bool create_sasl_io;
	bool create_cbs_connection;
	bool is_trace_on;
		
	ON_AMQP_CONNECTION_STATE_CHANGED on_state_changed_callback;
	const void* on_state_changed_context;
} AMQP_CONNECTION_CONFIG;

typedef struct AMQP_CONNECTION_INSTANCE* AMQP_CONNECTION_HANDLE;

MOCKABLE_FUNCTION(, AMQP_CONNECTION_HANDLE, amqp_connection_create, AMQP_CONNECTION_CONFIG*, config);
MOCKABLE_FUNCTION(, void, amqp_connection_destroy, AMQP_CONNECTION_HANDLE, conn_handle);
MOCKABLE_FUNCTION(, void, amqp_connection_do_work, AMQP_CONNECTION_HANDLE, conn_handle);
MOCKABLE_FUNCTION(, int, amqp_connection_get_session_handle, AMQP_CONNECTION_HANDLE, conn_handle, SESSION_HANDLE*, session_handle);
MOCKABLE_FUNCTION(, int, amqp_connection_get_cbs_handle, AMQP_CONNECTION_HANDLE, conn_handle, CBS_HANDLE*, cbs_handle);
MOCKABLE_FUNCTION(, int, amqp_connection_set_logging, AMQP_CONNECTION_HANDLE, conn_handle, bool, is_trace_on);
	
#ifdef __cplusplus
}
#endif

#endif /*IOTHUBTRANSPORTAMQP_AMQP_CONNECTION_H*/
