#pragma once

#include "items.h"

struct t_inventory;

struct t_inventory_slot {
    t_item_type_id item_type_id;
    zcl::t_i32 quantity;
};

t_inventory *InventoryCreate(const zcl::t_i32 slot_cnt, zcl::t_arena *const arena);
void InventoryAdd(t_inventory *const inventory, const t_item_type_id item_type_id, const zcl::t_i32 quantity);
t_inventory_slot InventoryGet(const t_inventory *const inventory, const zcl::t_i32 slot_index);
