/**
 * @file main.c
 * @brief Console entry point for the Blood Bank Management System.
 *
 * Navigation available in every data-entry workflow:
 *   -1  : move to the previous field
 *    0  : cancel and return to the current role menu
 *   -99 : cancel and return to the application main/login menu
 *
 * This file follows the project's MISRA-oriented checklist: fixed-width
 * integers, initialized variables, bounded input, pointer checks, braces on
 * control statements, checked business-API return values, no hardcoded
 * credentials, and explicit switch default cases.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "authentication.h"
#include "blood_inventory.h"
#include "blood_request_management.h"
#include "common_validation.h"
#include "donation_management.h"
#include "donation_camp_management.h"
#include "donor_management.h"
#include "emergency_alert_management.h"
#include "file_management.h"
#include "hospital_management.h"
#include "notification_management.h"
#include "multithreading.h"
#include "report_management.h"
#include "utility.h"

void BmsReferencePublicApis(void);

#define BMS_UI_INPUT_LENGTH             (256U)
#define BMS_UI_DATE_INPUT_LENGTH        (16U)
#define BMS_UI_NAV_PREVIOUS             (-1)
#define BMS_UI_NAV_MENU                 (0)
#define BMS_UI_NAV_MAIN                 (-99)
#define BMS_UI_FIRST_FIELD              (0U)
#define BMS_UI_MIN_PASSWORD_LENGTH      (8U)
#define BMS_UI_EXIT_SUCCESS             (0)
#define BMS_UI_EXIT_FAILURE             (1)
#define BMS_INVENTORY_MONITOR_INTERVAL_SECONDS (30U)

typedef enum
{
    BMS_UI_VALUE = 0,
    BMS_UI_PREVIOUS,
    BMS_UI_MENU,
    BMS_UI_MAIN,
    BMS_UI_INVALID
} BmsUiInputResult_t;

typedef enum
{
    BMS_UI_OPERATION_DONE = 0,
    BMS_UI_OPERATION_MENU,
    BMS_UI_OPERATION_MAIN
} BmsUiOperationResult_t;

typedef enum
{
    BMS_UI_ROLE_LOGOUT = 0,
    BMS_UI_ROLE_MAIN
} BmsUiRoleResult_t;
typedef struct
{
    BmsAuthenticationContext_t authentication;
    BmsDonorContext_t donors;
    BmsHospitalContext_t hospitals;
    BmsInventoryContext_t inventory;
    BmsDonationContext_t donations;
    BmsBloodRequestContext_t requests;
    BmsNotificationContext_t notifications;
    BmsEmergencyAlertContext_t alerts;
    BmsDonationCampContext_t camps;
    BmsThreadController_t threadController;
} BmsApplication_t;

static void PrintTitle(const char *title);
static void PrintStatus(const char *operation, BmsStatus_t status);
static void PrintNavigationHelp(void);
static BmsUiInputResult_t UiReadRaw(const char *prompt,
                                    char *buffer,
                                    size_t bufferSize);
static BmsUiInputResult_t UiReadText(const char *prompt,
                                     char *buffer,
                                     size_t bufferSize);
static BmsUiInputResult_t UiReadUint32(const char *prompt, uint32_t *value);
static BmsUiInputResult_t UiReadDate(const char *prompt, BmsDate_t *date);
static BmsUiInputResult_t UiReadBloodGroup(BmsBloodGroup_t *bloodGroup);
static BmsUiInputResult_t UiReadYesNo(const char *prompt, bool *value);
static bool MoveToPreviousField(uint32_t *fieldIndex);
static BmsUiOperationResult_t MapNavigation(BmsUiInputResult_t result);
static const char *RoleToString(BmsUserRole_t role);
static const char *RequestStatusToString(BmsRequestStatus_t status);

static BmsStatus_t ApplicationInitialize(BmsApplication_t *application);
static BmsStatus_t ApplicationSave(BmsApplication_t *application);
static void ApplicationDeinitialize(BmsApplication_t *application);
static BmsStatus_t FirstRunAdministratorSetup(BmsApplication_t *application);

static void MainMenu(BmsApplication_t *application);
static BmsUiOperationResult_t PublicRegister(BmsApplication_t *application);
static BmsUiOperationResult_t ForgotPassword(BmsApplication_t *application);
static BmsUiOperationResult_t ChangePassword(BmsApplication_t *application, const BmsUser_t *user);
static BmsUiOperationResult_t ReviewPendingUsers(BmsApplication_t *application);
static BmsUiRoleResult_t AdminMenu(BmsApplication_t *application,
                                   const BmsUser_t *user);
static BmsUiRoleResult_t BloodBankStaffMenu(BmsApplication_t *application,
                                            const BmsUser_t *user);
static BmsUiRoleResult_t HospitalStaffMenu(BmsApplication_t *application,
                                           const BmsUser_t *user);
static BmsUiRoleResult_t DonorMenu(BmsApplication_t *application,
                                   const BmsUser_t *user);

static BmsUiOperationResult_t RegisterUser(BmsApplication_t *application);
static BmsUiOperationResult_t AddDonor(BmsApplication_t *application);
static void ListDonors(BmsApplication_t *application);
static BmsUiOperationResult_t SearchDonor(BmsApplication_t *application);
static BmsUiOperationResult_t DeleteDonor(BmsApplication_t *application);
static BmsUiOperationResult_t AddHospital(BmsApplication_t *application, BmsUserId_t linkedUserId);
static void ListHospitals(BmsApplication_t *application);
static BmsUiOperationResult_t AddRoute(BmsApplication_t *application, const BmsUser_t *user);
static BmsUiOperationResult_t AddInventory(BmsApplication_t *application);
static void ListInventory(BmsApplication_t *application);
static BmsUiOperationResult_t CheckAvailability(BmsApplication_t *application);
static BmsUiOperationResult_t RecordDonation(BmsApplication_t *application,
                                                        const BmsUser_t *user);
static void ListDonations(BmsApplication_t *application);
static BmsUiOperationResult_t CreateBloodRequest(BmsApplication_t *application,
                                                  const BmsUser_t *requester);
static void ListRequests(BmsApplication_t *application);
static void ProcessNextRequest(BmsApplication_t *application);
static BmsUiOperationResult_t CreateEmergencyAlert(BmsApplication_t *application, const BmsUser_t *user);
static void ViewNotifications(BmsApplication_t *application, const BmsUser_t *user);
static void ViewEmergencyAlerts(BmsApplication_t *application);
static BmsUiOperationResult_t AddDonationCamp(BmsApplication_t *application);
static void ViewDonationCamps(BmsApplication_t *application);
static void ShowSummary(BmsApplication_t *application);
static void GenerateReports(BmsApplication_t *application);

static BmsStatus_t PrintDonorVisitor(const BmsDonor_t *donor, void *context);
static BmsStatus_t PrintHospitalVisitor(const BmsHospital_t *hospital,
                                        void *context);
static BmsStatus_t PrintInventoryVisitor(const BmsBloodInventory_t *record,
                                         void *context);
static BmsStatus_t PrintDonationVisitor(const BmsDonation_t *donation,
                                        void *context);
static BmsStatus_t PrintRequestVisitor(const BmsBloodRequest_t *request,
                                       void *context);
static uint32_t NextDonorId(const BmsApplication_t *application);
static uint32_t NextHospitalId(const BmsApplication_t *application);
static uint32_t NextBloodId(const BmsApplication_t *application);
static uint32_t NextDonationId(const BmsApplication_t *application);
static uint32_t NextRequestId(const BmsApplication_t *application);
static uint32_t NextAlertId(const BmsApplication_t *application);
static uint32_t NextNotificationId(const BmsApplication_t *application);
static uint32_t NextCampId(const BmsApplication_t *application);
static BmsStatus_t NotifyBloodBankStaff(
    BmsApplication_t *application,
    const BmsEmergencyAlert_t *alert);


static uint32_t NextIdFromList(const BmsLinkedList_t *list, size_t idOffset)
{
    uint32_t maximum = 0U;
    const BmsLinkedListNode_t *node = (list != NULL) ? list->head : NULL;
    while (node != NULL)
    {
        if (node->data != NULL)
        {
            uint32_t value = 0U;
            (void)memcpy(&value, ((const unsigned char *)node->data) + idOffset, sizeof(value));
            if (value > maximum) { maximum = value; }
        }
        node = node->next;
    }
    return (maximum == UINT32_MAX) ? 0U : (maximum + 1U);
}
static uint32_t NextDonorId(const BmsApplication_t *a) { return NextIdFromList(&a->donors.donors, offsetof(BmsDonor_t, donorId)); }
static uint32_t NextHospitalId(const BmsApplication_t *a) { return NextIdFromList(&a->hospitals.hospitals, offsetof(BmsHospital_t, hospitalId)); }
static uint32_t NextBloodId(const BmsApplication_t *a) { return NextIdFromList(&a->inventory.inventory, offsetof(BmsBloodInventory_t, bloodId)); }
static uint32_t NextDonationId(const BmsApplication_t *a) { return NextIdFromList(&a->donations.donations, offsetof(BmsDonation_t, donationId)); }
static uint32_t NextRequestId(const BmsApplication_t *a) { return NextIdFromList(&a->requests.requests, offsetof(BmsBloodRequest_t, requestId)); }
static uint32_t NextAlertId(const BmsApplication_t *a) { return NextIdFromList(&a->alerts.alertHistory, offsetof(BmsEmergencyAlert_t, alertId)); }
static uint32_t NextNotificationId(const BmsApplication_t *a) { return NextIdFromList(&a->notifications.history, offsetof(BmsNotification_t, notificationId)); }
static uint32_t NextCampId(const BmsApplication_t *a) { return NextIdFromList(&a->camps.camps, offsetof(BmsDonationCamp_t, campId)); }

int main(void)
{
    BmsApplication_t application;
    BmsStatus_t status = BMS_STATUS_INTERNAL_ERROR;
    int exitCode = BMS_UI_EXIT_FAILURE;

    BmsReferencePublicApis();
    (void)memset(&application, 0, sizeof(application));
    PrintTitle("BLOOD BANK MANAGEMENT SYSTEM - VERSION 2");

    status = ApplicationInitialize(&application);
    if (status == BMS_STATUS_OK)
    {
        if (!AuthenticationAdministratorExists(&application.authentication))
        {
            status = FirstRunAdministratorSetup(&application);
        }

        if (status == BMS_STATUS_OK)
        {
            BmsStatus_t threadStatus = BmsThreadControllerStart(
                &application.threadController,
                &application.inventory,
                BMS_INVENTORY_MONITOR_INTERVAL_SECONDS,
                BMS_LOW_STOCK_THRESHOLD_UNITS);
            if (threadStatus == BMS_STATUS_OK)
            {
                (void)printf("Background inventory monitor started.\n");
            }
            else
            {
                PrintStatus("Start background inventory monitor", threadStatus);
            }

            MainMenu(&application);
            BmsThreadControllerStop(&application.threadController);
            if (threadStatus == BMS_STATUS_OK)
            {
                (void)printf("Inventory monitor stopped: expired=%u, low-stock=%u.\n",
                             application.threadController.lastExpiredCount,
                             application.threadController.lastLowStockCount);
            }
            status = ApplicationSave(&application);
            if (status != BMS_STATUS_OK)
            {
                PrintStatus("Save application data", status);
            }
            else
            {
                exitCode = BMS_UI_EXIT_SUCCESS;
            }
        }
        else
        {
            PrintStatus("Administrator setup", status);
        }

        ApplicationDeinitialize(&application);
    }
    else
    {
        PrintStatus("Application initialization", status);
    }

    (void)printf("\nThank you for using the Blood Bank Management System.\n");
    return exitCode;
}

static void PrintTitle(const char *title)
{
    if (title != NULL)
    {
        (void)printf("\n============================================================\n");
        (void)printf("%s\n", title);
        (void)printf("============================================================\n");
    }
}

static void PrintStatus(const char *operation, BmsStatus_t status)
{
    if (operation != NULL)
    {
        (void)printf("%s: %s\n", operation, UtilityStatusToString(status));
    }
}

static void PrintNavigationHelp(void)
{
    (void)printf("[-1 = previous field | 0 = current menu | -99 = main menu]\n");
}

static BmsUiInputResult_t UiReadRaw(const char *prompt,
                                    char *buffer,
                                    size_t bufferSize)
{
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;
    BmsUiInputResult_t result = BMS_UI_INVALID;

    if ((prompt != NULL) && (buffer != NULL) && (bufferSize > 1U))
    {
        (void)printf("%s", prompt);
        status = UtilityReadLine(buffer, bufferSize);
        if (status == BMS_STATUS_OK)
        {
            UtilityTrimWhitespace(buffer);
            if (strcmp(buffer, "-1") == 0)
            {
                result = BMS_UI_PREVIOUS;
            }
            else if (strcmp(buffer, "0") == 0)
            {
                result = BMS_UI_MENU;
            }
            else if (strcmp(buffer, "-99") == 0)
            {
                result = BMS_UI_MAIN;
            }
            else if (buffer[0] != '\0')
            {
                result = BMS_UI_VALUE;
            }
            else
            {
                result = BMS_UI_INVALID;
            }
        }
    }

    return result;
}

static BmsUiInputResult_t UiReadText(const char *prompt,
                                     char *buffer,
                                     size_t bufferSize)
{
    return UiReadRaw(prompt, buffer, bufferSize);
}

static BmsUiInputResult_t UiReadUint32(const char *prompt, uint32_t *value)
{
    BmsUiInputResult_t result = BMS_UI_INVALID;

    if ((prompt != NULL) && (value != NULL))
    {
        char input[BMS_UI_INPUT_LENGTH] = { '\0' };
        uint32_t parsedValue = 0U;
        result = UiReadRaw(prompt, input, sizeof(input));
        if (result == BMS_UI_VALUE)
        {
            const BmsStatus_t status = UtilityParseUint32(input, &parsedValue);
            if (status == BMS_STATUS_OK)
            {
                *value = parsedValue;
            }
            else
            {
                result = BMS_UI_INVALID;
            }
        }
    }

    return result;
}

static BmsUiInputResult_t UiReadDate(const char *prompt, BmsDate_t *date)
{
    BmsUiInputResult_t result = BMS_UI_INVALID;

    if ((prompt != NULL) && (date != NULL))
    {
        char input[BMS_UI_DATE_INPUT_LENGTH] = { '\0' };
        unsigned int year = 0U;
        unsigned int month = 0U;
        unsigned int day = 0U;
        BmsDate_t parsedDate = { 0U, 0U, 0U };
        result = UiReadRaw(prompt, input, sizeof(input));
        if (result == BMS_UI_VALUE)
        {
            if ((sscanf(input, "%u-%u-%u", &year, &month, &day) == 3) &&
                (year <= UINT16_MAX) &&
                (month <= UINT8_MAX) &&
                (day <= UINT8_MAX))
            {
                parsedDate.year = (uint16_t)year;
                parsedDate.month = (uint8_t)month;
                parsedDate.day = (uint8_t)day;
                if (ValidateDateValue(&parsedDate))
                {
                    *date = parsedDate;
                }
                else
                {
                    result = BMS_UI_INVALID;
                }
            }
            else
            {
                result = BMS_UI_INVALID;
            }
        }
    }

    return result;
}

static BmsUiInputResult_t UiReadBloodGroup(BmsBloodGroup_t *bloodGroup)
{
    uint32_t choice = 0U;
    BmsUiInputResult_t result = BMS_UI_INVALID;

    if (bloodGroup != NULL)
    {
        (void)printf("\nSelect Blood Group\n\n");
        (void)printf("1. A+\n");
        (void)printf("2. A-\n");
        (void)printf("3. B+\n");
        (void)printf("4. B-\n");
        (void)printf("5. AB+\n");
        (void)printf("6. AB-\n");
        (void)printf("7. O+\n");
        (void)printf("8. O-\n");
        (void)printf("9. A1+\n\n");
        (void)printf("-1 = Previous field\n");
        (void)printf("0 = Current menu\n");
        (void)printf("-99 = Main menu\n\n");
        result = UiReadUint32("Select blood group: ", &choice);
        if (result == BMS_UI_VALUE)
        {
            if ((choice >= 1U) && (choice <= 9U))
            {
                *bloodGroup = (BmsBloodGroup_t)choice;
            }
            else
            {
                result = BMS_UI_INVALID;
            }
        }
    }

    return result;
}


static BmsUiInputResult_t UiReadYesNo(const char *prompt, bool *value)
{
    BmsUiInputResult_t result = BMS_UI_INVALID;

    if ((prompt != NULL) && (value != NULL))
    {
        char input[BMS_UI_INPUT_LENGTH] = { '\0' };
        result = UiReadRaw(prompt, input, sizeof(input));
        if (result == BMS_UI_VALUE)
        {
            if ((strcmp(input, "y") == 0) || (strcmp(input, "Y") == 0) ||
                (strcmp(input, "yes") == 0) || (strcmp(input, "YES") == 0))
            {
                *value = true;
            }
            else if ((strcmp(input, "n") == 0) || (strcmp(input, "N") == 0) ||
                     (strcmp(input, "no") == 0) || (strcmp(input, "NO") == 0))
            {
                *value = false;
            }
            else
            {
                result = BMS_UI_INVALID;
            }
        }
    }

    return result;
}

static bool MoveToPreviousField(uint32_t *fieldIndex)
{
    bool moved = false;

    if (fieldIndex != NULL)
    {
        if (*fieldIndex > BMS_UI_FIRST_FIELD)
        {
            --(*fieldIndex);
            moved = true;
        }
        else
        {
            (void)printf("Already at the first field.\n");
        }
    }

    return moved;
}

static BmsUiOperationResult_t MapNavigation(BmsUiInputResult_t result)
{
    BmsUiOperationResult_t operationResult = BMS_UI_OPERATION_DONE;

    if (result == BMS_UI_MAIN)
    {
        operationResult = BMS_UI_OPERATION_MAIN;
    }
    else if (result == BMS_UI_MENU)
    {
        operationResult = BMS_UI_OPERATION_MENU;
    }
    else
    {
        operationResult = BMS_UI_OPERATION_DONE;
    }

    return operationResult;
}

static const char *RoleToString(BmsUserRole_t role)
{
    const char *text;

    switch (role)
    {
        case BMS_ROLE_ADMIN:
            text = "Administrator";
            break;
        case BMS_ROLE_HOSPITAL_STAFF:
            text = "Hospital Staff";
            break;
        case BMS_ROLE_BLOOD_BANK_STAFF:
            text = "Blood Bank Staff";
            break;
        case BMS_ROLE_DONOR:
            text = "Donor";
            break;
        default:
            text = "Invalid";
            break;
    }

    return text;
}

static const char *RequestStatusToString(BmsRequestStatus_t status)
{
    const char *text;

    switch (status)
    {
        case BMS_REQUEST_STATUS_PENDING:
            text = "Pending";
            break;
        case BMS_REQUEST_STATUS_APPROVED:
            text = "Approved";
            break;
        case BMS_REQUEST_STATUS_REJECTED:
            text = "Rejected";
            break;
        case BMS_REQUEST_STATUS_PROCESSING:
            text = "Processing";
            break;
        case BMS_REQUEST_STATUS_FULFILLED:
            text = "Fulfilled";
            break;
        case BMS_REQUEST_STATUS_CANCELLED:
            text = "Cancelled";
            break;
        default:
            text = "Invalid";
            break;
    }

    return text;
}

static BmsStatus_t ApplicationInitialize(BmsApplication_t *application)
{
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;

    if (application != NULL)
    {
        (void)memset(application, 0, sizeof(*application));
        status = FileManagementInitialize();
        if (status == BMS_STATUS_OK)
        {
            status = AuthenticationInitialize(&application->authentication);
        }
        if (status == BMS_STATUS_OK)
        {
            status = DonorManagementInitialize(&application->donors);
        }
        if (status == BMS_STATUS_OK)
        {
            status = HospitalManagementInitialize(&application->hospitals);
        }
        if (status == BMS_STATUS_OK)
        {
            status = BloodInventoryInitialize(&application->inventory);
        }
        if (status == BMS_STATUS_OK)
        {
            status = DonationManagementInitialize(&application->donations);
        }
        if (status == BMS_STATUS_OK)
        {
            status = BloodRequestManagementInitialize(&application->requests);
        }
        if (status == BMS_STATUS_OK)
        {
            status = NotificationManagementInitialize(&application->notifications);
        }
        if (status == BMS_STATUS_OK)
        {
            status = EmergencyAlertManagementInitialize(&application->alerts);
        }
        if (status == BMS_STATUS_OK)
        {
            status = DonationCampManagementInitialize(&application->camps);
        }

        if (status == BMS_STATUS_OK)
        {
            BmsStatus_t loadStatus = AuthenticationLoad(&application->authentication);
            if ((loadStatus != BMS_STATUS_OK) &&
                (loadStatus != BMS_STATUS_FILE_NOT_FOUND))
            {
                PrintStatus("Load authentication data", loadStatus);
            }
            loadStatus = DonorManagementLoad(&application->donors);
            if ((loadStatus != BMS_STATUS_OK) &&
                (loadStatus != BMS_STATUS_FILE_NOT_FOUND))
            {
                PrintStatus("Load donor data", loadStatus);
            }
            loadStatus = HospitalManagementLoad(&application->hospitals);
            if ((loadStatus != BMS_STATUS_OK) &&
                (loadStatus != BMS_STATUS_FILE_NOT_FOUND))
            {
                PrintStatus("Load hospital data", loadStatus);
            }
            loadStatus = BloodInventoryLoad(&application->inventory);
            if ((loadStatus != BMS_STATUS_OK) &&
                (loadStatus != BMS_STATUS_FILE_NOT_FOUND))
            {
                PrintStatus("Load inventory data", loadStatus);
            }
            loadStatus = DonationManagementLoad(&application->donations);
            if ((loadStatus != BMS_STATUS_OK) &&
                (loadStatus != BMS_STATUS_FILE_NOT_FOUND))
            {
                PrintStatus("Load donation data", loadStatus);
            }
            loadStatus = BloodRequestManagementLoad(&application->requests);
            if ((loadStatus != BMS_STATUS_OK) &&
                (loadStatus != BMS_STATUS_FILE_NOT_FOUND))
            {
                PrintStatus("Load request data", loadStatus);
            }
            loadStatus = NotificationManagementLoad(&application->notifications);
            if ((loadStatus != BMS_STATUS_OK) &&
                (loadStatus != BMS_STATUS_FILE_NOT_FOUND))
            {
                PrintStatus("Load notification data", loadStatus);
            }
            loadStatus = EmergencyAlertManagementLoad(&application->alerts);
            if ((loadStatus != BMS_STATUS_OK) &&
                (loadStatus != BMS_STATUS_FILE_NOT_FOUND))
            {
                PrintStatus("Load alert data", loadStatus);
            }
            loadStatus = DonationCampManagementLoad(&application->camps);
            if ((loadStatus != BMS_STATUS_OK) && (loadStatus != BMS_STATUS_FILE_NOT_FOUND)) { PrintStatus("Load donation camp data", loadStatus); }
        }
    }

    return status;
}

static BmsStatus_t ApplicationSave(BmsApplication_t *application)
{
    BmsStatus_t overallStatus = BMS_STATUS_INVALID_ARGUMENT;

    if (application != NULL)
    {
        BmsStatus_t status = AuthenticationSave(&application->authentication);
        overallStatus = status;
        if (status != BMS_STATUS_OK)
        {
            PrintStatus("Save authentication data", status);
        }

        status = DonorManagementSave(&application->donors);
        if (status != BMS_STATUS_OK)
        {
            PrintStatus("Save donor data", status);
            overallStatus = status;
        }
        status = HospitalManagementSave(&application->hospitals);
        if (status != BMS_STATUS_OK)
        {
            PrintStatus("Save hospital data", status);
            overallStatus = status;
        }
        status = BloodInventorySave(&application->inventory);
        if (status != BMS_STATUS_OK)
        {
            PrintStatus("Save inventory data", status);
            overallStatus = status;
        }
        status = DonationManagementSave(&application->donations);
        if (status != BMS_STATUS_OK)
        {
            PrintStatus("Save donation data", status);
            overallStatus = status;
        }
        status = BloodRequestManagementSave(&application->requests);
        if (status != BMS_STATUS_OK)
        {
            PrintStatus("Save request data", status);
            overallStatus = status;
        }
        status = NotificationManagementSave(&application->notifications);
        if (status != BMS_STATUS_OK)
        {
            PrintStatus("Save notification data", status);
            overallStatus = status;
        }
        status = EmergencyAlertManagementSave(&application->alerts);
        if (status != BMS_STATUS_OK)
        {
            PrintStatus("Save alert data", status);
            overallStatus = status;
        }
        status = DonationCampManagementSave(&application->camps);
        if (status != BMS_STATUS_OK) { PrintStatus("Save donation camp data", status); overallStatus = status; }
    }

    return overallStatus;
}

static void ApplicationDeinitialize(BmsApplication_t *application)
{
    if (application != NULL)
    {
        DonationCampManagementDeinitialize(&application->camps);
        EmergencyAlertManagementDeinitialize(&application->alerts);
        NotificationManagementDeinitialize(&application->notifications);
        BloodRequestManagementDeinitialize(&application->requests);
        DonationManagementDeinitialize(&application->donations);
        BloodInventoryDeinitialize(&application->inventory);
        HospitalManagementDeinitialize(&application->hospitals);
        DonorManagementDeinitialize(&application->donors);
        AuthenticationDeinitialize(&application->authentication);
    }
}

static BmsStatus_t FirstRunAdministratorSetup(BmsApplication_t *application)
{
    BmsUser_t user;
    char password[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    char confirmPassword[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    char recoveryCode[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    uint32_t field = 0U;
    BmsStatus_t status = BMS_STATUS_OK;

    if (application == NULL)
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    (void)memset(&user, 0, sizeof(user));
    user.role = BMS_ROLE_ADMIN;
    user.status = BMS_USER_STATUS_ACTIVE;
    user.isActive = true;
    (void)AuthenticationGetNextUserId(&application->authentication, &user.userId);

    PrintTitle("FIRST-RUN ADMINISTRATOR SETUP");
    (void)printf("Create the bootstrap administrator. This screen appears only when no administrator exists.\n");
    PrintNavigationHelp();

    while (field < 3U)
    {
        BmsUiInputResult_t inputResult = BMS_UI_INVALID;
        switch (field)
        {
            case 0U:
                inputResult = UiReadText("Administrator username: ", user.username,
                                         sizeof(user.username));
                if ((inputResult == BMS_UI_VALUE) && (!ValidateUsername(user.username)))
                {
                    (void)printf("Username must be 4-%u characters and contain only letters, numbers, _ or .\n",
                                 BMS_MAX_USERNAME_LENGTH);
                    inputResult = BMS_UI_INVALID;
                }
                break;
            case 1U:
                inputResult = UiReadText("Administrator password: ", password,
                                         sizeof(password));
                if ((inputResult == BMS_UI_VALUE) && (!ValidatePassword(password)))
                {
                    (void)printf("Password needs 8+ characters with uppercase, lowercase, number and special character.\n");
                    inputResult = BMS_UI_INVALID;
                }
                break;
            case 2U:
                inputResult = UiReadText("Confirm password: ", confirmPassword,
                                         sizeof(confirmPassword));
                if ((inputResult == BMS_UI_VALUE) &&
                    (strcmp(password, confirmPassword) != 0))
                {
                    (void)printf("Passwords do not match.\n");
                    inputResult = BMS_UI_INVALID;
                }
                break;
            default:
                inputResult = BMS_UI_INVALID;
                break;
        }

        if (inputResult == BMS_UI_VALUE)
        {
            ++field;
        }
        else if (inputResult == BMS_UI_PREVIOUS)
        {
            (void)MoveToPreviousField(&field);
        }
        else if ((inputResult == BMS_UI_MENU) ||
                 (inputResult == BMS_UI_MAIN))
        {
            status = BMS_STATUS_ACCESS_DENIED;
            break;
        }
        else
        {
            (void)printf("Invalid input. Please try again.\n");
        }
    }

    if ((field == 3U) && (status != BMS_STATUS_ACCESS_DENIED))
    {
        status = AuthenticationRegisterUser(&application->authentication, &user,
                                            password);
        if (status == BMS_STATUS_OK)
        {
            (void)snprintf(recoveryCode, sizeof(recoveryCode), "BMS-%lu-%s",
                           (unsigned long)user.userId, user.username);
            status = AuthenticationSetRecoveryCode(&application->authentication,
                                                   user.userId, recoveryCode);
        }
        if (status == BMS_STATUS_OK)
        {
            status = AuthenticationSave(&application->authentication);
        }
        PrintStatus("Create initial administrator", status);
        if (status == BMS_STATUS_OK)
        {
            (void)printf("Recovery code: %s\nStore this code securely.\n", recoveryCode);
        }
    }

    UtilitySecureZero(password, sizeof(password));
    UtilitySecureZero(confirmPassword, sizeof(confirmPassword));
    UtilitySecureZero(recoveryCode, sizeof(recoveryCode));
    return status;
}

static void MainMenu(BmsApplication_t *application)
{
    if (application != NULL)
    {
        bool exitRequested = false;
        while (!exitRequested)
        {
            uint32_t choice = 0U;
            PrintTitle("MAIN MENU");
            (void)printf("1. Register\n");
            (void)printf("2. Login\n");
            (void)printf("3. Forgot password\n");
            (void)printf("4. Exit\n");

            if (UiReadUint32("Enter choice: ", &choice) != BMS_UI_VALUE)
            {
                (void)printf("Invalid choice.\n");
                continue;
            }

            switch (choice)
            {
                case 1U:
                    (void)PublicRegister(application);
                    break;
                case 2U:
                {
                    char username[BMS_MAX_USERNAME_LENGTH + 1U] = { '\0' };
                    char password[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
                    uint32_t field = 0U;
                    bool ready = false;
                    bool cancelled = false;

                    PrintTitle("LOGIN");
                    PrintNavigationHelp();
                    (void)printf("Forgot your password? Enter 0 and choose option 3 in the main menu.\n");

                    while ((!ready) && (!cancelled))
                    {
                        BmsUiInputResult_t inputResult = BMS_UI_INVALID;
                        if (field == 0U)
                        {
                            inputResult = UiReadText("Username: ", username,
                                                     sizeof(username));
                        }
                        else
                        {
                            inputResult = UiReadText("Password: ", password,
                                                     sizeof(password));
                        }

                        if (inputResult == BMS_UI_VALUE)
                        {
                            ++field;
                            ready = (field > 1U);
                        }
                        else if (inputResult == BMS_UI_PREVIOUS)
                        {
                            (void)MoveToPreviousField(&field);
                        }
                        else if ((inputResult == BMS_UI_MENU) ||
                                 (inputResult == BMS_UI_MAIN))
                        {
                            cancelled = true;
                        }
                        else
                        {
                            (void)printf("Invalid input.\n");
                        }
                    }

                    if (ready)
                    {
                        BmsUser_t user;
                        BmsStatus_t loginStatus;
                        BmsUiRoleResult_t roleResult = BMS_UI_ROLE_LOGOUT;
                        (void)memset(&user, 0, sizeof(user));

                        loginStatus = AuthenticationLogin(&application->authentication,
                                                          username, password, &user);
                        (void)AuthenticationSave(&application->authentication);

                        if (loginStatus == BMS_STATUS_OK)
                        {
                            (void)printf("\nWelcome %s (%s).\n", user.username,
                                         RoleToString(user.role));
                            switch (user.role)
                            {
                                case BMS_ROLE_ADMIN:
                                    roleResult = AdminMenu(application, &user);
                                    break;
                                case BMS_ROLE_BLOOD_BANK_STAFF:
                                    roleResult = BloodBankStaffMenu(application, &user);
                                    break;
                                case BMS_ROLE_HOSPITAL_STAFF:
                                    roleResult = HospitalStaffMenu(application, &user);
                                    break;
                                case BMS_ROLE_DONOR:
                                    roleResult = DonorMenu(application, &user);
                                    break;
                                default:
                                    (void)printf("Unsupported user role.\n");
                                    break;
                            }
                            (void)roleResult;
                        }
                        else
                        {
                            BmsUser_t storedUser;
                            (void)memset(&storedUser, 0, sizeof(storedUser));
                            if ((AuthenticationFindUserByUsername(&application->authentication,
                                                                  username,
                                                                  &storedUser) == BMS_STATUS_OK) &&
                                (storedUser.status == BMS_USER_STATUS_PENDING))
                            {
                                (void)printf("Your account is pending administrator approval.\n");
                            }
                            else if (loginStatus == BMS_STATUS_ACCOUNT_LOCKED)
                            {
                                (void)printf("Account locked after repeated failed attempts. Use Forgot Password.\n");
                            }
                            else
                            {
                                (void)printf("Invalid username or password.\n");
                            }
                        }
                    }
                    UtilitySecureZero(password, sizeof(password));
                    break;
                }
                case 3U:
                    (void)ForgotPassword(application);
                    break;
                case 4U:
                    exitRequested = true;
                    break;
                default:
                    (void)printf("Invalid choice. Enter 1 to 4.\n");
                    break;
            }
        }
    }
}

#define HANDLE_OPERATION_RESULT(call_)                                      \
    do                                                                       \
    {                                                                        \
        BmsUiOperationResult_t operationResult_ = (call_);                   \
        if (operationResult_ == BMS_UI_OPERATION_MAIN)                       \
        {                                                                    \
            return BMS_UI_ROLE_MAIN;                                         \
        }                                                                    \
    } while (false)

static BmsUiRoleResult_t AdminMenu(BmsApplication_t *application,const BmsUser_t *user)
{ bool done=false;if((application==NULL)||(user==NULL))return BMS_UI_ROLE_LOGOUT;while(!done){uint32_t c=0U;PrintTitle("ADMINISTRATOR MENU");
(void)printf("1. Create Administrator/Blood Bank Staff account\n2. Review pending Hospital Staff registrations\n3. List donors\n4. Search donor\n5. Delete donor\n6. List hospitals\n7. List inventory\n8. List donations\n9. View blood requests\n10. View emergency alerts\n11. View notifications\n12. Reports and summary\n13. Change password\n14. Logout\n");
if(UiReadUint32("Enter choice: ",&c)!=BMS_UI_VALUE){continue;}switch(c){case 1U:HANDLE_OPERATION_RESULT(RegisterUser(application));break;case 2U:HANDLE_OPERATION_RESULT(ReviewPendingUsers(application));break;case 3U:ListDonors(application);break;case 4U:HANDLE_OPERATION_RESULT(SearchDonor(application));break;case 5U:HANDLE_OPERATION_RESULT(DeleteDonor(application));break;case 6U:ListHospitals(application);break;case 7U:ListInventory(application);break;case 8U:ListDonations(application);break;case 9U:ListRequests(application);break;case 10U:ViewEmergencyAlerts(application);break;case 11U:ViewNotifications(application,user);break;case 12U:ShowSummary(application);GenerateReports(application);break;case 13U:HANDLE_OPERATION_RESULT(ChangePassword(application,user));break;case 14U:done=true;break;default:(void)printf("Invalid choice.\n");break;}}return BMS_UI_ROLE_LOGOUT;}

static BmsUiRoleResult_t BloodBankStaffMenu(BmsApplication_t *application,const BmsUser_t *user)
{bool done=false;if((application==NULL)||(user==NULL))return BMS_UI_ROLE_LOGOUT;while(!done){uint32_t c=0U;PrintTitle("BLOOD BANK STAFF MENU");(void)printf("1. Add donor\n2. List donors\n3. Search donor\n4. Add inventory\n5. List inventory\n6. Check availability\n7. Record donation\n8. List donations\n9. List blood requests\n10. Process pending blood request\n11. View emergency alerts\n12. View notifications\n13. Add donation camp\n14. View donation camps\n15. Logout\n");if(UiReadUint32("Enter choice: ",&c)!=BMS_UI_VALUE){continue;}switch(c){case 1U:HANDLE_OPERATION_RESULT(AddDonor(application));break;case 2U:ListDonors(application);break;case 3U:HANDLE_OPERATION_RESULT(SearchDonor(application));break;case 4U:HANDLE_OPERATION_RESULT(AddInventory(application));break;case 5U:ListInventory(application);break;case 6U:HANDLE_OPERATION_RESULT(CheckAvailability(application));break;case 7U:HANDLE_OPERATION_RESULT(RecordDonation(application,user));break;case 8U:ListDonations(application);break;case 9U:ListRequests(application);break;case 10U:ProcessNextRequest(application);break;case 11U:ViewEmergencyAlerts(application);break;case 12U:ViewNotifications(application,user);break;case 13U:HANDLE_OPERATION_RESULT(AddDonationCamp(application));break;case 14U:ViewDonationCamps(application);break;case 15U:done=true;break;default:(void)printf("Invalid choice.\n");break;}}return BMS_UI_ROLE_LOGOUT;}

static BmsUiRoleResult_t HospitalStaffMenu(BmsApplication_t *application,const BmsUser_t *user)
{bool done=false;BmsUser_t u;if((application==NULL)||(user==NULL))return BMS_UI_ROLE_LOGOUT;u=*user;while(!done){uint32_t c=0U;(void)AuthenticationFindUserById(&application->authentication,u.userId,&u);PrintTitle("HOSPITAL STAFF MENU");(void)printf("Linked Hospital ID: HOS%06lu\n",(unsigned long)u.hospitalId);(void)printf("1. Register/link hospital\n2. Add/update hospital route\n3. Check blood availability\n4. Create blood request\n5. List blood requests\n6. Create emergency alert\n7. View emergency alerts\n8. View notifications\n9. Logout\n");if(UiReadUint32("Enter choice: ",&c)!=BMS_UI_VALUE){continue;}switch(c){case 1U:if(u.hospitalId!=0U)(void)printf("A hospital is already linked.\n");else HANDLE_OPERATION_RESULT(AddHospital(application,u.userId));break;case 2U:HANDLE_OPERATION_RESULT(AddRoute(application,&u));break;case 3U:HANDLE_OPERATION_RESULT(CheckAvailability(application));break;case 4U:HANDLE_OPERATION_RESULT(CreateBloodRequest(application,&u));break;case 5U:ListRequests(application);break;case 6U:HANDLE_OPERATION_RESULT(CreateEmergencyAlert(application,&u));break;case 7U:ViewEmergencyAlerts(application);break;case 8U:ViewNotifications(application,&u);break;case 9U:done=true;break;default:(void)printf("Invalid choice.\n");break;}}return BMS_UI_ROLE_LOGOUT;}

static BmsUiRoleResult_t DonorMenu(BmsApplication_t *application,
                                           const BmsUser_t *user)
{
    bool finished = false;

    if ((application == NULL) || (user == NULL))
    {
        return BMS_UI_ROLE_LOGOUT;
    }

    while (!finished)
    {
        uint32_t choice = 0U;
        PrintTitle("DONOR MENU");
        (void)printf("1. Complete donor profile\n");
        (void)printf("2. View/search profile\n");
        (void)printf("3. View donation history\n");
        (void)printf("4. View donation camps\n");
        (void)printf("5. View notifications\n");
        (void)printf("6. Change password\n");
        (void)printf("7. Logout\n");

        if (UiReadUint32("Enter choice: ", &choice) != BMS_UI_VALUE)
        {
            (void)printf("Invalid choice.\n");
            continue;
        }

        switch (choice)
        {
            case 1U: HANDLE_OPERATION_RESULT(AddDonor(application)); break;
            case 2U: HANDLE_OPERATION_RESULT(SearchDonor(application)); break;
            case 3U: ListDonations(application); break;
            case 4U: ViewDonationCamps(application); break;
            case 5U: ViewNotifications(application, user); break;
            case 6U: HANDLE_OPERATION_RESULT(ChangePassword(application, user)); break;
            case 7U: finished = true; break;
            default:
                (void)printf("Invalid choice. Enter 1 to 7.\n");
                break;
        }
    }

    return BMS_UI_ROLE_LOGOUT;
}

/* Generic field-state pattern used by all forms below. */
#define HANDLE_FIELD_RESULT(result_, field_)                                \
    do                                                                       \
    {                                                                        \
        if ((result_) == BMS_UI_VALUE)                                       \
        {                                                                    \
            ++(field_);                                                      \
        }                                                                    \
        else if ((result_) == BMS_UI_PREVIOUS)                               \
        {                                                                    \
            (void)MoveToPreviousField(&(field_));                            \
        }                                                                    \
        else if (((result_) == BMS_UI_MENU) ||                               \
                 ((result_) == BMS_UI_MAIN))                                 \
        {                                                                    \
            return MapNavigation(result_);                                   \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            (void)printf("Invalid input. Please try again.\n");              \
        }                                                                    \
    } while (false)

