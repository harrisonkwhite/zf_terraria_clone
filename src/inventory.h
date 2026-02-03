#pragma once

#include "items.h"

// @note: So this could be moved into the world module and the assumption of the inventories being 2D could be safely made.

struct t_inventory;

struct t_inventory_slot {
    t_item_type_id item_type_id;
    zcl::t_i32 quantity;
};

t_inventory *InventoryCreate(const zcl::t_i32 slot_cnt, zcl::t_arena *const arena);

// Returns the quantity that couldn't be added due to the inventory getting filled (0 for all added).
zcl::t_i32 InventoryAdd(t_inventory *const inventory, const t_item_type_id item_type_id, zcl::t_i32 quantity);

// Returns the quantity that couldn't be added due to the slot getting filled (0 for all added).
zcl::t_i32 InventoryAddAt(t_inventory *const inventory, const zcl::t_i32 slot_index, const t_item_type_id item_type_id, const zcl::t_i32 quantity);

// Returns the quantity that couldn't be removed due to there not being enough of the item in the slot.
zcl::t_i32 InventoryRemoveAt(t_inventory *const inventory, const zcl::t_i32 slot_index, const zcl::t_i32 quantity);

t_inventory_slot InventoryGet(const t_inventory *const inventory, const zcl::t_i32 slot_index);
