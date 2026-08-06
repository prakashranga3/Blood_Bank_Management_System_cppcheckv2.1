# Requested BMS Workflow Changes

Only the four requested workflows were changed.

1. **Emergency routing**
   - A hospital emergency alert now creates a pending `BmsBloodRequest_t`.
   - Notifications are targeted only to active Blood Bank Staff users.
   - The existing graph-broadcast APIs remain unchanged and available.

2. **Pending emergency processing**
   - Because the emergency alert now creates a pending blood-request record,
     the existing **Process pending blood request** operation can find and
     process it through `BloodRequestManagementProcessNext()`.

3. **Automatic RBC expiry**
   - Added `BMS_RBC_SHELF_LIFE_DAYS` with a value of 45 days.
   - Manual inventory entry asks only for the collection date.
   - The expiry date is calculated using calendar-aware date addition.
   - Inventory created by donation recording uses the same 45-day calculation.

4. **Automatic donation recorder identity**
   - Blood Bank Staff are no longer asked for a collection-hospital ID.
   - The current authenticated Blood Bank Staff user ID is automatically stored
     in the existing collection identity field to preserve the current binary
     data structure and file format.

No graph algorithm, notification-management API, authentication workflow,
hospital workflow, report API, or unrelated menu option was removed or
redesigned.
