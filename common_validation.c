/** @file common_validation.c @brief Common validation implementation. */
#include "common_validation.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "utility.h"

static bool ValidateBoundedText(const char *text, size_t minimum, size_t maximum)
{
    size_t length;
    if (text == NULL) { return false; }
    length = strlen(text);
    return (length >= minimum) && (length <= maximum);
}

bool ValidatePositiveNumber(uint32_t value) { return value > 0U; }
bool ValidateNonZero(uint32_t value) { return value != 0U; }

bool ValidateName(const char *name)
{
    size_t i;
    if (!ValidateBoundedText(name, 2U, BMS_MAX_NAME_LENGTH)) { return false; }
    for (i = 0U; name[i] != '\0'; ++i)
    {
        if ((isalpha((unsigned char)name[i]) == 0) &&
            (name[i] != ' ') && (name[i] != '.') && (name[i] != '-'))
        { return false; }
    }
    return true;
}

bool ValidateUsername(const char *username)
{
    size_t i;
    if (!ValidateBoundedText(username, 4U, BMS_MAX_USERNAME_LENGTH)) { return false; }
    for (i = 0U; username[i] != '\0'; ++i)
    {
        if ((isalnum((unsigned char)username[i]) == 0) &&
            (username[i] != '_') && (username[i] != '.'))
        { return false; }
    }
    return true;
}

bool ValidatePassword(const char *password)
{
    bool upper = false, lower = false, digit = false, special = false;
    size_t i;
    if (!ValidateBoundedText(password, BMS_MIN_PASSWORD_LENGTH, BMS_MAX_PASSWORD_LENGTH))
    { return false; }
    for (i = 0U; password[i] != '\0'; ++i)
    {
        unsigned char c = (unsigned char)password[i];
        upper = upper || (isupper(c) != 0);
        lower = lower || (islower(c) != 0);
        digit = digit || (isdigit(c) != 0);
        special = special || (isalnum(c) == 0);
    }
    return upper && lower && digit && special;
}

bool ValidatePhoneNumber(const char *phone)
{
    size_t i;
    if ((phone == NULL) || (strlen(phone) != 10U)) { return false; }
    for (i = 0U; i < 10U; ++i) { if (isdigit((unsigned char)phone[i]) == 0) { return false; } }
    return true;
}

bool ValidateEmail(const char *email)
{
    const char *at; const char *dot; const char *p;
    if (!ValidateBoundedText(email, 6U, BMS_MAX_EMAIL_LENGTH)) { return false; }
    if (strchr(email, ' ') != NULL) { return false; }
    at = strchr(email, '@');
    if ((at == NULL) || (at == email) || (strchr(at + 1, '@') != NULL)) { return false; }
    dot = strrchr(at + 1, '.');
    if ((dot == NULL) || (dot <= at + 1) || (dot[1] == '\0')) { return false; }
    for (p=email; p<at; ++p) { if (!(isalnum((unsigned char)*p) || *p=='.' || *p=='_' || *p=='-' || *p=='+')) return false; }
    for (p=at+1; *p; ++p) { if (!(isalnum((unsigned char)*p) || *p=='.' || *p=='-')) return false; }
    return true;
}

bool ValidateAddress(const char *address)
{ return ValidateBoundedText(address, 5U, BMS_MAX_ADDRESS_LENGTH); }
bool ValidateLocation(const char *location)
{ return ValidateBoundedText(location, 2U, BMS_MAX_LOCATION_LENGTH); }
bool ValidateContactNumber(const char *contactNumber)
{ return ValidatePhoneNumber(contactNumber); }

bool ValidateUserId(uint32_t value) { return value != 0U; }
bool ValidateBloodId(uint32_t value) { return value != 0U; }
bool ValidateDonorId(uint32_t value) { return value != 0U; }
bool ValidateHospitalId(uint32_t value) { return value != 0U; }
bool ValidateRequestId(uint32_t value) { return value != 0U; }
bool ValidateRequesterId(uint32_t value) { return value != 0U; }
bool ValidateDonationId(uint32_t value) { return value != 0U; }

static bool BloodGroupTextEquals(const char *left, const char *right)
{
    bool equal = false;

    if ((left != NULL) && (right != NULL))
    {
        equal = true;
        while ((*left != '\0') && (*right != '\0'))
        {
            if (toupper((unsigned char)*left) != toupper((unsigned char)*right))
            {
                equal = false;
                break;
            }
            ++left;
            ++right;
        }

        if ((*left != '\0') || (*right != '\0'))
        {
            equal = false;
        }
    }

    return equal;
}

