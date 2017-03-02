// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "threadapi.h"
#include "crt_abstractions.h"
#include "iothub_client.h"
#include "iothub_message.h"
#include "iothubtransportamqp.h"

#ifdef MBED_BUILD_TIMESTAMP
#include "certs.h"
#endif // MBED_BUILD_TIMESTAMP

/*String containing Hostname, Device Id & Device Key in the format:                         */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"                */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessSignature=<device_sas_token>"    */
static const char* connectionString = "[device connection string]";

static int callbackCounter;
static bool g_continueRunning;
static char msgText[1024];
static char propText[1024];

static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
    int* counter = (int*)userContextCallback;
    const unsigned char* buffer = NULL;
    size_t size = 0;
    const char* messageId;
    const char* correlationId;

    // Message properties
    if ((messageId = IoTHubMessage_GetMessageId(message)) == NULL)
    {
        messageId = "<null>";
    }

    if ((correlationId = IoTHubMessage_GetCorrelationId(message)) == NULL)
    {
        correlationId = "<null>";
    }

    // Message content
    IOTHUBMESSAGE_CONTENT_TYPE contentType = IoTHubMessage_GetContentType(message);

    if (contentType == IOTHUBMESSAGE_BYTEARRAY)
    {
        if (IoTHubMessage_GetByteArray(message, &buffer, &size) == IOTHUB_MESSAGE_OK)
        {
            (void)printf("Received Message [%d]\r\n Message ID: %s\r\n Correlation ID: %s\r\n BINARY Data: <<<%.*s>>> & Size=%d\r\n", *counter, messageId, correlationId, (int)size, buffer, (int)size);
        }
        else
        {
            (void)printf("Failed getting the BINARY body of the message received.\r\n");
        }
    }
    else if (contentType == IOTHUBMESSAGE_STRING)
    {
        if ((buffer = (const unsigned char*)IoTHubMessage_GetString(message)) != NULL && (size = strlen((const char*)buffer)) > 0)
        {
            (void)printf("Received Message [%d]\r\n Message ID: %s\r\n Correlation ID: %s\r\n STRING Data: <<<%.*s>>> & Size=%d\r\n", *counter, messageId, correlationId, (int)size, buffer, (int)size);

            // If we receive the work 'quit' then we stop running
        }
        else
        {
            (void)printf("Failed getting the STRING body of the message received.\r\n");
        }
    }
    else
    {
        (void)printf("Failed getting the body of the message received (type %i).\r\n", contentType);
    }

    // Retrieve properties from the message
    MAP_HANDLE mapProperties = IoTHubMessage_Properties(message);
    if (mapProperties != NULL)
    {
        const char*const* keys;
        const char*const* values;
        size_t propertyCount = 0;
        if (Map_GetInternals(mapProperties, &keys, &values, &propertyCount) == MAP_OK)
        {
            if (propertyCount > 0)
            {
                size_t index;

                printf(" Message Properties:\r\n");
                for (index = 0; index < propertyCount; index++)
                {
                    printf("\tKey: %s Value: %s\r\n", keys[index], values[index]);
                }
                printf("\r\n");
            }
        }
    }

	if (memcmp(buffer, "quit", size) == 0)
    {
        g_continueRunning = false;
    }

    /* Some device specific action code goes here... */
    (*counter)++;
    return IOTHUBMESSAGE_ACCEPTED;
}

static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
	IOTHUB_MESSAGE_HANDLE* eventInstance = (IOTHUB_MESSAGE_HANDLE*)userContextCallback;
	MAP_HANDLE propMap = IoTHubMessage_Properties(eventInstance);

    (void)printf("Confirmation[%d] received for message tracking id = %zu with result = %s\r\n",
    		callbackCounter, Map_GetValueFromKey(propMap, "PropName"), ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
    /* Some device specific action code goes here... */
    callbackCounter++;
    IoTHubMessage_Destroy(eventInstance);
}

void iothub_client_sample_amqp_run(void)
{
    IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

    IOTHUB_MESSAGE_HANDLE messages;

    g_continueRunning = true;
    srand((unsigned int)time(NULL));
    double temperature = 10.0;

    callbackCounter = 0;
    int receiveContext = 0;

    (void)printf("Starting the IoTHub client sample AMQP...\r\n");

    if (platform_init() != 0)
    {
        printf("Failed to initialize the platform.\r\n");
    }
    else
    {
        if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, AMQP_Protocol)) == NULL)
        {
            (void)printf("ERROR: iotHubClientHandle is NULL!\r\n");
        }
        else
        {
            bool traceOn = true;
            IoTHubClient_LL_SetOption(iotHubClientHandle, "logtrace", &traceOn);

            /* Setting Message call back, so we can receive Commands. */
            if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK)
            {
                (void)printf("ERROR: IoTHubClient_SetMessageCallback..........FAILED!\r\n");
            }
            else
            {
                (void)printf("IoTHubClient_SetMessageCallback...successful.\r\n");
                size_t id = 0;
                /* Now that we are ready to receive commands, let's send some messages */
                do
                {
                	sprintf_s(msgText, sizeof(msgText), "{\"deviceId\":\"myFirstDevice\",\"temperature\":%.2f}", temperature + (rand() % 4 + 2));
					if ((messages = IoTHubMessage_CreateFromByteArray((const unsigned char*)msgText, strlen(msgText))) == NULL)
					{
						(void)printf("ERROR: iotHubMessageHandle is NULL!\r\n");
					}
					else
					{

						MAP_HANDLE propMap = IoTHubMessage_Properties(messages);
						(void)sprintf_s(propText, sizeof(propText), "RaspivnMsg_%zu", id);
						if (Map_AddOrUpdate(propMap, "PropName", propText) != MAP_OK)
						{
							(void)printf("ERROR: Map_AddOrUpdate Failed!\r\n");
						}

						if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messages, SendConfirmationCallback, &messages) != IOTHUB_CLIENT_OK)
						{
							(void)printf("ERROR: IoTHubClient_SendEventAsync..........FAILED!\r\n");
						}
						else
						{
							(void)printf("IoTHubClient_SendEventAsync accepted data for transmission to IoT Hub.\r\n");
						}
					}
                    IoTHubClient_LL_DoWork(iotHubClientHandle);
                    ThreadAPI_Sleep(1);

                    id++;
                    if(id == 10000){
                    	id = 0;
                    }
                } while (g_continueRunning);
            }
            IoTHubClient_LL_Destroy(iotHubClientHandle);
        }
        platform_deinit();
    }
}

int main(void)
{
    iothub_client_sample_amqp_run();
    return 0;
}
