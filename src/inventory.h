#pragma once

#include "items.h"

struct t_inventory;

t_inventory *InventoryCreate(const zcl::t_i32 slot_cnt, zcl::t_arena *const arena);
void InventoryAdd(const t_item_type_id item_type_id, const zcl::t_i32 quantity);