static BmsStatus_t NormalizeBloodGroupText(
    const char *text,
    char *normalized,
    size_t normalizedSize)
{
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;

    if ((text != NULL) && (normalized != NULL) && (normalizedSize > 0U))
    {
        size_t sourceIndex = 0U;
        size_t destinationIndex = 0U;
        status = BMS_STATUS_OK;

        while ((text[sourceIndex] != '\0') &&
               (destinationIndex < (normalizedSize - 1U)))
        {
            const unsigned char current = (unsigned char)text[sourceIndex];

            if ((isspace(current) == 0) &&
                (current != (unsigned char)'_') &&
                (current != (unsigned char)'-'))
            {
                normalized[destinationIndex] =
                    (char)toupper(current);
                ++destinationIndex;
            }
            else if (current == (unsigned char)'-')
            {
                normalized[destinationIndex] = '-';
                ++destinationIndex;
            }
            else
            {
                /* Ignore spaces and underscores. */
            }

            ++sourceIndex;
        }

        normalized[destinationIndex] = '\0';

        if (text[sourceIndex] != '\0')
        {
            status = BMS_STATUS_INVALID_DATA;
        }
    }

    return status;
}
//text->enum
BmsStatus_t ParseBloodGroup(const char *text, BmsBloodGroup_t *bloodGroup)
{
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;

    if ((text != NULL) && (bloodGroup != NULL))
    {
        char normalized[24U] = { '\0' };
        *bloodGroup = BMS_BLOOD_GROUP_INVALID;
        status = NormalizeBloodGroupText(text, normalized, sizeof(normalized));

        if (status == BMS_STATUS_OK)
        {
            if ((BloodGroupTextEquals(normalized, "A+") == true) ||
                (BloodGroupTextEquals(normalized, "APOSITIVE") == true) ||
                (BloodGroupTextEquals(normalized, "ARH+") == true) ||
                (BloodGroupTextEquals(normalized, "ARHD+") == true))
            {
                *bloodGroup = BMS_BLOOD_GROUP_A_POSITIVE;
            }
            else if ((BloodGroupTextEquals(normalized, "A-") == true) ||
                     (BloodGroupTextEquals(normalized, "ANEGATIVE") == true) ||
                     (BloodGroupTextEquals(normalized, "ARH-") == true) ||
                     (BloodGroupTextEquals(normalized, "ARHD-") == true))
            {
                *bloodGroup = BMS_BLOOD_GROUP_A_NEGATIVE;
            }
            else if ((BloodGroupTextEquals(normalized, "B+") == true) ||
                     (BloodGroupTextEquals(normalized, "BPOSITIVE") == true) ||
                     (BloodGroupTextEquals(normalized, "BRH+") == true) ||
                     (BloodGroupTextEquals(normalized, "BRHD+") == true))
            {
                *bloodGroup = BMS_BLOOD_GROUP_B_POSITIVE;
            }
            else if ((BloodGroupTextEquals(normalized, "B-") == true) ||
                     (BloodGroupTextEquals(normalized, "BNEGATIVE") == true) ||
                     (BloodGroupTextEquals(normalized, "BRH-") == true) ||
                     (BloodGroupTextEquals(normalized, "BRHD-") == true))
            {
                *bloodGroup = BMS_BLOOD_GROUP_B_NEGATIVE;
            }
            else if ((BloodGroupTextEquals(normalized, "AB+") == true) ||
                     (BloodGroupTextEquals(normalized, "ABPOSITIVE") == true) ||
                     (BloodGroupTextEquals(normalized, "ABRH+") == true) ||
                     (BloodGroupTextEquals(normalized, "ABRHD+") == true))
            {
                *bloodGroup = BMS_BLOOD_GROUP_AB_POSITIVE;
            }
            else if ((BloodGroupTextEquals(normalized, "AB-") == true) ||
                     (BloodGroupTextEquals(normalized, "ABNEGATIVE") == true) ||
                     (BloodGroupTextEquals(normalized, "ABRH-") == true) ||
                     (BloodGroupTextEquals(normalized, "ABRHD-") == true))
            {
                *bloodGroup = BMS_BLOOD_GROUP_AB_NEGATIVE;
            }
            else if ((BloodGroupTextEquals(normalized, "O+") == true) ||
                     (BloodGroupTextEquals(normalized, "OPOSITIVE") == true) ||
                     (BloodGroupTextEquals(normalized, "ORH+") == true) ||
                     (BloodGroupTextEquals(normalized, "ORHD+") == true))
            {
                *bloodGroup = BMS_BLOOD_GROUP_O_POSITIVE;
            }
            else if ((BloodGroupTextEquals(normalized, "O-") == true) ||
                     (BloodGroupTextEquals(normalized, "ONEGATIVE") == true) ||
                     (BloodGroupTextEquals(normalized, "ORH-") == true) ||
                     (BloodGroupTextEquals(normalized, "ORHD-") == true))
            {
                *bloodGroup = BMS_BLOOD_GROUP_O_NEGATIVE;
            }
            else if ((BloodGroupTextEquals(normalized, "A1+") == true) ||
                     (BloodGroupTextEquals(normalized, "A1POSITIVE") == true) ||
                     (BloodGroupTextEquals(normalized, "A1RH+") == true) ||
                     (BloodGroupTextEquals(normalized, "A1RHD+") == true))
            {
                *bloodGroup = BMS_BLOOD_GROUP_A1_POSITIVE;
            }
            else
            {
                status = BMS_STATUS_INVALID_DATA;
            }
        }
    }

    return status;
}
//enum->text
const char *BloodGroupToString(BmsBloodGroup_t bloodGroup)
{
    const char *text;

    switch (bloodGroup)
    {
        case BMS_BLOOD_GROUP_A_POSITIVE:
        {
            text = "A+";
            break;
        }

        case BMS_BLOOD_GROUP_A_NEGATIVE:
        {
            text = "A-";
            break;
        }

        case BMS_BLOOD_GROUP_B_POSITIVE:
        {
            text = "B+";
            break;
        }

        case BMS_BLOOD_GROUP_B_NEGATIVE:
        {
            text = "B-";
            break;
        }

        case BMS_BLOOD_GROUP_AB_POSITIVE:
        {
            text = "AB+";
            break;
        }

        case BMS_BLOOD_GROUP_AB_NEGATIVE:
        {
            text = "AB-";
            break;
        }

        case BMS_BLOOD_GROUP_O_POSITIVE:
        {
            text = "O+";
            break;
        }

        case BMS_BLOOD_GROUP_O_NEGATIVE:
        {
            text = "O-";
            break;
        }

        case BMS_BLOOD_GROUP_A1_POSITIVE:
        {
            text = "A1+";
            break;
        }

        case BMS_BLOOD_GROUP_INVALID:
        case BMS_BLOOD_GROUP_COUNT:
        default:
        {
            text = "INVALID";
            break;
        }
    }

    return text;
}

