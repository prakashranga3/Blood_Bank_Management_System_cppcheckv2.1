/**
 * @file common_validation.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Reusable validation APIs for BMS business modules.
 */

#ifndef BMS_COMMON_VALIDATION_H
#define BMS_COMMON_VALIDATION_H

#include <stdbool.h>
#include <stdint.h>

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

bool ValidatePositiveNumber(uint32_t value);
bool ValidateNonZero(uint32_t value);

bool ValidateName(const char *name);
bool ValidateUsername(const char *username);
bool ValidatePassword(const char *password);
bool ValidatePhoneNumber(const char *phone);
bool ValidateEmail(const char *email);
bool ValidateAddress(const char *address);
bool ValidateLocation(const char *location);
bool ValidateContactNumber(const char *contactNumber);

bool ValidateUserId(uint32_t userId);
bool ValidateBloodId(uint32_t bloodId);
bool ValidateDonorId(uint32_t donorId);
bool ValidateHospitalId(uint32_t hospitalId);
bool ValidateRequestId(uint32_t requestId);
bool ValidateRequesterId(uint32_t requesterId);
bool ValidateDonationId(uint32_t donationId);

bool ValidateBloodGroup(const char *bloodGroup);
bool ValidateBloodGroupValue(BmsBloodGroup_t bloodGroup);
BmsStatus_t ParseBloodGroup(const char *text, BmsBloodGroup_t *bloodGroup);
const char *BloodGroupToString(BmsBloodGroup_t bloodGroup);
bool ValidateUnits(uint32_t units);
bool ValidateAge(uint32_t age);
bool ValidateWeight(uint32_t weightKg);

bool ValidateDate(const char *date);
bool ValidateDateValue(const BmsDate_t *date);
BmsStatus_t BmsDateAddDays(const BmsDate_t *source,
                           uint32_t daysToAdd,
                           BmsDate_t *result);

bool ValidateUserRole(uint8_t role);
bool ValidateUserRoleValue(BmsUserRole_t role);

#ifdef __cplusplus
}
#endif

#endif /* BMS_COMMON_VALIDATION_H */
