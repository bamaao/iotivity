/* *****************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * *****************************************************************/

#include <memory.h>
#include "ocstack.h"
#include "securevirtualresourcetypes.h"
#include "doxmresource.h"
#include "cacommon.h"
#include "cainterface.h"
#include "oic_malloc.h"
#include "logger.h"
#include "pmtypes.h"
#include "ownershiptransfermanager.h"
#include "mbedtls/ssl_ciphersuites.h"

#define TAG "OIC_OXM_JustWorks"

OCStackResult CreateJustWorksSelectOxmPayload(OTMContext_t *otmCtx, uint8_t **payload, size_t *size)
{
    if (!otmCtx || !otmCtx->selectedDeviceInfo || !payload || *payload || !size)
    {
        return OC_STACK_INVALID_PARAM;
    }

    otmCtx->selectedDeviceInfo->doxm->oxmSel = OIC_JUST_WORKS;
    *payload = NULL;
    *size = 0;

    return DoxmToCBORPayload(otmCtx->selectedDeviceInfo->doxm, payload, size, true);
}

OCStackResult CreateJustWorksOwnerTransferPayload(OTMContext_t* otmCtx, uint8_t **payload, size_t *size)
{
    if (!otmCtx || !otmCtx->selectedDeviceInfo || !payload || *payload || !size)
    {
        return OC_STACK_INVALID_PARAM;
    }

    OicUuid_t uuidPT = {.id={0}};

    if (OC_STACK_OK != GetDoxmDeviceID(&uuidPT))
    {
        OIC_LOG(ERROR, TAG, "Error while retrieving provisioning tool's device ID");
        return OC_STACK_ERROR;
    }
    memcpy(otmCtx->selectedDeviceInfo->doxm->owner.id, uuidPT.id , UUID_LENGTH);

    *payload = NULL;
    *size = 0;

    return DoxmToCBORPayload(otmCtx->selectedDeviceInfo->doxm, payload, size, true);
}

OCStackResult LoadSecretJustWorksCallback(OTMContext_t* UNUSED_PARAM)
{
    //In case of 'just works', secret data not required
    (void)UNUSED_PARAM;
    return OC_STACK_OK;
}

OCStackResult CreateSecureSessionJustWorksCallback(OTMContext_t* otmCtx)
{
    OIC_LOG(INFO, TAG, "IN CreateSecureSessionJustWorksCallback");
    if (!otmCtx || !otmCtx->selectedDeviceInfo)
    {
        return OC_STACK_INVALID_PARAM;
    }

    CAResult_t caresult = CAEnableAnonECDHCipherSuite(true);
    if (CA_STATUS_OK != caresult)
    {
        OIC_LOG_V(ERROR, TAG, "Unable to enable anon cipher suite");
        return OC_STACK_ERROR;
    }
    OIC_LOG(INFO, TAG, "Anonymous cipher suite Enabled.");

    caresult  = CASelectCipherSuite(MBEDTLS_TLS_ECDH_ANON_WITH_AES_128_CBC_SHA256, otmCtx->selectedDeviceInfo->endpoint.adapter);
    if (CA_STATUS_OK != caresult)
    {
        OIC_LOG_V(ERROR, TAG, "Failed to select TLS_ECDH_anon_WITH_AES_128_CBC_SHA256");
        caresult = CAEnableAnonECDHCipherSuite(false);
        if (CA_STATUS_OK != caresult)
        {
            OIC_LOG_V(ERROR, TAG, "Unable to enable anon cipher suite");
        }
        else
        {
            OIC_LOG(INFO, TAG, "Anonymous cipher suite Disabled.");
        }
        return OC_STACK_ERROR;
    }
    OIC_LOG(INFO, TAG, "TLS_ECDH_anon_WITH_AES_128_CBC_SHA256 cipher suite selected.");

    OCProvisionDev_t *selDevInfo = otmCtx->selectedDeviceInfo;
    CAEndpoint_t endpoint;
    memcpy(&endpoint, &selDevInfo->endpoint, sizeof(CAEndpoint_t));

    if(CA_ADAPTER_IP == endpoint.adapter)
    {
        endpoint.port = selDevInfo->securePort;
        caresult = CAInitiateHandshake(&endpoint);
    }
    else if (CA_ADAPTER_GATT_BTLE == endpoint.adapter)
    {
        caresult = CAInitiateHandshake(&endpoint);
    }
#ifdef __WITH_TLS__
    else
    {
        endpoint.port = selDevInfo->tcpPort;
        caresult = CAinitiateSslHandshake(&endpoint);
    }
#endif
    if (CA_STATUS_OK != caresult)
    {
        OIC_LOG_V(ERROR, TAG, "DTLS/TLS handshake failure.");
        return OC_STACK_ERROR;
    }

    OIC_LOG(INFO, TAG, "OUT CreateSecureSessionJustWorksCallback");
    return OC_STACK_OK;
}

OCStackResult CreateMVJustWorksSelectOxmPayload(OTMContext_t *otmCtx, uint8_t **cborPayload,
                                             size_t *cborSize)
{
    if (!otmCtx || !otmCtx->selectedDeviceInfo || !cborPayload || *cborPayload || !cborSize)
    {
        return OC_STACK_INVALID_PARAM;
    }

    otmCtx->selectedDeviceInfo->doxm->oxmSel = OIC_MV_JUST_WORKS;
    *cborPayload = NULL;
    *cborSize = 0;

    return DoxmToCBORPayload(otmCtx->selectedDeviceInfo->doxm, cborPayload, cborSize, true);
}

