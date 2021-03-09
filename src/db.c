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
  strncpy(item_info[item].name, str, MIN(strlen(str)+1, 18));
}

void db()
{
  memset(item_info, 0, sizeof(item_info_t) * ITEM_NUM);

  // potion of healing
  item_info[ITEM_POTION_HEALING].base_uses = 1;
  db_set_name(ITEM_POTION_HEALING, "POTION");
  db_set_desc(ITEM_POTION_HEALING, "AN UNIDENTIFIED POTION");

  // scroll of magic mapping
  item_info[ITEM_SCROLL_MAPPING].base_uses = 1;
  db_set_name(ITEM_SCROLL_MAPPING, "SCROLL");
  db_set_desc(ITEM_SCROLL_MAPPING, "AN UNIDENTIFIED SCROLL");

  // wand of fire bolt
  item_info[ITEM_WAND_FIREBOLT].base_uses = 10;
  item_info[ITEM_WAND_FIREBOLT].range = 10;
  item_info[ITEM_WAND_FIREBOLT].damage = 20;
  item_info[ITEM_WAND_FIREBOLT].element = ELEMENT_FIRE;
  db_set_name(ITEM_WAND_FIREBOLT, "WAND");
  db_set_desc(ITEM_WAND_FIREBOLT, "AN UNIDENTIFIED WAND");

  // chain helmet
  item_info[ITEM_GEAR_CHAINHELM].armor = 5;
  item_info[ITEM_GEAR_CHAINHELM].slot = SLOT_HEAD;
  item_info[ITEM_GEAR_CHAINHELM].identified = 1;
  db_set_name(ITEM_GEAR_CHAINHELM, "CHAINMAIL HELM");
  db_set_desc(ITEM_GEAR_CHAINHELM, "A HELMET MADE OF CHAIN MAIL");

  // iron dagger
  item_info[ITEM_GEAR_IRONDAGGER].slot = SLOT_RIGHT;
  item_info[ITEM_GEAR_IRONDAGGER].damage = 5;
  item_info[ITEM_GEAR_IRONDAGGER].identified = 1;
  db_set_name(ITEM_GEAR_IRONDAGGER, "IRON DAGGER");
  db_set_desc(ITEM_GEAR_IRONDAGGER, "A BASIC DAGGER FORGED FROM IRON");
}

void db_set(int item)
{
  item_info[item].identified = 1;

  switch (item) {
    case ITEM_POTION_HEALING: {
      db_set_name(ITEM_POTION_HEALING, "POTION OF HEALING");
      db_set_desc(ITEM_POTION_HEALING, "A POTION OF HEALING");
      break;
    }

    case ITEM_SCROLL_MAPPING: {
      db_set_name(ITEM_SCROLL_MAPPING, "MAGIC MAPPING");
      db_set_desc(ITEM_SCROLL_MAPPING, "A POTION OF MAGIC MAPPING");
      break;
    }

    case ITEM_WAND_FIREBOLT: {
      db_set_name(ITEM_WAND_FIREBOLT, "WAND OF FIREBOLT");
      db_set_desc(ITEM_WAND_FIREBOLT, "A WAND OF FIRE BOLT");
      break;
    }
  }
}