bool ValidateBloodGroup(const char *bloodGroup)
{
    BmsBloodGroup_t value = BMS_BLOOD_GROUP_INVALID;

    return ParseBloodGroup(bloodGroup, &value) == BMS_STATUS_OK;
}

bool ValidateBloodGroupValue(BmsBloodGroup_t value)
{
    return (value > BMS_BLOOD_GROUP_INVALID) &&
           (value < BMS_BLOOD_GROUP_COUNT);
}
bool ValidateUnits(uint32_t units)
{ return (units > 0U) && (units <= BMS_MAX_BLOOD_UNITS); }
bool ValidateAge(uint32_t age)
{ return (age >= BMS_MIN_DONOR_AGE) && (age <= BMS_MAX_DONOR_AGE); }
bool ValidateWeight(uint32_t weightKg)
{ return weightKg >= BMS_MIN_DONOR_WEIGHT_KG; }

bool ValidateDateValue(const BmsDate_t *date)
{
    uint8_t days;
    if ((date == NULL) || (date->year < 1900U) || (date->month == 0U) ||
        (date->month > 12U)) { return false; }
    days = UtilityDaysInMonth(date->year, date->month);
    return (date->day > 0U) && (date->day <= days);
}

BmsStatus_t BmsDateAddDays(const BmsDate_t *source,
                           uint32_t daysToAdd,
                           BmsDate_t *result)
{
    uint32_t index;

    if ((source == NULL) || (result == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    if (!ValidateDateValue(source))
    {
        return BMS_STATUS_INVALID_DATA;
    }

    *result = *source;
    for (index = 0U; index < daysToAdd; ++index)
    {
        const uint8_t daysInMonth =
            UtilityDaysInMonth(result->year, result->month);

        if (result->day < daysInMonth)
        {
            result->day = (uint8_t)(result->day + 1U);
        }
        else
        {
            result->day = 1U;
            if (result->month < 12U)
            {
                result->month = (uint8_t)(result->month + 1U);
            }
            else
            {
                if (result->year == UINT16_MAX)
                {
                    return BMS_STATUS_INVALID_DATA;
                }
                result->month = 1U;
                result->year = (uint16_t)(result->year + 1U);
            }
        }
    }

    return BMS_STATUS_OK;
}

bool ValidateDate(const char *date)
{
    BmsDate_t parsed = {0U, 0U, 0U};
    unsigned int y, m, d;
    if ((date == NULL) || (strlen(date) != 10U)) { return false; }
    if (sscanf(date, "%4u-%2u-%2u", &y, &m, &d) != 3) { return false; }
    parsed.year = (uint16_t)y; parsed.month = (uint8_t)m; parsed.day = (uint8_t)d;
    return ValidateDateValue(&parsed);
}

bool ValidateUserRole(uint8_t role)
{ return ValidateUserRoleValue((BmsUserRole_t)role); }
bool ValidateUserRoleValue(BmsUserRole_t role)
{ return (role >= BMS_ROLE_ADMIN) && (role <= BMS_ROLE_DONOR); }
