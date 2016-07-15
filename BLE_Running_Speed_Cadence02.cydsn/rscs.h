/*******************************************************************************
* File Name: rscs.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants used by the example project.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


/***************************************
##Data Struct Definition
***************************************/

/* RSC measurement */
typedef struct
{
    uint8 flags;
    uint16 instSpeed;
    uint8 instCadence;
    uint16 instStridelen;
    uint32 totalDistance;
} RSC_RSC_MEASUREMENT_T;


/***************************************
*          Constants
***************************************/

/* Walking or Running profile */
#define WALKING                                 (0u)
#define RUNNING                                 (1u)

/* Walking profile constants */
#define WALKING_INST_CADENCE_MIN                (80u)
#define WALKING_INST_CADENCE_MAX                (95u)

/* Stride length is shown in centimetres */
#define WALKING_INST_STRIDE_LENGTH_MIN          (65u)
#define WALKING_INST_STRIDE_LENGTH_MAX          (85u)

/* Running profile constants */
#define RUNNING_INST_CADENCE_MIN                (120u)
#define RUNNING_INST_CADENCE_MAX                (155u)

/* Stride length is shown in centimetres */
#define RUNNING_INST_STRIDE_LENGTH_MIN          (100u)
#define RUNNING_INST_STRIDE_LENGTH_MAX          (115u)

/* RSC Characteristic constants */
#define RSC_CHAR_FLAGS_OFFSET                   (0u)
#define RSC_CHAR_INST_SPEED_OFFSET              (1u)
#define RSC_CHAR_INST_CADENCE_OFFSET            (3u)
#define RSC_CHAR_INST_STRIDE_LEN_OFFSET         (4u)
#define RSC_CHAR_TOTAL_DISTANCE_OFFSET          (6u)

#define RSC_FEATURE_INST_STRIDE_PRESENT         (0x01u)
#define RSC_FEATURE_TOTAL_DISTANCE_PRESENT      (0x02u)
#define RSC_FEATURE_WALK_RUN_STATUS_MASK        (0x04u)
#define RSC_FEATURE_MULTIPLE_SENSOR_LOC_PRESENT (0x10u)

#define RSC_SENSOR_LOC_OTHER                    (0u)
#define RSC_SENSOR_LOC_TOP_OF_SHOE              (1u)
#define RSC_SENSOR_LOC_IN_SHOE                  (2u)
#define RSC_SENSOR_LOC_HIP                      (3u)
#define RSC_SENSOR_LOC_FRONT_WHEEL              (4u)
#define RSC_SENSOR_LOC_LEFT_CRANK               (5u)
#define RSC_SENSOR_LOC_RIGHT_CRANK              (6u)
#define RSC_SENSOR_LOC_LEFT_PEDAL               (7u)
#define RSC_SENSOR_LOC_RIGHT_PEDAL              (8u)
#define RSC_SENSOR_LOC_FRONT_HUB                (9u)
#define RSC_SENSOR_LOC_REAR_DROPOUT             (10u)
#define RSC_SENSOR_LOC_CHAINSTRAY               (11u)
#define RSC_SENSOR_LOC_REAR_WHEEL               (12u)
#define RSC_SENSOR_LOC_REAR_HUB                 (13u)
#define RSC_SENSOR_LOC_CHEST                    (14u)

/* RSCS Characteristic sizes */
#define RSC_RSC_MEASUREMENT_CHAR_SIZE           (10u)
#define RSC_RSC_FEATURE_SIZE                    (2u)
#define RSC_SC_CP_SIZE                          (3u)

/* Conversion constants */
#define RSCS_CM_TO_METER_VALUE                  (100u)
#define RSCS_CM_TO_DM_VALUE                     (10u)
#define RSCS_MIN_TO_SEC_VALUE                   (60u)

/* SC Control Point Characteristic indexes */
#define RSC_SC_CP_OP_CODE_IDX                   (0u)
#define RSC_SC_CUM_VAL_BYTE0_IDX                (1u)
#define RSC_SC_CUM_VAL_BYTE1_IDX                (2u)
#define RSC_SC_CUM_VAL_BYTE2_IDX                (3u)
#define RSC_SC_CUM_VAL_BYTE3_IDX                (4u)
#define RSC_SC_SENSOR_LOC_IDX                   (1u)
#define RSC_SC_CP_RESP_OP_CODE_IDX              (0u)
#define RSC_SC_CP_REQ_OP_CODE_IDX               (1u)
#define RSC_SC_CP_RESP_VAL_IDX                  (2u)
#define RSC_SC_CP_RESP_PARAM_IDX                (3u)

#define RSC_SC_CP_INVALID_OP_CODE               (0xFFu)

#define RSC_SENSORS_NUMBER                      (0x2u)
#define RSC_SENSOR1_IDX                         (0x0u)
#define RSC_SENSOR2_IDX                         (0x1u)

#define RSC_SET_CUMMULATIVE_VALUE_LEN           (5u)
#define RSC_START_SENSOR_CALIBRATION_LEN        (1u)
#define RSC_UPDATE_SENSOR_LOCATION_LEN          (2u)
#define RSC_REQ_SUPPORTED_SENSOR_LOCATION_LEN   (1u)


/***************************************
*        Function Prototypes
***************************************/
void InitProfile(void);
void UpdatePace(void);
void SimulateProfile(void);
void HandleRscNotifications(void);
void HandleRscIndications(void);
void GetRscFeatureChar(uint16 * feature);
uint8 IsSensorLocationSupported(uint8 sensorLocation);
void RscServiceAppEventHandler(uint32 event, void * eventParam);


/***************************************
* External data references
***************************************/
extern CYBLE_CONN_HANDLE_T      connectionHandle;
extern uint8                    profile;
extern uint8                    rscNotificationState;
extern uint8                    rscIndicationState;
extern RSC_RSC_MEASUREMENT_T    rscMeasurement;
extern uint16                   rscFeature;
extern uint8                    rscIndicationPending;
extern uint8                    rcsOpCode;
extern uint8                    rcsRespValue;
extern uint8                    rscSensors[RSC_SENSORS_NUMBER];


/* [] END OF FILE */
