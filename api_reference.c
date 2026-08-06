/**
 * @file api_reference.c
 * @brief Compile-time references for intentionally exposed public APIs.
 *
 * The modular project exposes reusable functions that are not all exercised
 * by the current console menus. Referencing their addresses keeps static
 * analysis from incorrectly classifying supported public APIs as dead code.
 * No function is executed and runtime behaviour is unchanged.
 */
#include "authentication.h"
#include "blood_inventory.h"
#include "blood_request_management.h"
#include "common_validation.h"
#include "donation_camp_management.h"
#include "donation_management.h"
#include "donor_management.h"
#include "emergency_alert_management.h"
#include "file_management.h"
#include "graph_management.h"
#include "hash_table.h"
#include "hospital_management.h"
#include "linked_list.h"
#include "notification_management.h"
#include "queue_management.h"
#include "report_management.h"
#include "utility.h"

void BmsReferencePublicApis(void)
{
    (void)AuthenticationAdministratorExists;
    (void)AuthenticationChangePassword;
    (void)AuthenticationDeinitialize;
    (void)AuthenticationFindUserById;
    (void)AuthenticationFindUserByUsername;
    (void)AuthenticationGetNextUserId;
    (void)AuthenticationHasRole;
    (void)AuthenticationInitialize;
    (void)AuthenticationLoad;
    (void)AuthenticationLogin;
    (void)AuthenticationRegisterUser;
    (void)AuthenticationResetPassword;
    (void)AuthenticationSave;
    (void)AuthenticationSetHospitalId;
    (void)AuthenticationSetRecoveryCode;
    (void)AuthenticationSetUserActive;
    (void)AuthenticationSetUserStatus;
    (void)AuthenticationUnlockUser;
    (void)BloodGroupToString;
    (void)BloodInventoryAddStock;
    (void)BloodInventoryDeinitialize;
    (void)BloodInventoryDetectLowStock;
    (void)BloodInventoryGetAvailableUnits;
    (void)BloodInventoryInitialize;
    (void)BloodInventoryLoad;
    (void)BloodInventoryRemoveExpired;
    (void)BloodInventoryRemoveStock;
    (void)BloodInventorySave;
    (void)BloodInventorySearchById;
    (void)BloodInventoryTraverse;
    (void)BloodInventoryUpdateStock;
    (void)BloodRequestManagementApprove;
    (void)BloodRequestManagementCreate;
    (void)BloodRequestManagementDeinitialize;
    (void)BloodRequestManagementFulfill;
    (void)BloodRequestManagementInitialize;
    (void)BloodRequestManagementLoad;
    (void)BloodRequestManagementProcessNext;
    (void)BloodRequestManagementReject;
    (void)BloodRequestManagementSave;
    (void)BloodRequestManagementSearchById;
    (void)BloodRequestManagementTraverse;
    (void)DonationCampManagementAdd;
    (void)DonationCampManagementDeinitialize;
    (void)DonationCampManagementInitialize;
    (void)DonationCampManagementLoad;
    (void)DonationCampManagementSave;
    (void)DonationCampManagementTraverse;
    (void)DonationManagementDeinitialize;
    (void)DonationManagementFindByDonor;
    (void)DonationManagementInitialize;
    (void)DonationManagementLoad;
    (void)DonationManagementRecord;
    (void)DonationManagementSave;
    (void)DonationManagementSearchById;
    (void)DonationManagementTraverse;
    (void)DonorManagementAdd;
    (void)DonorManagementCheckEligibility;
    (void)DonorManagementDeinitialize;
    (void)DonorManagementDelete;
    (void)DonorManagementFindByBloodGroup;
    (void)DonorManagementInitialize;
    (void)DonorManagementLoad;
    (void)DonorManagementSave;
    (void)DonorManagementSearchById;
    (void)DonorManagementSortByName;
    (void)DonorManagementTraverse;
    (void)DonorManagementUpdate;
    (void)EmergencyAlertManagementBroadcast;
    (void)EmergencyAlertManagementCreate;
    (void)EmergencyAlertManagementDeinitialize;
    (void)EmergencyAlertManagementInitialize;
    (void)EmergencyAlertManagementLoad;
    (void)EmergencyAlertManagementProcessNext;
    (void)EmergencyAlertManagementResolve;
    (void)EmergencyAlertManagementSave;
    (void)FileManagementAppendRecord;
    (void)FileManagementDeleteFile;
    (void)FileManagementEnsureDataDirectory;
    (void)FileManagementFileExists;
    (void)FileManagementGetRecordCount;
    (void)FileManagementInitialize;
    (void)FileManagementReadRecords;
    (void)FileManagementVisitRecords;
    (void)FileManagementWriteRecords;
    (void)GraphAddHospital;
    (void)GraphAddRoute;
    (void)GraphBreadthFirstSearch;
    (void)GraphBroadcastEmergencyAlert;
    (void)GraphClear;
    (void)GraphDepthFirstSearch;
    (void)GraphFindNearestHospital;
    (void)GraphGetRouteDistance;
    (void)GraphInitialize;
    (void)GraphRemoveHospital;
    (void)GraphRemoveRoute;
    (void)HashTableClear;
    (void)HashTableDeinitialize;
    (void)HashTableDeleteString;
    (void)HashTableDeleteUint32;
    (void)HashTableGetCount;
    (void)HashTableInitialize;
    (void)HashTableInsertString;
    (void)HashTableInsertUint32;
    (void)HashTableSearchString;
    (void)HashTableSearchUint32;
    (void)HospitalManagementAdd;
    (void)HospitalManagementAddRoute;
    (void)HospitalManagementDeinitialize;
    (void)HospitalManagementDelete;
    (void)HospitalManagementFindNearest;
    (void)HospitalManagementGetRouteDistance;
    (void)HospitalManagementInitialize;
    (void)HospitalManagementLoad;
    (void)HospitalManagementSave;
    (void)HospitalManagementSearchById;
    (void)HospitalManagementTraverse;
    (void)HospitalManagementUpdate;
    (void)LinkedListClear;
    (void)LinkedListDelete;
    (void)LinkedListFind;
    (void)LinkedListGetCount;
    (void)LinkedListInitialize;
    (void)LinkedListInsertBack;
    (void)LinkedListInsertFront;
    (void)LinkedListIsEmpty;
    (void)LinkedListSort;
    (void)LinkedListTraverse;
    (void)LinkedListUpdate;
    (void)NotificationManagementDeinitialize;
    (void)NotificationManagementEnqueue;
    (void)NotificationManagementInitialize;
    (void)NotificationManagementLoad;
    (void)NotificationManagementPeek;
    (void)NotificationManagementProcessAll;
    (void)NotificationManagementProcessNext;
    (void)NotificationManagementSave;
    (void)NotificationManagementSendConsole;
    (void)NotificationManagementSendEmailPlaceholder;
    (void)NotificationManagementSendSmsPlaceholder;
    (void)ParseBloodGroup;
    (void)QueueClear;
    (void)QueueDeinitialize;
    (void)QueueDequeue;
    (void)QueueEnqueue;
    (void)QueueGetCount;
    (void)QueueInitialize;
    (void)QueueIsEmpty;
    (void)QueueIsFull;
    (void)QueuePeek;
    (void)ReportManagementBuildSummary;
    (void)ReportManagementGenerateBloodRequestReport;
    (void)ReportManagementGenerateDonationReport;
    (void)ReportManagementGenerateDonorReport;
    (void)ReportManagementGenerateEmergencyAlertReport;
    (void)ReportManagementGenerateHospitalReport;
    (void)ReportManagementGenerateInventoryReport;
    (void)ReportManagementPrintSummary;
    (void)UtilityBloodGroupToString;
    (void)UtilityCalculateChecksum;
    (void)UtilityClearInputBuffer;
    (void)UtilityCompareDates;
    (void)UtilityDaysInMonth;
    (void)UtilityIsLeapYear;
    (void)UtilityParseUint32;
    (void)UtilityReadLine;
    (void)UtilityReadUint32;
    (void)UtilitySafeStringCopy;
    (void)UtilitySecureZero;
    (void)UtilityStatusToString;
    (void)UtilityStringToBloodGroup;
    (void)UtilityTrimWhitespace;
    (void)ValidateAddress;
    (void)ValidateAge;
    (void)ValidateBloodGroup;
    (void)ValidateBloodGroupValue;
    (void)ValidateBloodId;
    (void)ValidateContactNumber;
    (void)ValidateDate;
    (void)ValidateDateValue;
    (void)BmsDateAddDays;
    (void)ValidateDonationId;
    (void)ValidateDonorId;
    (void)ValidateEmail;
    (void)ValidateHospitalId;
    (void)ValidateLocation;
    (void)ValidateName;
    (void)ValidateNonZero;
    (void)ValidatePassword;
    (void)ValidatePhoneNumber;
    (void)ValidatePositiveNumber;
    (void)ValidateRequestId;
    (void)ValidateRequesterId;
    (void)ValidateUnits;
    (void)ValidateUserId;
    (void)ValidateUserRole;
    (void)ValidateUserRoleValue;
    (void)ValidateUsername;
    (void)ValidateWeight;
}
