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
  item_info[ITEM_WAND_FIREBOLT].damage = 10;
  db_set_name(ITEM_WAND_FIREBOLT, "WAND");
  db_set_desc(ITEM_WAND_FIREBOLT, "AN UNIDENTIFIED WAND");

  // wand of lightning
  item_info[ITEM_WAND_LIGHTNING].base_uses = 5;
  item_info[ITEM_WAND_LIGHTNING].range = 10;
  item_info[ITEM_WAND_LIGHTNING].damage = 25;
  db_set_name(ITEM_WAND_LIGHTNING, "WAND");
  db_set_desc(ITEM_WAND_LIGHTNING, "AN UNIDENTIFIED WAND");

  // wand of identify
  item_info[ITEM_WAND_IDENTIFY].base_uses = 20;
  item_info[ITEM_WAND_IDENTIFY].range = 100;
  item_info[ITEM_WAND_IDENTIFY].damage = 0;
  item_info[ITEM_WAND_IDENTIFY].identified = 1;
  db_set_name(ITEM_WAND_IDENTIFY, "WAND OF IDENTIFY");
  db_set_desc(ITEM_WAND_IDENTIFY, "SILENTLY IDENTIFIES ANDINSPECTS A CREATURE");

  // chain helmet
  item_info[ITEM_GEAR_CHAINHELM].armor = 1;
  item_info[ITEM_GEAR_CHAINHELM].slot = SLOT_HEAD;
  item_info[ITEM_GEAR_CHAINHELM].identified = 1;
  db_set_name(ITEM_GEAR_CHAINHELM, "CHAINMAIL HELM");
  db_set_desc(ITEM_GEAR_CHAINHELM, "A HELMET MADE OF CHAIN MAIL");

  // chain chest
  item_info[ITEM_GEAR_CHAINCHEST].armor = 1;
  item_info[ITEM_GEAR_CHAINCHEST].slot = SLOT_CHEST;
  item_info[ITEM_GEAR_CHAINCHEST].identified = 1;
  db_set_name(ITEM_GEAR_CHAINCHEST, "CHAINMAIL PLATE");
  db_set_desc(ITEM_GEAR_CHAINCHEST, "A BREASTPLATE MADE OF CHAIN MAIL");

  // chain legs
  item_info[ITEM_GEAR_CHAINLEGS].armor = 1;
  item_info[ITEM_GEAR_CHAINLEGS].slot = SLOT_LEGS;
  item_info[ITEM_GEAR_CHAINLEGS].identified = 1;
  db_set_name(ITEM_GEAR_CHAINLEGS, "CHAINMAIL LEGS");
  db_set_desc(ITEM_GEAR_CHAINLEGS, "LEGGINGS MADE OF CHAIN MAIL");

  // chain boots
  item_info[ITEM_GEAR_IRONBOOTS].armor = 1;
  item_info[ITEM_GEAR_IRONBOOTS].slot = SLOT_FEET;
  item_info[ITEM_GEAR_IRONBOOTS].identified = 1;
  db_set_name(ITEM_GEAR_IRONBOOTS, "IRON BOOTS");
  db_set_desc(ITEM_GEAR_IRONBOOTS, "BOOTS FORGED FROM IRON");

  // leather gloves
  item_info[ITEM_GEAR_GLOVES].armor = 1;
  item_info[ITEM_GEAR_GLOVES].slot = SLOT_HANDS;
  item_info[ITEM_GEAR_GLOVES].identified = 1;
  db_set_name(ITEM_GEAR_GLOVES, "LEATHER GLOVES");
  db_set_desc(ITEM_GEAR_GLOVES, "BASIC LEATHER GLOVES");

  // iron paulders
  item_info[ITEM_GEAR_ARMS].armor = 1;
  item_info[ITEM_GEAR_ARMS].slot = SLOT_ARMS;
  item_info[ITEM_GEAR_ARMS].identified = 1;
  db_set_name(ITEM_GEAR_ARMS, "IRON SPAULDERS");
  db_set_desc(ITEM_GEAR_ARMS, "FORGED IRON SPAULDERS");

  // leather shield
  item_info[ITEM_GEAR_SHIELD].armor = 1;
  item_info[ITEM_GEAR_SHIELD].slot = SLOT_LEFT;
  item_info[ITEM_GEAR_SHIELD].identified = 1;
  db_set_name(ITEM_GEAR_SHIELD, "LEATHER SHIELD");
  db_set_desc(ITEM_GEAR_SHIELD, "A BASIC LEATHER SHIELD");

  // iron dagger
  item_info[ITEM_GEAR_IRONDAGGER].slot = SLOT_RIGHT;
  item_info[ITEM_GEAR_IRONDAGGER].damage = 5;
  item_info[ITEM_GEAR_IRONDAGGER].identified = 1;
  db_set_name(ITEM_GEAR_IRONDAGGER, "IRON DAGGER");
  db_set_desc(ITEM_GEAR_IRONDAGGER, "A BASIC DAGGER FORGED FROM IRON");

  // iron dagger
  item_info[ITEM_GEAR_IRONSWORD].slot = SLOT_RIGHT;
  item_info[ITEM_GEAR_IRONSWORD].damage = 8;
  item_info[ITEM_GEAR_IRONSWORD].identified = 1;
  db_set_name(ITEM_GEAR_IRONSWORD, "IRON LONG SWORD");
  db_set_desc(ITEM_GEAR_IRONSWORD, "A BASIC LONG SWORD FORGED FROM IRON");

  // great sword
  item_info[ITEM_GEAR_GREATSWORD].slot = SLOT_RIGHT;
  item_info[ITEM_GEAR_GREATSWORD].damage = 10;
  item_info[ITEM_GEAR_GREATSWORD].identified = 1;
  db_set_name(ITEM_GEAR_GREATSWORD, "IRON GREAT SWORD");
  db_set_desc(ITEM_GEAR_GREATSWORD, "AN IRON GREAT SWORD. ROUGHLY THE SIZE OF A HUMAN");

  // key
  item_info[ITEM_KEY].base_uses = 1;
  item_info[ITEM_KEY].identified = 1;
  db_set_name(ITEM_KEY, "GOLDEN KEY");
  db_set_desc(ITEM_KEY, "A GOLDEN KEY GLOWING   WITH MAGIC ENERGY");
}

void db_set(int item)
{
  item_info[item].identified = 1;

  switch (item) {
    case ITEM_POTION_HEALING: {
      db_set_name(ITEM_POTION_HEALING, "POTION OF HEALING");
      db_set_desc(ITEM_POTION_HEALING, "HEALS A LARGE PORTION OF HEALTH UPON CONSUMPTION");
      break;
    }

    case ITEM_SCROLL_MAPPING: {
      db_set_name(ITEM_SCROLL_MAPPING, "MAGIC MAPPING");
      db_set_desc(ITEM_SCROLL_MAPPING, "UNVEILS YOUR SURROUNDINGS");
      break;
    }

    case ITEM_WAND_FIREBOLT: {
      db_set_name(ITEM_WAND_FIREBOLT, "WAND OF FIREBOLT");
      db_set_desc(ITEM_WAND_FIREBOLT, "FIRES A BOLT OF MOLTEN LIQUID");
      break;
    }

    case ITEM_WAND_LIGHTNING: {
      db_set_name(ITEM_WAND_LIGHTNING, "WAND OF LIGHTNING");
      db_set_desc(ITEM_WAND_LIGHTNING, "ZAPS ALL THOSE AROUND IT WITH AN ARC OF ELECTRICITY");
      break;
    }
  }
}