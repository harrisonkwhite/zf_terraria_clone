#pragma once

#include "items.h"

struct t_inventory;

struct t_inventory_slot {
    t_item_type_id item_type_id;
    zcl::t_i32 quantity;
};

t_inventory *InventoryCreate(const zcl::t_v2_i size, zcl::t_arena *const arena);

// Returns the quantity that couldn't be added due to the inventory getting filled (0 for all added).
zcl::t_i32 InventoryAdd(t_inventory *const inventory, const t_item_type_id item_type_id, zcl::t_i32 quantity);

// Returns the quantity that couldn't be added due to the slot getting filled (0 for all added).
zcl::t_i32 InventoryAddAt(t_inventory *const inventory, const zcl::t_v2_i slot_pos, const t_item_type_id item_type_id, const zcl::t_i32 quantity);

// Returns the quantity that couldn't be removed due to there not being enough of the item in the slot.
zcl::t_i32 InventoryRemoveAt(t_inventory *const inventory, const zcl::t_v2_i slot_pos, const zcl::t_i32 quantity);

// @todo: Would probably be safer to have this require that the slot is non-empty, and add a helper function for checking emptiness.
t_inventory_slot InventoryGet(const t_inventory *const inventory, const zcl::t_v2_i slot_pos);

zcl::t_v2_i InventoryGetSize(const t_inventory *const inventory);
