#include <string.h>
#include "db.h"
#include "math/linmath.h"

item_info_t item_info[ITEM_NUM] = {0};

void db_set_desc(int item, const char *str)
{
  strcpy(item_info[item].description, str);
}

void db_set_name(int item, const char *str)
{
  strncpy(item_info[item].name, str, MIN(strlen(str), 17));
}

void db()
{
  item_info[ITEM_POTION_HEALING].base_uses = 1;
  db_set_name(ITEM_POTION_HEALING, "POTION OF HEALING");
  db_set_desc(ITEM_POTION_HEALING, "A POTION OF HEALING");

  item_info[ITEM_SCROLL_MAPPING].base_uses = 1;
  db_set_name(ITEM_SCROLL_MAPPING, "MAGIC MAPPING");
  db_set_desc(ITEM_SCROLL_MAPPING, "A POTION OF MAGIC MAPPING");

  item_info[ITEM_WAND_FIREBOLT].base_uses = 10;
  db_set_name(ITEM_WAND_FIREBOLT, "FIREBOLT");
  db_set_desc(ITEM_WAND_FIREBOLT, "A WAND OF FIRE BOLT");

  item_info[ITEM_GEAR_CHAINHELM].base_uses = 1;
  item_info[ITEM_GEAR_CHAINHELM].armor = 5;
  item_info[ITEM_GEAR_CHAINHELM].slot = SLOT_HEAD;
  db_set_name(ITEM_GEAR_CHAINHELM, "CHAINMAIL HELM");
  db_set_desc(ITEM_GEAR_CHAINHELM, "A HELMET MADE OF CHAIN MAIL");
}