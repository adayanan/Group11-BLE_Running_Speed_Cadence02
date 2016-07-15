/*******************************************************************************
* File Name: rscs.c
*
* Version: 1.0
*
* Description:
*  This file contains routines related to Running Speed and Cadence Service.
*
* Hardware Dependency:
*  CY8CKIT-042 BLE
* 
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"
#include "rscs.h"


/***************************************
*        Global Variables
***************************************/
uint8                   profile = WALKING;
uint16                  currSpeed = 0u;
uint8                   rscNotificationState = DISABLED;
uint8                   rscIndicationState = DISABLED;
uint8                   rscIndicationPending = NO;
uint8                   rcsOpCode = RSC_SC_CP_INVALID_OP_CODE;
uint8                   rcsRespValue = RSC_SC_CP_INVALID_OP_CODE;

/* Sensor locations supported by the device */
uint8                   rscSensors[RSC_SENSORS_NUMBER];

/* This variable contains profile simulation data */
RSC_RSC_MEASUREMENT_T   rscMeasurement;

/* This variable contains profile simulation data */
uint16                  rscFeature;


/*******************************************************************************
* Function Name: RscServiceAppEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component,
*  which are specific to Running Speed and Cadence Service.
*
* Parameters:  
*  uint8 event:       Running Speed and Cadence Service event.
*  void* eventParams: Data structure specific to event received.
*
* Return: 
*  None
*
*******************************************************************************/
void RscServiceAppEventHandler(uint32 event, void *eventParam)
{
    uint8 i;
    CYBLE_RSCS_CHAR_VALUE_T *wrReqParam;
    
    switch(event)
    {
    /***************************************
    *        RSCS Server events
    ***************************************/
    case CYBLE_EVT_RSCSS_NOTIFICATION_ENABLED:
        printf("Notifications for RSC Measurement Characteristic are enabled\r\n");
        rscNotificationState = ENABLED;
        break;
        
    case CYBLE_EVT_RSCSS_NOTIFICATION_DISABLED:
        printf("Notifications for RSC Measurement Characteristic are disabled\r\n");
        rscNotificationState = DISABLED;
		break;

    case CYBLE_EVT_RSCSS_INDICATION_ENABLED:
        printf("Indications for SC Control point Characteristic are enabled\r\n");
        rscIndicationState = ENABLED;
		break;
        
    case CYBLE_EVT_RSCSS_INDICATION_DISABLED:
        printf("Indications for SC Control point Characteristic are disabled\r\n");
        rscIndicationState = DISABLED;
		break;

    case CYBLE_EVT_RSCSS_INDICATION_CONFIRMATION:
        printf("Indication Confirmation for SC Control point was received\r\n");
		break;
	
    case CYBLE_EVT_RSCSS_CHAR_WRITE:
        
        wrReqParam = (CYBLE_RSCS_CHAR_VALUE_T *) eventParam;
        printf("Write to SC Control Point Characteristic occurred\r\n");
        printf("Data length: %d\r\n", wrReqParam->value->len);
        printf("Received data:");

        for(i = 0u; i < wrReqParam->value->len; i++)
        {
             printf(" %x", wrReqParam->value->val[i]);
        }
         printf("\r\n");
        
        rcsOpCode = wrReqParam->value->val[RSC_SC_CP_OP_CODE_IDX];

        switch(rcsOpCode)
        {
        case CYBLE_RSCS_SET_CUMMULATIVE_VALUE:
            /* Validate command length */
            if(wrReqParam->value->len == RSC_SET_CUMMULATIVE_VALUE_LEN)
            {
                if(0u != (rscFeature && RSC_FEATURE_INST_STRIDE_PRESENT))
                {
                    rscMeasurement.totalDistance = (wrReqParam->value->val[RSC_SC_CUM_VAL_BYTE3_IDX] << THREE_BYTES_SHIFT) |
                                    (wrReqParam->value->val[RSC_SC_CUM_VAL_BYTE2_IDX] << TWO_BYTES_SHIFT) |
                                    (wrReqParam->value->val[RSC_SC_CUM_VAL_BYTE1_IDX] << ONE_BYTE_SHIFT) |
                                    wrReqParam->value->val[RSC_SC_CUM_VAL_BYTE0_IDX];

                    printf("Set cumulative value command was received.\r\n");
                    rscMeasurement.totalDistance *= RSCS_CM_TO_DM_VALUE;
                    rcsRespValue = CYBLE_RSCS_ERR_SUCCESS;
                }
                else
                {
                    printf("The procedure is not supported.\r\n");
                    rcsRespValue = CYBLE_RSCS_ERR_OP_CODE_NOT_SUPPORTED;
                }
            }
            else
            {
                rcsRespValue = CYBLE_RSCS_ERR_OPERATION_FAILED;
            }
            break;

        case CYBLE_RSCS_START_SENSOR_CALIBRATION:
            printf("Start Sensor calibration command was received.\r\n");
            rcsRespValue = CYBLE_RSCS_ERR_OP_CODE_NOT_SUPPORTED;
            rcsOpCode = CYBLE_RSCS_START_SENSOR_CALIBRATION;
            break;

        case CYBLE_RSCS_UPDATE_SENSOR_LOCATION:
            /* Validate command length */
            if(wrReqParam->value->len == RSC_UPDATE_SENSOR_LOCATION_LEN)
            {
                if(0u != (rscFeature & RSC_FEATURE_MULTIPLE_SENSOR_LOC_PRESENT))
                {
                    printf("Update sensor location command was received.\r\n");
                    
                    /* Check if the requested sensor location is supported */
                    if(YES == IsSensorLocationSupported(wrReqParam->value->val[RSC_SC_SENSOR_LOC_IDX]))
                    {
                        printf("New Sensor location was set.\r\n");
                        /* Set requested sensor location */
                        CyBle_RscssSetCharacteristicValue(CYBLE_RSCS_SENSOR_LOCATION, 1u, 
                            &wrReqParam->value->val[RSC_SC_SENSOR_LOC_IDX]);
                    }
                    else
                    {
                        printf("The requested sensor location is not supported.\r\n");
                        /* Invalid sensor location is received */
                        rcsRespValue = CYBLE_RSCS_ERR_INVALID_PARAMETER;
                    }
                }
                else
                {
                    printf("The procedure is not supported.\r\n");
                    rcsRespValue = CYBLE_RSCS_ERR_OP_CODE_NOT_SUPPORTED;
                }
            }
            else
            {
                rcsRespValue = CYBLE_RSCS_ERR_OPERATION_FAILED;
            }
            break;

        case CYBLE_RSCS_REQ_SUPPORTED_SENSOR_LOCATION:
            /* Validate command length */
            if(wrReqParam->value->len == RSC_REQ_SUPPORTED_SENSOR_LOCATION_LEN)
            {
                if(0u != (rscFeature & RSC_FEATURE_MULTIPLE_SENSOR_LOC_PRESENT))
                {
                    printf("Supported sensor location command was received.\r\n");
                    rcsRespValue = CYBLE_RSCS_ERR_SUCCESS;
                }
                else
                {
                    printf("The procedure is not supported.\r\n");
                    rcsRespValue = CYBLE_RSCS_ERR_OP_CODE_NOT_SUPPORTED;
                }
            }
            else
            {
                rcsRespValue = CYBLE_RSCS_ERR_OPERATION_FAILED;
            }
            break;

        default:
            printf("Unsupported command.\r\n");
            break;
        }

        /* Set the flag to sent indication from main() */
        rscIndicationPending = YES;
		break;

    /***************************************
    *        RSCS Client events
    ***************************************/
    case CYBLE_EVT_RSCSC_NOTIFICATION:
        break;
    case CYBLE_EVT_RSCSC_INDICATION:
        break;
    case CYBLE_EVT_RSCSC_READ_CHAR_RESPONSE:
        break;
    case CYBLE_EVT_RSCSC_WRITE_CHAR_RESPONSE:
        break;
    case CYBLE_EVT_RSCSC_READ_DESCR_RESPONSE:
        break;
    case CYBLE_EVT_RSCSC_WRITE_DESCR_RESPONSE:
        break;

	default:
        printf("Unrecognised RSCS event.\r\n");
	    break;
    }
}


