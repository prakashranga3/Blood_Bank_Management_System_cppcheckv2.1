/**
 * @file config.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Compile-time configuration for BMS Version 2.
 */

#ifndef BMS_CONFIG_H
#define BMS_CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BMS_VERSION_MAJOR                 (2U)
#define BMS_VERSION_MINOR                 (0U)
#define BMS_VERSION_PATCH                 (0U)

#define BMS_MAX_NAME_LENGTH               (64U)
#define BMS_MAX_USERNAME_LENGTH           (32U)
#define BMS_MAX_PASSWORD_LENGTH           (64U)
#define BMS_MIN_PASSWORD_LENGTH           (8U)
#define BMS_MAX_PHONE_LENGTH              (16U)
#define BMS_MAX_EMAIL_LENGTH              (96U)
#define BMS_MAX_ADDRESS_LENGTH            (160U)
#define BMS_MAX_LOCATION_LENGTH           (80U)
#define BMS_MAX_DATE_LENGTH               (11U)
#define BMS_MAX_MESSAGE_LENGTH            (256U)
#define BMS_MAX_BLOOD_GROUP_LENGTH        (4U)
#define BMS_MAX_REPORT_PATH_LENGTH        (260U)

#define BMS_MAX_LOGIN_ATTEMPTS            (3U)
#define BMS_HASH_BUCKET_COUNT             (101U)
#define BMS_NOTIFICATION_QUEUE_CAPACITY   (128U)
#define BMS_ALERT_QUEUE_CAPACITY          (64U)
#define BMS_REQUEST_QUEUE_CAPACITY        (128U)
#define BMS_GRAPH_MAX_HOSPITALS           (128U)

#define BMS_LOW_STOCK_THRESHOLD_UNITS     (5U)
#define BMS_RBC_SHELF_LIFE_DAYS           (45U)
#define BMS_MAX_BLOOD_UNITS               (100000U)
#define BMS_MIN_DONOR_AGE                 (18U)
#define BMS_MAX_DONOR_AGE                 (65U)
#define BMS_MIN_DONOR_WEIGHT_KG           (50U)

#define BMS_FILE_MAGIC                    (0x424D5332UL)
#define BMS_FILE_FORMAT_VERSION           (1U)

#ifdef __cplusplus
}
#endif

#endif /* BMS_CONFIG_H */