static BmsUiOperationResult_t RegisterUser(BmsApplication_t *application)
{
    BmsUser_t user;
    char password[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    char confirmPassword[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    char recoveryCode[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    uint32_t roleChoice = 0U;
    uint32_t field = 0U;
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;

    if (application == NULL)
    {
        return BMS_UI_OPERATION_MENU;
    }

    (void)memset(&user, 0, sizeof(user));
    user.status = BMS_USER_STATUS_ACTIVE;
    user.isActive = true;
    status = AuthenticationGetNextUserId(&application->authentication, &user.userId);
    if (status != BMS_STATUS_OK)
    {
        PrintStatus("Generate user ID", status);
        return BMS_UI_OPERATION_DONE;
    }

    PrintTitle("CREATE PRIVILEGED SYSTEM USER");
    (void)printf("Only an authenticated administrator can create these roles.\n");
    PrintNavigationHelp();

    while (field < 4U)
    {
        BmsUiInputResult_t inputResult = BMS_UI_INVALID;
        switch (field)
        {
            case 0U:
                inputResult = UiReadText("Username: ", user.username,
                                         sizeof(user.username));
                if ((inputResult == BMS_UI_VALUE) && (!ValidateUsername(user.username)))
                {
                    (void)printf("Invalid username.\n");
                    inputResult = BMS_UI_INVALID;
                }
                break;
            case 1U:
                inputResult = UiReadText("Password: ", password, sizeof(password));
                if ((inputResult == BMS_UI_VALUE) && (!ValidatePassword(password)))
                {
                    (void)printf("Password needs 8+ characters with uppercase, lowercase, number and special character.\n");
                    inputResult = BMS_UI_INVALID;
                }
                break;
            case 2U:
                inputResult = UiReadText("Confirm password: ", confirmPassword,
                                         sizeof(confirmPassword));
                if ((inputResult == BMS_UI_VALUE) &&
                    (strcmp(password, confirmPassword) != 0))
                {
                    (void)printf("Passwords do not match.\n");
                    inputResult = BMS_UI_INVALID;
                }
                break;
            case 3U:
                (void)printf("1. Administrator\n2. Blood Bank Staff\n");
                inputResult = UiReadUint32("Select role: ", &roleChoice);
                if ((inputResult == BMS_UI_VALUE) &&
                    (roleChoice != 1U) && (roleChoice != 2U))
                {
                    (void)printf("Invalid role.\n");
                    inputResult = BMS_UI_INVALID;
                }
                else if (inputResult == BMS_UI_VALUE)
                {
                    user.role = (roleChoice == 1U) ? BMS_ROLE_ADMIN :
                                                     BMS_ROLE_BLOOD_BANK_STAFF;
                }
                break;
            default:
                break;
        }
        HANDLE_FIELD_RESULT(inputResult, field);
    }

    status = AuthenticationRegisterUser(&application->authentication, &user, password);
    if (status == BMS_STATUS_OK)
    {
        (void)snprintf(recoveryCode, sizeof(recoveryCode), "BMS-%lu-%s",
                       (unsigned long)user.userId, user.username);
        status = AuthenticationSetRecoveryCode(&application->authentication,
                                               user.userId, recoveryCode);
    }
    if (status == BMS_STATUS_OK)
    {
        status = AuthenticationSave(&application->authentication);
    }

    PrintStatus("Create system user", status);
    if (status == BMS_STATUS_OK)
    {
        (void)printf("User ID: %lu\nRecovery code: %s\n",
                     (unsigned long)user.userId, recoveryCode);
    }

    UtilitySecureZero(password, sizeof(password));
    UtilitySecureZero(confirmPassword, sizeof(confirmPassword));
    UtilitySecureZero(recoveryCode, sizeof(recoveryCode));
    return BMS_UI_OPERATION_DONE;
}

static BmsUiOperationResult_t PublicRegister(BmsApplication_t *application)
{
    BmsUser_t user;
    char password[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    char confirmPassword[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    char recoveryCode[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    uint32_t roleChoice = 0U;
    uint32_t field = 0U;
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;

    if (application == NULL)
    {
        return BMS_UI_OPERATION_MENU;
    }

    (void)memset(&user, 0, sizeof(user));
    status = AuthenticationGetNextUserId(&application->authentication, &user.userId);
    if (status != BMS_STATUS_OK)
    {
        PrintStatus("Generate user ID", status);
        return BMS_UI_OPERATION_DONE;
    }

    PrintTitle("PUBLIC REGISTRATION");
    (void)printf("Public users cannot create Administrator or Blood Bank Staff accounts.\n");
    PrintNavigationHelp();

    while (field < 4U)
    {
        BmsUiInputResult_t inputResult = BMS_UI_INVALID;
        switch (field)
        {
            case 0U:
                inputResult = UiReadText("Username: ", user.username,
                                         sizeof(user.username));
                if ((inputResult == BMS_UI_VALUE) && (!ValidateUsername(user.username)))
                {
                    (void)printf("Invalid username. Use 4-%u letters, numbers, _ or .\n",
                                 BMS_MAX_USERNAME_LENGTH);
                    inputResult = BMS_UI_INVALID;
                }
                break;
            case 1U:
                inputResult = UiReadText("Password: ", password, sizeof(password));
                if ((inputResult == BMS_UI_VALUE) && (!ValidatePassword(password)))
                {
                    (void)printf("Password needs 8+ characters with uppercase, lowercase, number and special character.\n");
                    inputResult = BMS_UI_INVALID;
                }
                break;
            case 2U:
                inputResult = UiReadText("Confirm password: ", confirmPassword,
                                         sizeof(confirmPassword));
                if ((inputResult == BMS_UI_VALUE) &&
                    (strcmp(password, confirmPassword) != 0))
                {
                    (void)printf("Passwords do not match.\n");
                    inputResult = BMS_UI_INVALID;
                }
                break;
            case 3U:
                (void)printf("1. Donor\n2. Hospital Staff\n");
                inputResult = UiReadUint32("Choose role: ", &roleChoice);
                if ((inputResult == BMS_UI_VALUE) &&
                    (roleChoice != 1U) && (roleChoice != 2U))
                {
                    (void)printf("Only Donor or Hospital Staff can self-register.\n");
                    inputResult = BMS_UI_INVALID;
                }
                else if (inputResult == BMS_UI_VALUE)
                {
                    if (roleChoice == 1U)
                    {
                        user.role = BMS_ROLE_DONOR;
                        user.status = BMS_USER_STATUS_ACTIVE;
                        user.isActive = true;
                    }
                    else
                    {
                        user.role = BMS_ROLE_HOSPITAL_STAFF;
                        user.status = BMS_USER_STATUS_PENDING;
                        user.isActive = false;
                    }
                }
                break;
            default:
                break;
        }
        HANDLE_FIELD_RESULT(inputResult, field);
    }

    status = AuthenticationRegisterUser(&application->authentication, &user, password);
    if (status == BMS_STATUS_OK)
    {
        (void)snprintf(recoveryCode, sizeof(recoveryCode), "BMS-%lu-%s",
                       (unsigned long)user.userId, user.username);
        status = AuthenticationSetRecoveryCode(&application->authentication,
                                               user.userId, recoveryCode);
    }
    if (status == BMS_STATUS_OK)
    {
        status = AuthenticationSave(&application->authentication);
    }

    PrintStatus("Register account", status);
    if (status == BMS_STATUS_OK)
    {
        (void)printf("User ID: %lu\nRecovery code: %s\n",
                     (unsigned long)user.userId, recoveryCode);
        if (user.status == BMS_USER_STATUS_PENDING)
        {
            (void)printf("Hospital Staff account is pending administrator approval.\n");
        }
    }

    UtilitySecureZero(password, sizeof(password));
    UtilitySecureZero(confirmPassword, sizeof(confirmPassword));
    UtilitySecureZero(recoveryCode, sizeof(recoveryCode));
    return BMS_UI_OPERATION_DONE;
}

static BmsUiOperationResult_t ForgotPassword(BmsApplication_t *application)
{
    char username[BMS_MAX_USERNAME_LENGTH + 1U] = { '\0' };
    char recoveryCode[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    char password[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    char confirmPassword[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    uint32_t field = 0U;
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;

    if (application == NULL)
    {
        return BMS_UI_OPERATION_MENU;
    }

    PrintTitle("FORGOT PASSWORD");
    PrintNavigationHelp();

    while (field < 4U)
    {
        BmsUiInputResult_t inputResult = BMS_UI_INVALID;
        switch (field)
        {
            case 0U:
                inputResult = UiReadText("Username: ", username, sizeof(username));
                break;
            case 1U:
                inputResult = UiReadText("Recovery code: ", recoveryCode,
                                         sizeof(recoveryCode));
                break;
            case 2U:
                inputResult = UiReadText("New password: ", password,
                                         sizeof(password));
                if ((inputResult == BMS_UI_VALUE) && (!ValidatePassword(password)))
                {
                    (void)printf("Password needs 8+ characters with uppercase, lowercase, number and special character.\n");
                    inputResult = BMS_UI_INVALID;
                }
                break;
            case 3U:
                inputResult = UiReadText("Confirm new password: ", confirmPassword,
                                         sizeof(confirmPassword));
                if ((inputResult == BMS_UI_VALUE) &&
                    (strcmp(password, confirmPassword) != 0))
                {
                    (void)printf("Passwords do not match.\n");
                    inputResult = BMS_UI_INVALID;
                }
                break;
            default:
                break;
        }
        HANDLE_FIELD_RESULT(inputResult, field);
    }

    status = AuthenticationResetPassword(&application->authentication,
                                         username, recoveryCode, password);
    if (status == BMS_STATUS_OK)
    {
        status = AuthenticationSave(&application->authentication);
    }

    if (status == BMS_STATUS_OK)
    {
        (void)printf("Password reset successfully.\n");
    }
    else
    {
        (void)printf("Password reset failed. Verify the username and recovery code.\n");
    }

    UtilitySecureZero(recoveryCode, sizeof(recoveryCode));
    UtilitySecureZero(password, sizeof(password));
    UtilitySecureZero(confirmPassword, sizeof(confirmPassword));
    return BMS_UI_OPERATION_DONE;
}

static BmsUiOperationResult_t ChangePassword(BmsApplication_t *application,
                                              const BmsUser_t *user)
{
    char currentPassword[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    char newPassword[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    char confirmPassword[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    uint32_t field = 0U;
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;

    if ((application == NULL) || (user == NULL))
    {
        return BMS_UI_OPERATION_MENU;
    }

    PrintTitle("CHANGE PASSWORD");
    PrintNavigationHelp();

    while (field < 3U)
    {
        BmsUiInputResult_t inputResult = BMS_UI_INVALID;
        switch (field)
        {
            case 0U:
                inputResult = UiReadText("Current password: ", currentPassword,
                                         sizeof(currentPassword));
                break;
            case 1U:
                inputResult = UiReadText("New password: ", newPassword,
                                         sizeof(newPassword));
                if ((inputResult == BMS_UI_VALUE) && (!ValidatePassword(newPassword)))
                {
                    (void)printf("Password needs 8+ characters with uppercase, lowercase, number and special character.\n");
                    inputResult = BMS_UI_INVALID;
                }
                break;
            case 2U:
                inputResult = UiReadText("Confirm new password: ", confirmPassword,
                                         sizeof(confirmPassword));
                if ((inputResult == BMS_UI_VALUE) &&
                    (strcmp(newPassword, confirmPassword) != 0))
                {
                    (void)printf("Passwords do not match.\n");
                    inputResult = BMS_UI_INVALID;
                }
                break;
            default:
                break;
        }
        HANDLE_FIELD_RESULT(inputResult, field);
    }

    status = AuthenticationChangePassword(&application->authentication,
                                          user->userId, currentPassword, newPassword);
    if (status == BMS_STATUS_OK)
    {
        status = AuthenticationSave(&application->authentication);
    }
    PrintStatus("Change password", status);

    UtilitySecureZero(currentPassword, sizeof(currentPassword));
    UtilitySecureZero(newPassword, sizeof(newPassword));
    UtilitySecureZero(confirmPassword, sizeof(confirmPassword));
    return BMS_UI_OPERATION_DONE;
}

static BmsUiOperationResult_t ReviewPendingUsers(BmsApplication_t *application)
{
    BmsLinkedListNode_t *node = NULL;
    uint32_t userId = 0U;
    uint32_t decision = 0U;
    bool foundPending = false;
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;

    if (application == NULL)
    {
        return BMS_UI_OPERATION_MENU;
    }

    PrintTitle("PENDING REGISTRATIONS");
    node = application->authentication.users.head;
    while (node != NULL)
    {
        if (node->data != NULL)
        {
            const BmsUser_t *pendingUser = (const BmsUser_t *)node->data;
            if (pendingUser->status == BMS_USER_STATUS_PENDING)
            {
                foundPending = true;
                (void)printf("ID:%lu | Username:%s | Role:%s\n",
                             (unsigned long)pendingUser->userId,
                             pendingUser->username,
                             RoleToString(pendingUser->role));
            }
        }
        node = node->next;
    }

    if (!foundPending)
    {
        (void)printf("No pending registrations.\n");
        return BMS_UI_OPERATION_DONE;
    }

    PrintNavigationHelp();
    {
        BmsUiInputResult_t inputResult = UiReadUint32("User ID to review: ", &userId);
        if (inputResult != BMS_UI_VALUE)
        {
            return MapNavigation(inputResult);
        }
    }

    (void)printf("1. Approve\n2. Reject/disable\n");
    {
        BmsUiInputResult_t inputResult = UiReadUint32("Decision: ", &decision);
        if (inputResult != BMS_UI_VALUE)
        {
            return MapNavigation(inputResult);
        }
    }

    if (decision == 1U)
    {
        status = AuthenticationSetUserStatus(&application->authentication,
                                             userId, BMS_USER_STATUS_ACTIVE);
    }
    else if (decision == 2U)
    {
        status = AuthenticationSetUserStatus(&application->authentication,
                                             userId, BMS_USER_STATUS_DISABLED);
    }
    else
    {
        status = BMS_STATUS_INVALID_ARGUMENT;
    }

    if (status == BMS_STATUS_OK)
    {
        status = AuthenticationSave(&application->authentication);
    }
    PrintStatus("Update account status", status);
    return BMS_UI_OPERATION_DONE;
}


static BmsUiOperationResult_t AddDonor(BmsApplication_t *application)
{
    BmsDonor_t donor;
    uint32_t age = 0U;
    uint32_t weight = 0U;
    uint32_t field = 0U;

    if (application == NULL)
    {
        return BMS_UI_OPERATION_MENU;
    }

    (void)memset(&donor, 0, sizeof(donor));
    donor.donorId = NextDonorId(application);
    donor.isActive = true;
    PrintTitle("ADD DONOR");
    PrintNavigationHelp();

    while (field < 8U)
    {
        BmsUiInputResult_t result = BMS_UI_INVALID;
        switch (field)
        {
            case 0U:
                result = UiReadText("Name: ", donor.name, sizeof(donor.name));
                break;
            case 1U:
                result = UiReadUint32("Age: ", &age);
                if ((result == BMS_UI_VALUE) && (age > UINT8_MAX))
                {
                    result = BMS_UI_INVALID;
                }
                else if (result == BMS_UI_VALUE)
                {
                    donor.age = (uint8_t)age;
                }
                break;
            case 2U:
                result = UiReadUint32("Weight in kg: ", &weight);
                if ((result == BMS_UI_VALUE) && (weight > UINT16_MAX))
                {
                    result = BMS_UI_INVALID;
                }
                else if (result == BMS_UI_VALUE)
                {
                    donor.weightKg = (uint16_t)weight;
                }
                break;
            case 3U:
                result = UiReadBloodGroup(&donor.bloodGroup);
                break;
            case 4U:
                result = UiReadText("Phone: ", donor.phone, sizeof(donor.phone));
                if ((result == BMS_UI_VALUE) && !ValidatePhoneNumber(donor.phone)) { (void)printf("Invalid phone. Enter exactly 10 digits.\n"); result=BMS_UI_INVALID; }
                break;
            case 5U:
                result = UiReadText("Email: ", donor.email, sizeof(donor.email));
                if ((result == BMS_UI_VALUE) && !ValidateEmail(donor.email)) { (void)printf("Invalid email address.\n"); result=BMS_UI_INVALID; }
                break;
            case 6U:
                result = UiReadText("Address: ", donor.address,
                                    sizeof(donor.address));
                break;
            case 7U:
                result = UiReadYesNo("Eligible to donate? (y/n): ",
                                     &donor.isEligible);
                break;
            default:
                result = BMS_UI_INVALID;
                break;
        }
        HANDLE_FIELD_RESULT(result, field);
    }

    PrintStatus("Add donor", DonorManagementAdd(&application->donors, &donor));
    return BMS_UI_OPERATION_DONE;
}

static void ListDonors(BmsApplication_t *application)
{
    if (application != NULL)
    {
        PrintTitle("DONOR LIST");
        if (application->donors.donors.count == 0U)
        {
            (void)printf("No donors found.\n");
        }
        else
        {
            BmsStatus_t status = DonorManagementTraverse(&application->donors,
                                                          PrintDonorVisitor,
                                                          NULL);
            if (status != BMS_STATUS_OK)
            {
                PrintStatus("List donors", status);
            }
        }
    }
}

static BmsUiOperationResult_t SearchDonor(BmsApplication_t *application)
{
    uint32_t choice = 0U;
    BmsLinkedListNode_t *node = NULL;
    bool found = false;

    if (application == NULL)
    {
        return BMS_UI_OPERATION_MENU;
    }

    PrintTitle("SEARCH DONOR");
    (void)printf("1. Donor ID\n2. Name\n3. Phone number\n4. Blood group\n");
    if (UiReadUint32("Search by: ", &choice) != BMS_UI_VALUE)
    {
        return BMS_UI_OPERATION_DONE;
    }

    if (choice == 1U)
    {
        uint32_t id = 0U;
        BmsDonor_t donor;
        if ((UiReadUint32("Donor ID number (example: 1 for DON000001): ", &id) == BMS_UI_VALUE) &&
            (DonorManagementSearchById(&application->donors, id, &donor) == BMS_STATUS_OK))
        {
            (void)PrintDonorVisitor(&donor, NULL);
        }
        else
        {
            (void)printf("No matching donor found.\n");
        }
        return BMS_UI_OPERATION_DONE;
    }

    if (choice == 4U)
    {
        BmsBloodGroup_t group;
        if (UiReadBloodGroup(&group) != BMS_UI_VALUE)
        {
            return BMS_UI_OPERATION_DONE;
        }
        for (node = application->donors.donors.head; node != NULL; node = node->next)
        {
            BmsDonor_t *donor = (BmsDonor_t *)node->data;
            if ((donor != NULL) && (donor->bloodGroup == group))
            {
                (void)PrintDonorVisitor(donor, NULL);
                found = true;
            }
        }
    }
    else if ((choice == 2U) || (choice == 3U))
    {
        char query[BMS_MAX_NAME_LENGTH + 1U] = { '\0' };
        const char *prompt = (choice == 2U) ? "Name/full or partial: " : "Phone number: ";
        if (UiReadText(prompt, query, sizeof(query)) != BMS_UI_VALUE)
        {
            return BMS_UI_OPERATION_DONE;
        }
        for (node = application->donors.donors.head; node != NULL; node = node->next)
        {
            BmsDonor_t *donor = (BmsDonor_t *)node->data;
            if ((donor != NULL) &&
                (((choice == 2U) && (strstr(donor->name, query) != NULL)) ||
                 ((choice == 3U) && (strcmp(donor->phone, query) == 0))))
            {
                (void)PrintDonorVisitor(donor, NULL);
                found = true;
            }
        }
    }
    else
    {
        (void)printf("Invalid search option.\n");
        return BMS_UI_OPERATION_DONE;
    }

    if (!found)
    {
        (void)printf("No matching donor found.\n");
    }
    return BMS_UI_OPERATION_DONE;
}

static BmsUiOperationResult_t DeleteDonor(BmsApplication_t *application)
{
    BmsDonorId_t donorId = 0U;
    BmsUiInputResult_t inputResult = BMS_UI_INVALID;

    if (application == NULL)
    {
        return BMS_UI_OPERATION_MENU;
    }

    PrintTitle("DELETE DONOR");
    PrintNavigationHelp();
    inputResult = UiReadUint32("Donor ID: ", &donorId);
    if ((inputResult == BMS_UI_MENU) || (inputResult == BMS_UI_MAIN))
    {
        return MapNavigation(inputResult);
    }
    if (inputResult == BMS_UI_VALUE)
    {
        bool confirmed = false;
        inputResult = UiReadYesNo("Confirm deletion? (y/n): ", &confirmed);
        if ((inputResult == BMS_UI_MENU) || (inputResult == BMS_UI_MAIN))
        {
            return MapNavigation(inputResult);
        }
        if ((inputResult == BMS_UI_VALUE) && confirmed)
        {
            PrintStatus("Delete donor",
                        DonorManagementDelete(&application->donors, donorId));
        }
        else
        {
            (void)printf("Deletion cancelled.\n");
        }
    }
    else
    {
        (void)printf("Invalid donor ID.\n");
    }
    return BMS_UI_OPERATION_DONE;
}

static BmsUiOperationResult_t AddHospital(BmsApplication_t *application, BmsUserId_t linkedUserId)
{
    BmsHospital_t hospital; uint32_t field = 0U; BmsStatus_t status;
    if (application == NULL) { return BMS_UI_OPERATION_MENU; }
    (void)memset(&hospital, 0, sizeof(hospital));
    hospital.hospitalId = NextHospitalId(application); hospital.isActive = true;
    PrintTitle("REGISTER HOSPITAL");
    (void)printf("Hospital ID: %lu (auto-generated)\n", (unsigned long)hospital.hospitalId);
    PrintNavigationHelp();
    while (field < 5U)
    {
        BmsUiInputResult_t result = BMS_UI_INVALID;
        switch (field)
        {
            case 0U: result = UiReadText("Hospital name: ", hospital.name, sizeof(hospital.name)); break;
            case 1U: result = UiReadText("Location: ", hospital.location, sizeof(hospital.location)); break;
            case 2U: result = UiReadText("Address: ", hospital.address, sizeof(hospital.address)); break;
            case 3U:
                result = UiReadText("Contact number: ", hospital.contactNumber, sizeof(hospital.contactNumber));
                if ((result == BMS_UI_VALUE) && (!ValidatePhoneNumber(hospital.contactNumber))) { (void)printf("Invalid phone number.\n"); result = BMS_UI_INVALID; }
                break;
            case 4U:
                result = UiReadText("Email: ", hospital.email, sizeof(hospital.email));
                if ((result == BMS_UI_VALUE) && (!ValidateEmail(hospital.email))) { (void)printf("Invalid email address.\n"); result = BMS_UI_INVALID; }
                break;
            default: break;
        }
        HANDLE_FIELD_RESULT(result, field);
    }
    status = HospitalManagementAdd(&application->hospitals, &hospital);
    if ((status == BMS_STATUS_OK) && (linkedUserId != 0U))
    {
        status = AuthenticationSetHospitalId(&application->authentication, linkedUserId, hospital.hospitalId);
        if (status == BMS_STATUS_OK) { status = AuthenticationSave(&application->authentication); }
    }
    PrintStatus("Register hospital", status);
    if (status == BMS_STATUS_OK) { (void)printf("Generated Hospital ID: %lu\n", (unsigned long)hospital.hospitalId); }
    return BMS_UI_OPERATION_DONE;
}

static void ListHospitals(BmsApplication_t *application)
{
    if (application != NULL)
    {
        PrintTitle("HOSPITAL LIST");
        if (application->hospitals.hospitals.count == 0U)
        {
            (void)printf("No hospitals found.\n");
        }
        else
        {
            BmsStatus_t status = HospitalManagementTraverse(&application->hospitals,
                                                             PrintHospitalVisitor,
                                                             NULL);
            if (status != BMS_STATUS_OK)
            {
                PrintStatus("List hospitals", status);
            }
        }
    }
}

static BmsUiOperationResult_t AddRoute(BmsApplication_t *application,const BmsUser_t *user)
{BmsHospitalId_t destination=0U;uint32_t distance=0U;if((application==NULL)||(user==NULL)||user->hospitalId==0U){(void)printf("Register/link your hospital first.\n");return BMS_UI_OPERATION_DONE;}ListHospitals(application);(void)printf("Source hospital: HOS%06lu (auto-linked)\n",(unsigned long)user->hospitalId);if(UiReadUint32("Destination hospital ID number: ",&destination)!=BMS_UI_VALUE)return BMS_UI_OPERATION_DONE;if(UiReadUint32("Distance in km: ",&distance)!=BMS_UI_VALUE)return BMS_UI_OPERATION_DONE;PrintStatus("Add route",HospitalManagementAddRoute(&application->hospitals,user->hospitalId,destination,distance));return BMS_UI_OPERATION_DONE;}

static BmsUiOperationResult_t AddInventory(BmsApplication_t *application)
{
    BmsBloodInventory_t record;
    uint32_t field = 0U;

    if (application == NULL)
    {
        return BMS_UI_OPERATION_MENU;
    }

    (void)memset(&record, 0, sizeof(record));
    record.bloodId = NextBloodId(application);
    record.isAvailable = true;
    PrintTitle("ADD BLOOD INVENTORY");
    PrintNavigationHelp();

    while (field < 3U)
    {
        BmsUiInputResult_t result = BMS_UI_INVALID;
        switch (field)
        {
            case 0U:
                result = UiReadBloodGroup(&record.bloodGroup);
                break;
            case 1U:
                result = UiReadUint32("Units: ", &record.units);
                break;
            case 2U:
                result = UiReadDate("Collection date (YYYY-MM-DD): ",
                                    &record.collectionDate);
                break;
            default:
                result = BMS_UI_INVALID;
                break;
        }
        HANDLE_FIELD_RESULT(result, field);
    }

    {
        const BmsStatus_t dateStatus =
            BmsDateAddDays(&record.collectionDate,
                           BMS_RBC_SHELF_LIFE_DAYS,
                           &record.expiryDate);
        if (dateStatus != BMS_STATUS_OK)
        {
            PrintStatus("Calculate expiry date", dateStatus);
            return BMS_UI_OPERATION_DONE;
        }
    }

    (void)printf("Expiry date automatically calculated: %04u-%02u-%02u\n",
                 record.expiryDate.year,
                 record.expiryDate.month,
                 record.expiryDate.day);
    PrintStatus("Add inventory",
                BloodInventoryAddStock(&application->inventory, &record));
    return BMS_UI_OPERATION_DONE;
}

static void ListInventory(BmsApplication_t *application)
{
    if (application != NULL)
    {
        PrintTitle("BLOOD INVENTORY");
        if (application->inventory.inventory.count == 0U)
        {
            (void)printf("No blood inventory records.\n");
        }
        else
        {
            BmsStatus_t status = BloodInventoryTraverse(&application->inventory,
                                                         PrintInventoryVisitor,
                                                         NULL);
            if (status != BMS_STATUS_OK)
            {
                PrintStatus("List inventory", status);
            }
        }
    }
}

static BmsUiOperationResult_t CheckAvailability(BmsApplication_t *application)
{
    BmsBloodGroup_t bloodGroup = BMS_BLOOD_GROUP_INVALID;
    uint32_t units = 0U;
    BmsUiInputResult_t inputResult = BMS_UI_INVALID;

    if (application == NULL)
    {
        return BMS_UI_OPERATION_MENU;
    }

    PrintTitle("CHECK BLOOD AVAILABILITY");
    PrintNavigationHelp();
    inputResult = UiReadBloodGroup(&bloodGroup);
    if ((inputResult == BMS_UI_MENU) || (inputResult == BMS_UI_MAIN))
    {
        return MapNavigation(inputResult);
    }
    if (inputResult == BMS_UI_VALUE)
    {
        BmsStatus_t status = BloodInventoryGetAvailableUnits(&application->inventory,
                                                              bloodGroup,
                                                              &units);
        PrintStatus("Check availability", status);
        if (status == BMS_STATUS_OK)
        {
            (void)printf("Available %s units: %u\n",
                         BloodGroupToString(bloodGroup),
                         units);
        }
    }
    else
    {
        (void)printf("Invalid blood group.\n");
    }
    return BMS_UI_OPERATION_DONE;
}

static BmsUiOperationResult_t RecordDonation(BmsApplication_t *application,
                                                        const BmsUser_t *user)
{
    BmsDonation_t donation;
    uint32_t field = 0U;

    if ((application == NULL) || (user == NULL))
    {
        return BMS_UI_OPERATION_MENU;
    }

    (void)memset(&donation, 0, sizeof(donation));
    donation.donationId = NextDonationId(application);
    donation.collectionHospitalId = (BmsHospitalId_t)user->userId;
    PrintTitle("RECORD DONATION");
    (void)printf("Recorded by Blood Bank Staff ID: USR%06lu (auto-linked)\n",
                 (unsigned long)user->userId);
    PrintNavigationHelp();

    while (field < 4U)
    {
        BmsUiInputResult_t result = BMS_UI_INVALID;
        switch (field)
        {
            case 0U:
                result = UiReadUint32("Donor ID: ", &donation.donorId);
                break;
            case 1U:
                result = UiReadBloodGroup(&donation.bloodGroup);
                break;
            case 2U:
                result = UiReadUint32("Units donated: ", &donation.units);
                break;
            case 3U:
                result = UiReadDate("Donation date (YYYY-MM-DD): ",
                                    &donation.donationDate);
                break;
            default:
                result = BMS_UI_INVALID;
                break;
        }
        HANDLE_FIELD_RESULT(result, field);
    }

    PrintStatus("Record donation",
                DonationManagementRecord(&application->donations,
                                         &application->donors,
                                         &application->inventory,
                                         &donation));
    return BMS_UI_OPERATION_DONE;
}

static void ListDonations(BmsApplication_t *application)
{
    if (application != NULL)
    {
        PrintTitle("DONATION HISTORY");
        if (application->donations.donations.count == 0U)
        {
            (void)printf("No donation records.\n");
        }
        else
        {
            BmsStatus_t status = DonationManagementTraverse(&application->donations,
                                                             PrintDonationVisitor,
                                                             NULL);
            if (status != BMS_STATUS_OK)
            {
                PrintStatus("List donations", status);
            }
        }
    }
}

static BmsUiOperationResult_t CreateBloodRequest(BmsApplication_t *application, const BmsUser_t *requester)
{
    BmsBloodRequest_t request; BmsHospital_t hospital; uint32_t priority = 0U; uint32_t field = 0U;
    if ((application == NULL) || (requester == NULL)) { return BMS_UI_OPERATION_MENU; }
    if (requester->hospitalId == 0U) { (void)printf("No hospital is linked. Register a hospital first.\n"); return BMS_UI_OPERATION_DONE; }
    if (HospitalManagementSearchById(&application->hospitals, requester->hospitalId, &hospital) != BMS_STATUS_OK)
    { (void)printf("Linked hospital record not found.\n"); return BMS_UI_OPERATION_DONE; }
    (void)memset(&request, 0, sizeof(request));
    request.requestId = NextRequestId(application); request.requesterId = requester->userId;
    request.requesterHospitalId = requester->hospitalId; request.status = BMS_REQUEST_STATUS_PENDING;
    PrintTitle("CREATE BLOOD REQUEST");
    (void)printf("Request ID: %lu (auto-generated)\n", (unsigned long)request.requestId);
    (void)printf("Hospital: %s [ID %lu] (auto-linked)\n", hospital.name, (unsigned long)hospital.hospitalId);
    PrintNavigationHelp();
    while (field < 4U)
    {
        BmsUiInputResult_t result = BMS_UI_INVALID;
        switch (field)
        {
            case 0U: result = UiReadBloodGroup(&request.bloodGroup); break;
            case 1U: result = UiReadUint32("Required units: ", &request.requestedUnits); if ((result == BMS_UI_VALUE) && (request.requestedUnits == 0U)) { result = BMS_UI_INVALID; } break;
            case 2U:
                (void)printf("0.Low  1.Normal  2.High  3.Emergency\n"); result = UiReadUint32("Priority: ", &priority);
                if ((result == BMS_UI_VALUE) && (priority <= 3U)) { request.priority = (BmsPriority_t)priority; } else if (result == BMS_UI_VALUE) { result = BMS_UI_INVALID; }
                break;
            case 3U: result = UiReadDate("Request date (YYYY-MM-DD): ", &request.requestDate); break;
            default: break;
        }
        HANDLE_FIELD_RESULT(result, field);
    }
    PrintStatus("Create request", BloodRequestManagementCreate(&application->requests, &request));
    (void)printf("Generated Request ID: %lu\n", (unsigned long)request.requestId);
    return BMS_UI_OPERATION_DONE;
}

static void ListRequests(BmsApplication_t *application)
{
    if (application != NULL)
    {
        PrintTitle("BLOOD REQUESTS");
        if (application->requests.requests.count == 0U)
        {
            (void)printf("No blood requests.\n");
        }
        else
        {
            BmsStatus_t status = BloodRequestManagementTraverse(&application->requests,
                                                                 PrintRequestVisitor,
                                                                 NULL);
            if (status != BMS_STATUS_OK)
            {
                PrintStatus("List requests", status);
            }
        }
    }
}

static void ProcessNextRequest(BmsApplication_t *application)
{
    if (application != NULL)
    {
        BmsBloodRequest_t request;
        BmsStatus_t status;
        (void)memset(&request, 0, sizeof(request));
        status = BloodRequestManagementProcessNext(&application->requests,
                                                    &application->inventory,
                                                    &request);
        PrintStatus("Process next request", status);
        if ((status == BMS_STATUS_OK) ||
            (status == BMS_STATUS_INSUFFICIENT_STOCK))
        {
            BmsStatus_t printStatus = PrintRequestVisitor(&request, NULL);
            if (printStatus != BMS_STATUS_OK)
            {
                PrintStatus("Print request", printStatus);
            }
        }
    }
}

static BmsStatus_t NotifyBloodBankStaff(
    BmsApplication_t *application,
    const BmsEmergencyAlert_t *alert)
{
    BmsLinkedListNode_t *node;
    BmsStatus_t status = BMS_STATUS_NOT_FOUND;
    bool recipientFound = false;

    if ((application == NULL) || (alert == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    for (node = application->authentication.users.head;
         node != NULL;
         node = node->next)
    {
        const BmsUser_t *recipient = (const BmsUser_t *)node->data;
        if ((recipient != NULL) &&
            (recipient->role == BMS_ROLE_BLOOD_BANK_STAFF) &&
            (recipient->status == BMS_USER_STATUS_ACTIVE) &&
            recipient->isActive)
        {
            BmsNotification_t notification;
            (void)memset(&notification, 0, sizeof(notification));
            notification.notificationId = NextNotificationId(application);
            notification.recipientUserId = recipient->userId;
            notification.channel = BMS_NOTIFICATION_CHANNEL_CONSOLE;
            notification.priority = BMS_PRIORITY_EMERGENCY;
            (void)snprintf(
                notification.message,
                sizeof(notification.message),
                "Emergency request REQ%06lu / ALT%06lu from HOS%06lu: "
                "%u units %s - %.120s",
                (unsigned long)alert->requestId,
                (unsigned long)alert->alertId,
                (unsigned long)alert->sourceHospitalId,
                alert->requiredUnits,
                BloodGroupToString(alert->bloodGroup),
                alert->message);

            status = NotificationManagementEnqueue(&application->notifications,
                                                    &notification);
            if (status != BMS_STATUS_OK)
            {
                return status;
            }
            recipientFound = true;
        }
    }

    return recipientFound ? BMS_STATUS_OK : BMS_STATUS_NOT_FOUND;
}

static BmsUiOperationResult_t CreateEmergencyAlert(
    BmsApplication_t *application,
    const BmsUser_t *user)
{
    BmsEmergencyAlert_t alert;
    BmsBloodRequest_t request;
    uint32_t field = 0U;
    BmsStatus_t status;

    if ((application == NULL) || (user == NULL) || (user->hospitalId == 0U))
    {
        (void)printf("Register/link your hospital first.\n");
        return BMS_UI_OPERATION_DONE;
    }

    (void)memset(&alert, 0, sizeof(alert));
    (void)memset(&request, 0, sizeof(request));

    alert.alertId = NextAlertId(application);
    alert.requestId = NextRequestId(application);
    alert.sourceHospitalId = user->hospitalId;
    alert.createdByUserId = user->userId;
    alert.priority = BMS_PRIORITY_EMERGENCY;

    PrintTitle("CREATE EMERGENCY ALERT");
    (void)printf("Alert ID: ALT%06lu | Request ID: REQ%06lu | "
                 "Source Hospital: HOS%06lu (auto-linked)\n",
                 (unsigned long)alert.alertId,
                 (unsigned long)alert.requestId,
                 (unsigned long)alert.sourceHospitalId);
    PrintNavigationHelp();

    while (field < 3U)
    {
        BmsUiInputResult_t result = BMS_UI_INVALID;
        switch (field)
        {
            case 0U:
                result = UiReadBloodGroup(&alert.bloodGroup);
                break;
            case 1U:
                result = UiReadUint32("Required units: ",
                                      &alert.requiredUnits);
                if ((result == BMS_UI_VALUE) &&
                    (alert.requiredUnits == 0U))
                {
                    result = BMS_UI_INVALID;
                }
                break;
            case 2U:
                result = UiReadText("Emergency reason/message: ",
                                    alert.message,
                                    sizeof(alert.message));
                break;
            default:
                result = BMS_UI_INVALID;
                break;
        }
        HANDLE_FIELD_RESULT(result, field);
    }

    request.requestId = alert.requestId;
    request.requesterHospitalId = alert.sourceHospitalId;
    request.requesterId = alert.createdByUserId;
    request.bloodGroup = alert.bloodGroup;
    request.requestedUnits = alert.requiredUnits;
    request.priority = BMS_PRIORITY_EMERGENCY;
    request.status = BMS_REQUEST_STATUS_PENDING;

    status = BloodRequestManagementCreate(&application->requests, &request);
    PrintStatus("Create pending emergency request", status);
    if (status != BMS_STATUS_OK)
    {
        return BMS_UI_OPERATION_DONE;
    }

    status = EmergencyAlertManagementCreate(&application->alerts, &alert);
    PrintStatus("Create emergency alert", status);
    if (status != BMS_STATUS_OK)
    {
        return BMS_UI_OPERATION_DONE;
    }

    status = NotifyBloodBankStaff(application, &alert);
    PrintStatus("Notify blood bank staff", status);
    if (status == BMS_STATUS_OK)
    {
        (void)printf("Emergency request sent only to Blood Bank Staff.\n");
    }

    return BMS_UI_OPERATION_DONE;
}

static void ViewNotifications(BmsApplication_t *application,const BmsUser_t *user)
{BmsLinkedListNode_t*n;if((application==NULL)||(user==NULL))return;PrintTitle("NOTIFICATIONS");for(n=application->notifications.history.head;n;n=n->next){BmsNotification_t*x=(BmsNotification_t*)n->data;if((x->recipientUserId==0U||x->recipientUserId==user->userId)&&(x->recipientHospitalId==0U||x->recipientHospitalId==user->hospitalId))(void)printf("NOT%06u | %s\n",x->notificationId,x->message);}}
static void ViewEmergencyAlerts(BmsApplication_t *application){BmsLinkedListNode_t*n;if(application==NULL)return;PrintTitle("EMERGENCY ALERTS");if(application->alerts.alertHistory.count==0U){(void)printf("No emergency alerts.\n");return;}for(n=application->alerts.alertHistory.head;n;n=n->next){BmsEmergencyAlert_t*a=(BmsEmergencyAlert_t*)n->data;(void)printf("ALT%06u | HOS%06u | %s | Units:%u | %s | %s\n",a->alertId,a->sourceHospitalId,BloodGroupToString(a->bloodGroup),a->requiredUnits,a->resolved?"RESOLVED":"ACTIVE",a->message);}}
static BmsStatus_t PrintCamp(const BmsDonationCamp_t*c,void*x){(void)x;if(c==NULL)return BMS_STATUS_INVALID_ARGUMENT;(void)printf("CAMP%06u | %s | %04u-%02u-%02u %s | %s, %s | Contact:%s | %s\n",c->campId,c->name,c->date.year,c->date.month,c->date.day,c->time,c->venue,c->city,c->contactNumber,c->isActive?"ACTIVE":"INACTIVE");return BMS_STATUS_OK;}
static void ViewDonationCamps(BmsApplication_t *application){if(application==NULL)return;PrintTitle("BLOOD DONATION CAMPS");if(application->camps.camps.count==0U)(void)printf("No donation camps announced.\n");else(void)DonationCampManagementTraverse(&application->camps,PrintCamp,NULL);}
static BmsUiOperationResult_t AddDonationCamp(BmsApplication_t *application){BmsDonationCamp_t c;uint32_t field=0U;if(application==NULL)return BMS_UI_OPERATION_MENU;(void)memset(&c,0,sizeof(c));c.campId=NextCampId(application);c.isActive=true;PrintTitle("ADD BLOOD DONATION CAMP");(void)printf("Camp ID: CAMP%06lu (auto-generated)\n",(unsigned long)c.campId);while(field<8U){BmsUiInputResult_t r=BMS_UI_INVALID;switch(field){case 0U:r=UiReadText("Camp name: ",c.name,sizeof(c.name));break;case 1U:r=UiReadDate("Camp date (YYYY-MM-DD): ",&c.date);break;case 2U:r=UiReadText("Time: ",c.time,sizeof(c.time));break;case 3U:r=UiReadText("Venue: ",c.venue,sizeof(c.venue));break;case 4U:r=UiReadText("City: ",c.city,sizeof(c.city));break;case 5U:r=UiReadText("Organizer: ",c.organizer,sizeof(c.organizer));break;case 6U:r=UiReadText("Contact number: ",c.contactNumber,sizeof(c.contactNumber));if((r==BMS_UI_VALUE)&&!ValidatePhoneNumber(c.contactNumber)){(void)printf("Invalid phone. Enter exactly 10 digits.\n");r=BMS_UI_INVALID;}break;case 7U:r=UiReadText("Contact email: ",c.email,sizeof(c.email));if((r==BMS_UI_VALUE)&&!ValidateEmail(c.email)){(void)printf("Invalid email.\n");r=BMS_UI_INVALID;}break;default:break;}HANDLE_FIELD_RESULT(r,field);}PrintStatus("Add donation camp",DonationCampManagementAdd(&application->camps,&c));{BmsNotification_t n;(void)memset(&n,0,sizeof(n));n.notificationId=c.campId+100000U;n.priority=BMS_PRIORITY_NORMAL;n.channel=BMS_NOTIFICATION_CHANNEL_BROADCAST;(void)snprintf(n.message,sizeof(n.message),"Blood donation camp: %.50s on %04u-%02u-%02u at %.80s, %.40s",c.name,c.date.year,c.date.month,c.date.day,c.venue,c.city);(void)NotificationManagementEnqueue(&application->notifications,&n);}return BMS_UI_OPERATION_DONE;}

static void ShowSummary(BmsApplication_t *application)
{
    if (application != NULL)
    {
        BmsSummaryReport_t summary;
        BmsStatus_t status;
        (void)memset(&summary, 0, sizeof(summary));
        status = ReportManagementBuildSummary(&application->donors,
                                               &application->hospitals,
                                               &application->inventory,
                                               &application->requests,
                                               &application->donations,
                                               &application->alerts,
                                               &summary);
        if (status == BMS_STATUS_OK)
        {
            status = ReportManagementPrintSummary(&summary);
        }
        PrintStatus("Summary report", status);
    }
}

static void GenerateReports(BmsApplication_t *application)
{
    if (application != NULL)
    {
        BmsStatus_t status = FileManagementEnsureDataDirectory();
        PrintStatus("Prepare report directory", status);
        if (status == BMS_STATUS_OK)
        {
            PrintStatus("Donor report",
                        ReportManagementGenerateDonorReport(
                            &application->donors,
                            "data/donor_report.txt"));
            PrintStatus("Hospital report",
                        ReportManagementGenerateHospitalReport(
                            &application->hospitals,
                            "data/hospital_report.txt"));
            PrintStatus("Inventory report",
                        ReportManagementGenerateInventoryReport(
                            &application->inventory,
                            "data/inventory_report.txt"));
            PrintStatus("Donation report",
                        ReportManagementGenerateDonationReport(
                            &application->donations,
                            "data/donation_report.txt"));
            PrintStatus("Request report",
                        ReportManagementGenerateBloodRequestReport(
                            &application->requests,
                            "data/request_report.txt"));
            PrintStatus("Emergency report",
                        ReportManagementGenerateEmergencyAlertReport(
                            &application->alerts,
                            "data/emergency_report.txt"));
        }
    }
}

static BmsStatus_t PrintDonorVisitor(const BmsDonor_t *donor, void *context)
{
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;
    (void)context;

    if (donor != NULL)
    {
        (void)printf("DON%06u | %s | Age:%u | Weight:%u kg | Group:%s | "
                     "Eligible:%s | Phone:%s\n",
                     donor->donorId,
                     donor->name,
                     donor->age,
                     donor->weightKg,
                     BloodGroupToString(donor->bloodGroup),
                     donor->isEligible ? "Yes" : "No",
                     donor->phone);
        status = BMS_STATUS_OK;
    }
    return status;
}

static BmsStatus_t PrintHospitalVisitor(const BmsHospital_t *hospital,
                                        void *context)
{
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;
    (void)context;

    if (hospital != NULL)
    {
        (void)printf("HOS%06u | %s | %s | Contact:%s | Active:%s\n",
                     hospital->hospitalId,
                     hospital->name,
                     hospital->location,
                     hospital->contactNumber,
                     hospital->isActive ? "Yes" : "No");
        status = BMS_STATUS_OK;
    }
    return status;
}

static BmsStatus_t PrintInventoryVisitor(const BmsBloodInventory_t *record,
                                         void *context)
{
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;
    (void)context;

    if (record != NULL)
    {
        (void)printf("Stock ID:%u | Group:%s | Units:%u | "
                     "Expiry:%04u-%02u-%02u | Available:%s\n",
                     record->bloodId,
                     BloodGroupToString(record->bloodGroup),
                     record->units,
                     record->expiryDate.year,
                     record->expiryDate.month,
                     record->expiryDate.day,
                     record->isAvailable ? "Yes" : "No");
        status = BMS_STATUS_OK;
    }
    return status;
}

static BmsStatus_t PrintDonationVisitor(const BmsDonation_t *donation,
                                        void *context)
{
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;
    (void)context;

    if (donation != NULL)
    {
        (void)printf("Donation ID:%u | Donor:%u | Group:%s | Units:%u | "
                     "Date:%04u-%02u-%02u | RecordedByStaff:%u\n",
                     donation->donationId,
                     donation->donorId,
                     BloodGroupToString(donation->bloodGroup),
                     donation->units,
                     donation->donationDate.year,
                     donation->donationDate.month,
                     donation->donationDate.day,
                     donation->collectionHospitalId);
        status = BMS_STATUS_OK;
    }
    return status;
}

static BmsStatus_t PrintRequestVisitor(const BmsBloodRequest_t *request,
                                       void *context)
{
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;
    (void)context;

    if (request != NULL)
    {
        (void)printf("REQ%06u | HOS%06u | Group:%s | Required:%u | "
                     "Fulfilled:%u | Status:%s | Priority:%u | NeedBy:%04u-%02u-%02u\n",
                     request->requestId,
                     request->requesterHospitalId,
                     BloodGroupToString(request->bloodGroup),
                     request->requestedUnits,
                     request->fulfilledUnits,
                     RequestStatusToString(request->status),
                     (unsigned int)request->priority, request->requiredByDate.year, request->requiredByDate.month, request->requiredByDate.day);
        status = BMS_STATUS_OK;
    }
    return status;
}


