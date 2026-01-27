#include "game.h"

t_s32 AddToInventory(s_inventory_slot* const slots, const t_s32 slot_cnt, const e_item_type item_type, t_s32 quantity) {
    assert(slots);
    assert(slot_cnt > 0);
    assert(quantity > 0);

    for (t_s32 i = 0; i < slot_cnt && quantity > 0; i++) {
        if (slots[i].quantity > 0 && slots[i].item_type == item_type && slots[i].quantity < ITEM_QUANTITY_LIMIT) {
            const t_s32 quant_to_add = MIN(ITEM_QUANTITY_LIMIT - slots[i].quantity, quantity);
            slots[i].quantity += quant_to_add;
            quantity -= quant_to_add;
        }
    }

    for (t_s32 i = 0; i < slot_cnt && quantity > 0; i++) {
        if (slots[i].quantity == 0) {
            const t_s32 quant_to_add = MIN(ITEM_QUANTITY_LIMIT, quantity);
            slots[i].item_type = item_type;
            slots[i].quantity += quant_to_add;
            quantity -= quant_to_add;
        }
    }

    assert(quantity >= 0);
    return quantity;
}

t_s32 RemoveFromInventory(s_inventory_slot* const slots, const t_s32 slot_cnt, const e_item_type item_type, t_s32 quantity) {
    assert(slots);
    assert(slot_cnt > 0);
    assert(quantity > 0);

    for (t_s32 i = 0; i < slot_cnt && quantity > 0; i++) {
        if (slots[i].quantity > 0 && slots[i].item_type == item_type) {
            const t_s32 quant_to_remove = MIN(slots[i].quantity, quantity);
            slots[i].quantity -= quant_to_remove;
            quantity -= quant_to_remove;
        }
    }

    return quantity;
}

bool DoesInventoryHaveRoomFor(s_inventory_slot* const slots, const t_s32 slot_cnt, const e_item_type item_type, t_s32 quantity) {
    for (t_s32 i = 0; i < slot_cnt && quantity > 0; i++) {
        s_inventory_slot* const slot = &slots[i];

        if (slot->quantity == 0 || slot->item_type == item_type) {
            const t_s32 remaining = ITEM_QUANTITY_LIMIT - slot->quantity;
            quantity = MAX(quantity - remaining, 0);
        }
    }

    assert(quantity >= 0);
    return quantity == 0;
}