/*******************************************************************************
* Function Name: InitProfile
********************************************************************************
*
* Summary:
*  Performs profile's initialization.
*
* Parameters:  
*  None.
*
* Return: 
*  None
*
*******************************************************************************/
void InitProfile(void)
{
    uint8 buff[RSC_RSC_MEASUREMENT_CHAR_SIZE];
    
    if(CyBle_RscssGetCharacteristicValue(CYBLE_RSCS_RSC_MEASUREMENT, RSC_RSC_MEASUREMENT_CHAR_SIZE, buff) !=
            CYBLE_ERROR_OK)
    {
        printf("Failed to read the RSC Measurement value.\r\n");
    }

    /* Set initial RSC Characteristic flags as per values set in the customizer */
    rscMeasurement.flags = buff[RSC_CHAR_FLAGS_OFFSET];

    /* Get the RSC Feature */
    GetRscFeatureChar(&rscFeature);

    rscMeasurement.instCadence = WALKING_INST_CADENCE_MIN;
    rscMeasurement.instStridelen = WALKING_INST_STRIDE_LENGTH_MIN;
    rscMeasurement.totalDistance = 0u;

    /* Set supported sensor locations */
    rscSensors[RSC_SENSOR1_IDX] = RSC_SENSOR_LOC_IN_SHOE;
    rscSensors[RSC_SENSOR2_IDX] = RSC_SENSOR_LOC_HIP;
}


