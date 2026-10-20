#include "headerfile/ITEM.h"

t_item generateItemOnDestroy(ITEM_TEX itemTex, ITEM_TYPE itemtype)
{
	return makeItem(itemTex, itemtype, rand() % 2 + 1);
}

t_item makeItem(ITEM_TEX itemTex, ITEM_TYPE itemtype, int count)
{
	t_item newitem;
	newitem.TEX = itemTex;
	newitem.TYPE = itemtype;
	newitem.rect.x = 0;
	newitem.rect.y = 0;
	newitem.rect.w = 16;
	newitem.rect.h = 16;
	newitem.itemCount = count;

	return newitem;
}

const char* GetItemName(ITEM_TEX itemTex)
{
	switch (itemTex)
	{
	case STONE_TEX: return "Stone";
	case WOOD_TEX: return "Wood";
	case STICK_TEX: return "Stick";
	case WOOD_PICK_TEX: return "Wood Pick";
	case WOOD_SWORD_TEX: return "Wood Sword";
	case STONE_PICK_TEX: return "Stone Pick";
	case STONE_SWORD_TEX: return "Stone Sword";
	case PLANK_TEX: return "Plank";
	case LEAVES_TEX: return "Leaves";
	case MOSS_TEX: return "Moss";
	case COMPOST_TEX: return "Compost";
	default: return "";
	}
}

ITEM_TYPE GetItemType(ITEM_TEX itemTex)
{
	switch (itemTex)
	{
	case WOOD_PICK_TEX:
	case STONE_PICK_TEX: return MINE;
	case WOOD_SWORD_TEX:
	case STONE_SWORD_TEX: return ITEMATT;
	case NONE_TEX: return NONE;
	default: return MATERIAL;
	}
}

// damage dealt to leaf spirits with the selected item in hand
double GetWeaponDamage(ITEM_TEX itemTex)
{
	switch (itemTex)
	{
	case WOOD_SWORD_TEX: return 8.0;
	case STONE_SWORD_TEX: return 13.0;
	case WOOD_PICK_TEX:
	case STONE_PICK_TEX: return 5.0;
	default: return 4.0;
	}
}

// how hard the selected item hits trees, rocks and the rest
double GetMiningMultiplier(ITEM_TEX itemTex)
{
	switch (itemTex)
	{
	case WOOD_PICK_TEX: return 2.0;
	case STONE_PICK_TEX: return 3.5;
	case WOOD_SWORD_TEX:
	case STONE_SWORD_TEX: return 1.5;
	default: return 1.0;
	}
}

// 0 = no pickaxe, 1 = wood, 2 = stone: some rocks need a stronger pick
int GetPickTier(ITEM_TEX itemTex)
{
	if (itemTex == STONE_PICK_TEX)
		return 2;
	if (itemTex == WOOD_PICK_TEX)
		return 1;
	return 0;
}
