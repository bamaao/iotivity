/******************************************************************
 *
 * Copyright 2014 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include <stdlib.h>

#include "canetworkconfigurator.h"
#include "cainterfacecontroller.h"
#include "cainterface.h"
#include "uarraylist.h"
#include "logger.h"

#define TAG "OIC_CA_NW_CONFIG"

static u_arraylist_t *g_selectedNetworkList = NULL;
static CATransportAdapter_t CASelectedNetwork = CA_DEFAULT_ADAPTER;

#ifdef EDR_ADAPTER
static uint32_t NETWORK_RFCOMM = CA_ADAPTER_RFCOMM_BTEDR;
#endif

#ifdef LE_ADAPTER
static uint32_t NETWORK_GATT = CA_ADAPTER_GATT_BTLE;
#endif

#ifdef IP_ADAPTER
static uint32_t NETWORK_IP = CA_ADAPTER_IP;
#endif

#ifdef NFC_ADAPTER
static uint32_t NETWORK_NFC = CA_ADAPTER_NFC;
#endif

#ifdef RA_ADAPTER
static uint32_t NETWORK_RA = CA_ADAPTER_REMOTE_ACCESS;
#endif

#ifdef TCP_ADAPTER
static uint32_t NETWORK_TCP = CA_ADAPTER_TCP;
#endif

CAResult_t CAAddNetworkType(CATransportAdapter_t transportType)
{
    if (NULL == g_selectedNetworkList)
    {
        OIC_LOG(DEBUG, TAG, "Create network list");

        g_selectedNetworkList = u_arraylist_create();

        if (NULL == g_selectedNetworkList)
        {
            return CA_MEMORY_ALLOC_FAILED;
        }
    }
    bool res = true;
    switch (transportType)
    {
        case CA_ADAPTER_IP:
#ifndef IP_ADAPTER
            return CA_NOT_SUPPORTED;
#else

            if (u_arraylist_contains(g_selectedNetworkList, &NETWORK_IP))
            {
                goto exit;
            }
            res = u_arraylist_add(g_selectedNetworkList, &NETWORK_IP);
            CASelectedNetwork |= CA_ADAPTER_IP;

            break;
#endif // IP_ADAPTER

        case CA_ADAPTER_RFCOMM_BTEDR:
#ifndef EDR_ADAPTER
            return CA_NOT_SUPPORTED;
#else

            if (u_arraylist_contains(g_selectedNetworkList, &NETWORK_RFCOMM))
            {
                goto exit;
            }
            res = u_arraylist_add(g_selectedNetworkList, &NETWORK_RFCOMM);
            CASelectedNetwork |= CA_ADAPTER_RFCOMM_BTEDR;

            break;
#endif // EDR_ADAPTER

        case CA_ADAPTER_GATT_BTLE:
#ifndef LE_ADAPTER
            return CA_NOT_SUPPORTED;
#else

            if (u_arraylist_contains(g_selectedNetworkList, &NETWORK_GATT))
            {
                goto exit;
            }
            res = u_arraylist_add(g_selectedNetworkList, &NETWORK_GATT);
            CASelectedNetwork |= CA_ADAPTER_GATT_BTLE;

            break;
#endif // LE_ADAPTER

#ifdef RA_ADAPTER
        case CA_ADAPTER_REMOTE_ACCESS:

           if (u_arraylist_contains(g_selectedNetworkList, &NETWORK_RA))
           {
               goto exit;
           }
           res = u_arraylist_add(g_selectedNetworkList, &NETWORK_RA);
           CASelectedNetwork |= CA_ADAPTER_REMOTE_ACCESS;
           break;
#endif // RA_ADAPTER

#ifdef TCP_ADAPTER
        case CA_ADAPTER_TCP:

           if (u_arraylist_contains(g_selectedNetworkList, &NETWORK_TCP))
           {
               goto exit;
           }
           res = u_arraylist_add(g_selectedNetworkList, &NETWORK_TCP);
           CASelectedNetwork |= CA_ADAPTER_TCP;
           break;
#endif // TCP_ADAPTER


        case CA_ADAPTER_NFC:
 #ifndef NFC_ADAPTER
            return CA_NOT_SUPPORTED;

#else
           if (u_arraylist_contains(g_selectedNetworkList, &NETWORK_NFC))
           {
               goto exit;
           }
           res = u_arraylist_add(g_selectedNetworkList, &NETWORK_NFC);
           CASelectedNetwork |= CA_ADAPTER_NFC;

           break;
#endif // CA_ADAPTER_NFC

        default:
            break;
    }

    if (!res)
    {
        OIC_LOG_V(ERROR, TAG, "Add arraylist failed[Err code: %d]", res);
        return CA_STATUS_FAILED;
    }
    // start selected interface adapter
    CAResult_t result = CAStartAdapter(transportType);
    return result;

exit:
    OIC_LOG(DEBUG, TAG, "This adapter is already enabled");
    return CA_STATUS_OK;
}

CAResult_t CARemoveNetworkType(CATransportAdapter_t transportType)
{
    if (NULL == g_selectedNetworkList)
    {
        OIC_LOG(ERROR, TAG, "SelectedNetwork list is NULL");

        return CA_STATUS_FAILED;
    }

    uint32_t selectedNetworkLength = u_arraylist_length(g_selectedNetworkList);
    for (uint32_t index = 0; index < selectedNetworkLength; index++)
    {
        void* ptrType = u_arraylist_get(g_selectedNetworkList, index);
        if (NULL == ptrType)
        {
            continue;
        }

        CATransportAdapter_t connType = *(CATransportAdapter_t *)ptrType;

        if (transportType == connType)
        {
            switch (transportType)
            {
                case CA_ADAPTER_IP:
#ifndef IP_ADAPTER
                    OIC_LOG(DEBUG, TAG, "Remove network type(IP) - Not Supported");
                    return CA_NOT_SUPPORTED;
#else

                    OIC_LOG(DEBUG, TAG, "Remove network type(IP)");
                    u_arraylist_remove(g_selectedNetworkList, index);
                    CASelectedNetwork = CASelectedNetwork & (~CA_ADAPTER_IP);

                    break;
#endif //IP_ADAPTER

                case CA_ADAPTER_RFCOMM_BTEDR:
#ifndef EDR_ADAPTER
                    OIC_LOG(DEBUG, TAG, "Remove network type(EDR) - Not Supported");
                    return CA_NOT_SUPPORTED;
#else
                    OIC_LOG(DEBUG, TAG, "Remove network type(EDR)");
                    u_arraylist_remove(g_selectedNetworkList, index);
                    CASelectedNetwork = CASelectedNetwork & (~CA_ADAPTER_RFCOMM_BTEDR);

                    break;
#endif // EDR_ADAPTER

                case CA_ADAPTER_GATT_BTLE:
#ifndef LE_ADAPTER
                    OIC_LOG(DEBUG, TAG, "Remove network type(LE) - Not Supported");
                    return CA_NOT_SUPPORTED;
#else
                    OIC_LOG(DEBUG, TAG, "Remove network type(LE)");
                    u_arraylist_remove(g_selectedNetworkList, index);
                    CASelectedNetwork = CASelectedNetwork & (~CA_ADAPTER_GATT_BTLE);
                    break;
#endif // LE_ADAPTER

#ifdef RA_ADAPTER
                case CA_ADAPTER_REMOTE_ACCESS:
                    OIC_LOG(DEBUG, TAG, "Remove network type(RA)");
                    u_arraylist_remove(g_selectedNetworkList, index);
                    CASelectedNetwork = CASelectedNetwork & (~CA_ADAPTER_REMOTE_ACCESS);
                    break;
#endif // RA_ADAPTER

#ifdef TCP_ADAPTER
                case CA_ADAPTER_TCP:
                    OIC_LOG(DEBUG, TAG, "Remove network type(TCP)");
                    u_arraylist_remove(g_selectedNetworkList, index);
                    CASelectedNetwork = CASelectedNetwork & (~CA_ADAPTER_TCP);
                    break;
#endif // TCP_ADAPTER

#ifndef NFC_ADAPTER
                case CA_ADAPTER_NFC:
                    OIC_LOG(DEBUG, TAG, "Add network type(NFC) - Not Supported");
                    return CA_NOT_SUPPORTED;
#else
                   OIC_LOG(DEBUG, TAG, "Add network type(NFC)");
                   u_arraylist_remove(g_selectedNetworkList, index);
                   CASelectedNetwork = CASelectedNetwork & (~CA_ADAPTER_NFC);
                   break;
#endif // CA_ADAPTER_NFC
                default:
                    break;
            }

            // stop selected interface adapter
            CAStopAdapter(connType);
            return CA_STATUS_OK;
        }
    }

    return CA_STATUS_FAILED;
}

u_arraylist_t *CAGetSelectedNetworkList()
{
    return g_selectedNetworkList;
}

CATransportAdapter_t CAGetSelectedNetwork()
{
    return CASelectedNetwork;
}

CAResult_t CAGetNetworkInformationInternal(CAEndpoint_t **info, uint32_t *size)
{
    OIC_LOG(DEBUG, TAG, "get network information.");

    if (NULL == info || NULL == size)
    {
        OIC_LOG(ERROR, TAG, "Input parameter is invalid value");

        return CA_STATUS_INVALID_PARAM;
    }

    return CAGetNetworkInfo(info, size);
}

CAResult_t CATerminateNetworkType()
{
    OIC_LOG(DEBUG, TAG, "CATerminateNetworkType()");
    if (NULL != g_selectedNetworkList)
    {
        u_arraylist_free(&g_selectedNetworkList);
    }
    return CA_STATUS_OK;
}