/*******************************************************************************
* Function Name: SimulateProfile
********************************************************************************
*
* Summary:
*  Simulates the Running Speed and Cadence profile. When this function is called, 
*  it is assumed that the a complete stride has occurred and it is the time to
*  update the speed and the total distance values.
*
* Parameters:  
*  None.
*
* Return: 
*  None
*
*******************************************************************************/
void SimulateProfile(void)
{
    /* Update total distance */
    rscMeasurement.totalDistance += rscMeasurement.instStridelen;

    /* Calculate speed in m/s with resolution of 1/256 of second */
    currSpeed = (((uint16)(2 * rscMeasurement.instCadence * rscMeasurement.instStridelen)) << 8u) /
                    (RSCS_MIN_TO_SEC_VALUE * RSCS_CM_TO_METER_VALUE);
}


/*******************************************************************************
* Function Name: IsSensorLocationSupported
********************************************************************************
*
* Summary:
*  Verifies if the requested sensor location is supported by the device.
*
* Parameters:  
*  sensorLocation: Sensor location.
*
* Return: 
*  Status:
*   YES - the sensor is supported;
*   NO - sensor is not supported.
*
*******************************************************************************/
uint8 IsSensorLocationSupported(uint8 sensorLocation)
{
    uint8 i;
    uint8 result = NO;
    
    for(i = 0u; i < RSC_SENSORS_NUMBER; i++)
    {
        if(rscSensors[i] == sensorLocation)
        {
            result = YES;
        }
    }
    return(result);
}


