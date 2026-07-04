#include "inventories.h"

struct t_inventory {
    zcl::t_i32 width;
    zcl::t_array_mut<t_inventory_slot> slots;
};

t_inventory *InventoryCreate(const zcl::t_v2_i size, zcl::t_arena *const arena) {
    ZCL_ASSERT(size.x > 0 && size.y > 0);

    const auto result = zcl::ArenaPush<t_inventory>(arena);
    result->slots = zcl::ArenaPushArray<t_inventory_slot>(arena, size.x * size.y);
    result->width = size.x;

    return result;
}

zcl::t_i32 InventoryAdd(t_inventory *const inventory, const t_item_type_id item_type_id, zcl::t_i32 quantity) {
    ZCL_ASSERT(item_type_id >= 0 && item_type_id < ekm_item_type_id_cnt);
    ZCL_ASSERT(quantity >= 0);

    if (quantity == 0) {
        return 0;
    }

    zcl::t_i32 empty_slots_begin_index = -1; // Update to the index of the first empty slot (for convenience in the second loop).

    // First try adding to wherever the item type already may be in the inventory.
    for (zcl::t_i32 i = 0; i < inventory->slots.len; i++) {
        const auto slot = &inventory->slots[i];

        if (slot->quantity == 0) {
            if (empty_slots_begin_index == -1) {
                empty_slots_begin_index = i;
            }

            continue;
        }

        if (slot->item_type_id != item_type_id) {
            continue;
        }

        quantity = InventoryAddAt(inventory, {i % inventory->width, i / inventory->width}, item_type_id, quantity);

        if (quantity == 0) {
            return 0;
        }
    }

    // Next try adding to the empty slots.
    for (zcl::t_i32 i = (empty_slots_begin_index != -1 ? empty_slots_begin_index : 0); i < inventory->slots.len; i++) {
        const auto slot = &inventory->slots[i];

        quantity = InventoryAddAt(inventory, {i % inventory->width, i / inventory->width}, item_type_id, quantity);

        if (quantity == 0) {
            return 0;
        }
    }

    return quantity;
}

zcl::t_i32 InventoryAddAt(t_inventory *const inventory, const zcl::t_v2_i slot_pos, const t_item_type_id item_type_id, const zcl::t_i32 quantity) {
    ZCL_ASSERT(item_type_id >= 0 && item_type_id < ekm_item_type_id_cnt);
    ZCL_ASSERT(quantity >= 0);

    const auto slot = &inventory->slots[(slot_pos.y * inventory->width) + slot_pos.x];

    if (slot->quantity > 0 && slot->item_type_id != item_type_id) {
        return quantity;
    }

    const zcl::t_i32 quantity_to_add = zcl::CalcMin(quantity, g_item_types[item_type_id].quantity_limit - slot->quantity);

    slot->item_type_id = item_type_id;
    slot->quantity += quantity_to_add;

    return quantity - quantity_to_add;
}

zcl::t_i32 InventoryRemoveAt(t_inventory *const inventory, const zcl::t_v2_i slot_pos, const zcl::t_i32 quantity) {
    ZCL_ASSERT(quantity >= 0);

    const auto slot = &inventory->slots[(slot_pos.y * inventory->width) + slot_pos.x];

    const zcl::t_i32 quantity_to_remove = zcl::CalcMin(quantity, slot->quantity);

    slot->quantity -= quantity_to_remove;

    return quantity - quantity_to_remove;
}

t_inventory_slot InventoryGet(const t_inventory *const inventory, const zcl::t_v2_i slot_pos) {
    return inventory->slots[(slot_pos.y * inventory->width) + slot_pos.x];
}

zcl::t_v2_i InventoryGetSize(const t_inventory *const inventory) {
    return {inventory->width, inventory->slots.len / inventory->width};
}

zcl::t_str_mut InventoryDetermineItemStr(const t_item_type_id item_type_id, const zcl::t_i32 quantity, zcl::t_arena *const arena) {
    ZCL_ASSERT(quantity > 0);

    const auto item_type_name = g_item_types[item_type_id].name;
    const zcl::t_i32 quantity_digit_cnt = zcl::CalcDigitCount(quantity);

    const zcl::t_i32 str_byte_cnt = item_type_name.bytes.len + (quantity > 1 ? quantity_digit_cnt + 4 : 0);
    const auto str_bytes = zcl::ArenaPushArray<zcl::t_u8>(arena, str_byte_cnt);
    auto str_bytes_stream = zcl::ByteStreamCreate(str_bytes, zcl::ek_stream_mode_write);

    if (quantity > 1) {
        zcl::PrintFormat(zcl::ByteStreamGetView(&str_bytes_stream), ZCL_STR_LITERAL("% (x%)"), item_type_name, quantity);
    } else {
        zcl::PrintFormat(zcl::ByteStreamGetView(&str_bytes_stream), ZCL_STR_LITERAL("%"), item_type_name);
    }

    return {str_bytes};
}
