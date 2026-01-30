#include "inventory.h"

constexpr zcl::t_i32 k_quantity_limit = 99; // @todo: This is vary based on item type in the future.

struct t_inventory {
    zcl::t_array_mut<t_inventory_slot> slots;
};

t_inventory *InventoryCreate(const zcl::t_i32 slot_cnt, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_inventory>(arena);
    result->slots = zcl::ArenaPushArray<t_inventory_slot>(arena, slot_cnt);

    return result;
}

static void InventoryAddHelper(t_inventory *const inventory, const t_item_type_id item_type_id, const zcl::t_i32 quantity, const zcl::t_i32 begin_slot_index) {
    ZCL_ASSERT(begin_slot_index >= 0 && begin_slot_index <= inventory->slots.len);

    if (quantity == 0) {
        return;
    }

    for (zcl::t_i32 i = begin_slot_index; i < inventory->slots.len; i++) {
        const auto slot = &inventory->slots[i];

        zcl::t_b8 add = false;

        if (slot->quantity == 0) {
            add = true;
        } else if (slot->item_type_id == item_type_id && slot->quantity < k_quantity_limit) {
            add = true;
        }

        if (add) {
            const zcl::t_i32 quantity_to_add = zcl::CalcMin(quantity, k_quantity_limit - slot->quantity);

            slot->item_type_id = item_type_id;
            slot->quantity += quantity_to_add;

            InventoryAddHelper(inventory, item_type_id, quantity - quantity_to_add, i + 1);

            return;
        }
    }
}

void InventoryAdd(t_inventory *const inventory, const t_item_type_id item_type_id, const zcl::t_i32 quantity) {
    ZCL_ASSERT(item_type_id >= 0 && item_type_id < ekm_item_type_id_cnt);
    ZCL_ASSERT(quantity >= 0);

    InventoryAddHelper(inventory, item_type_id, quantity, 0);
}

t_inventory_slot InventoryGet(const t_inventory *const inventory, const zcl::t_i32 slot_index) {
    return inventory->slots[slot_index];
}