/*******************************************************************************
* Function Name: HandleRscNotifications
********************************************************************************
*
* Summary:
*  Handles Notifications to the Client device.
*
* Parameters:  
*  None.
*
* Return: 
*  None
*
*******************************************************************************/
void HandleRscNotifications(void)
{
    uint8 rcsValue[RSC_RSC_MEASUREMENT_CHAR_SIZE];
    CYBLE_API_RESULT_T apiResult;
    uint32 totalDistanceDm;

    /* Convert total distance to decimeters per BLE RSCS spec */
    totalDistanceDm = rscMeasurement.totalDistance / RSCS_CM_TO_DM_VALUE;

    /* Update the characteristic */
    rcsValue[RSC_CHAR_FLAGS_OFFSET]                    = rscMeasurement.flags;
    rcsValue[RSC_CHAR_INST_SPEED_OFFSET]               = LO8(currSpeed);
    rcsValue[RSC_CHAR_INST_SPEED_OFFSET + 1u]          = HI8(currSpeed);
    rcsValue[RSC_CHAR_INST_CADENCE_OFFSET]             = LO8(rscMeasurement.instCadence);
    rcsValue[RSC_CHAR_INST_STRIDE_LEN_OFFSET]          = LO8(rscMeasurement.instStridelen);
    rcsValue[RSC_CHAR_INST_STRIDE_LEN_OFFSET + 1u]     = HI8(rscMeasurement.instStridelen);
    rcsValue[RSC_CHAR_TOTAL_DISTANCE_OFFSET]           = LO8(LO16(totalDistanceDm));
    rcsValue[RSC_CHAR_TOTAL_DISTANCE_OFFSET + 1u]      = HI8(LO16(totalDistanceDm));
    rcsValue[RSC_CHAR_TOTAL_DISTANCE_OFFSET + 2u]      = LO8(HI16(totalDistanceDm));
    rcsValue[RSC_CHAR_TOTAL_DISTANCE_OFFSET + 3u]      = HI8(HI16(totalDistanceDm));

    /* Send notification to the peer Client */
    apiResult = CyBle_RscssSendNotification(connectionHandle, CYBLE_RSCS_RSC_MEASUREMENT, RSC_RSC_MEASUREMENT_CHAR_SIZE, rcsValue);

    /* Update the debug info if notification is sent */
    if(CYBLE_ERROR_OK == apiResult)
    {
        printf("Notification is sent! \r\n");
        printf("Cadence: %d, ", rscMeasurement.instCadence);
        printf("Speed: %d, ", currSpeed);
        printf("Stride length: %d, ", rscMeasurement.instStridelen);
        printf("Total distance: %d, ", LO16(totalDistanceDm));

        if(WALKING == profile)
        {
            printf("Status: Walking \r\n");
        }
        else
        {
            printf("Status: Running \r\n");
        }
    }
    else
    {
        if(CYBLE_ERROR_INVALID_PARAMETER == apiResult)
        {
            printf("CyBle_RscssSendNotification() resulted with CYBLE_ERROR_INVALID_PARAMETER\r\n");
        }
        else if(CYBLE_ERROR_NTF_DISABLED == apiResult)
        {
            printf("CyBle_RscssSendNotification() resulted with CYBLE_ERROR_NTF_DISABLED\r\n");
        }
        else
        {
            printf("CyBle_RscssSendNotification() resulted with CYBLE_ERROR_INVALID_STATE\r\n");
        }
    }
}


