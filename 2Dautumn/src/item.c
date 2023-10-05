#include "ITEM.h"

t_item generateItemOnDestroy(ITEM_TEX itemTex, ITEM_TYPE itemtype)
{
	t_item newitem;
	newitem.TEX = itemTex;
	newitem.TYPE = itemtype;
	newitem.rect.x = 0;
	newitem.rect.y = 0;
	newitem.rect.w = 16;
	newitem.rect.h = 16;
	newitem.itemCount = rand() % 2 + 1;

	return newitem;
}