/*******************************************************************************
* Function Name: HandleRscIndications
********************************************************************************
*
* Summary:
*  Handles SC Control Point indications to the Client device. With this 
*  indication the Client receives a response for the previously send SC Control
*  Point Procedure.
*  
* Parameters:  
*  None.
*
* Return: 
*  None
*
*******************************************************************************/
void HandleRscIndications(void)
{
    uint8 i;
    uint8 size = RSC_SC_CP_SIZE;
    CYBLE_API_RESULT_T apiResult;
    uint8 buff[RSC_SC_CP_SIZE + RSC_SENSORS_NUMBER];

    /* Handle the received SC Control Point Op Code */
    switch(rcsOpCode)
    {
    case CYBLE_RSCS_REQ_SUPPORTED_SENSOR_LOCATION:
        buff[RSC_SC_CP_RESP_VAL_IDX] = rcsRespValue;
        if(rcsRespValue == CYBLE_RSCS_ERR_SUCCESS)
        {
            for(i = 0u; i < RSC_SENSORS_NUMBER; i++)
            {
                buff[RSC_SC_CP_RESP_PARAM_IDX + i] = rscSensors[i];
            }

            /* Update the size with number of supported sensors */
            size += RSC_SENSORS_NUMBER;
        }
    case CYBLE_RSCS_START_SENSOR_CALIBRATION:
    case CYBLE_RSCS_SET_CUMMULATIVE_VALUE:
    case CYBLE_RSCS_UPDATE_SENSOR_LOCATION:
        buff[RSC_SC_CP_RESP_VAL_IDX] = rcsRespValue;
        break;

    default:
        buff[RSC_SC_CP_RESP_VAL_IDX] = CYBLE_RSCS_ERR_OP_CODE_NOT_SUPPORTED;
        break;
    }

    buff[RSC_SC_CP_RESP_OP_CODE_IDX] = CYBLE_RSCS_RESPONSE_CODE;
    buff[RSC_SC_CP_REQ_OP_CODE_IDX] = rcsOpCode;

    apiResult = CyBle_RscssSendIndication(connectionHandle, CYBLE_RSCS_SC_CONTROL_POINT, size, buff);
    
    if(CYBLE_ERROR_OK == apiResult)
    {
        printf("CyBle_RscssSendIndication() succeeded\r\n");
    }
    else
    {
        printf("CyBle_RscssSendIndication() resulted with an error. Error code: %x\r\n", apiResult);
    }
        
    /* Clear the Op Code and the indication pending flag */
    rcsOpCode = RSC_SC_CP_INVALID_OP_CODE;
    rscIndicationPending = NO;
}


/*******************************************************************************
* Function Name: UpdatePace
********************************************************************************
*
* Summary:
*  Simulates "RUNNING" or "WALKING" profile.
*
* Parameters:  
*  None.
*
* Return: 
*  None
*
*******************************************************************************/
void UpdatePace(void)
{
    if(WALKING == profile)
    {
        /* Update stride length */
        if(rscMeasurement.instStridelen <= WALKING_INST_STRIDE_LENGTH_MAX)
        {
            rscMeasurement.instStridelen++;
        }
        else
        {
            rscMeasurement.instStridelen = WALKING_INST_STRIDE_LENGTH_MIN;
        }
        
        /* .. and cadence */
        if(rscMeasurement.instCadence <= WALKING_INST_CADENCE_MAX)
        {
            rscMeasurement.instCadence++;
        }
        else
        {
            rscMeasurement.instCadence = WALKING_INST_CADENCE_MIN;
        }
    }
    else
    {
        /* Update stride length */
        if(rscMeasurement.instStridelen <= RUNNING_INST_STRIDE_LENGTH_MAX)
        {
            rscMeasurement.instStridelen++;
        }
        else
        {
            rscMeasurement.instStridelen = RUNNING_INST_STRIDE_LENGTH_MIN;
        }
        
        /* .. and cadence */
        if(rscMeasurement.instCadence <= RUNNING_INST_CADENCE_MAX)
        {
            rscMeasurement.instCadence++;
        }
        else
        {
            rscMeasurement.instCadence = RUNNING_INST_CADENCE_MIN;
        }
    }
}


/*******************************************************************************
* Function Name: GetRscFeatureChar
********************************************************************************
*
* Summary:
*  Reads RSC Feature Characteristic value from the database and pack it to 
*  RSC_RSC_MEASUREMENT_T structure.
*
* Parameters:
*  rsc: Placeholder for RSC Measurement Characteristic value.
*
* Return: 
*  None
*
*******************************************************************************/
void GetRscFeatureChar(uint16 * feature)
{
    uint8 buff[RSC_RSC_FEATURE_SIZE];

    /* Get the characteristic into the buffer */
    if((CyBle_RscssGetCharacteristicValue(CYBLE_RSCS_RSC_FEATURE, RSC_RSC_FEATURE_SIZE, buff)) == CYBLE_ERROR_OK)
    {
        /* Fill in the RSC_RSC_MEASUREMENT_T structure */
        *feature = (uint16) ((((uint16) buff[1u]) << ONE_BYTE_SHIFT) | ((uint16) buff[0u]));
    }
    else
    {
        printf("Failed to read the RSC Featute value.\r\n");
    }
}


/* [] END OF FILE */
