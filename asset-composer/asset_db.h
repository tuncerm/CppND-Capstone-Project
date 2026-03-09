#ifndef ASSET_DB_H
#define ASSET_DB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ASSET_DB_VERSION 1u
#define ASSET_DB_MAGIC "ASDB"
#define ASSET_DB_NAME_LEN 16
#define ASSET_DB_MAX_ASSETS 1024
#define ASSET_DB_CELL_TILE_COUNT 16

typedef enum {
    ASSET_TYPE_INVALID = 0,
    ASSET_TYPE_CELL32 = 1,
    ASSET_TYPE_SPRITE = 2,
    ASSET_TYPE_ANIM = 3,
} AssetType;

typedef struct {
    uint16_t asset_id;
    uint8_t type;
    char name[ASSET_DB_NAME_LEN + 1];
    uint16_t flags;
    uint8_t tile_refs[ASSET_DB_CELL_TILE_COUNT];
} AssetRecord;

typedef struct {
    AssetRecord records[ASSET_DB_MAX_ASSETS];
    size_t count;
} AssetDb;

void asset_db_init_empty(AssetDb* db);
void asset_db_init_default(AssetDb* db);
bool asset_db_add_default_cell(AssetDb* db, uint16_t asset_id);
bool asset_db_remove_at(AssetDb* db, size_t index);
bool asset_db_load(AssetDb* db, const char* path, char* error, size_t error_size);
bool asset_db_save(const AssetDb* db, const char* path, char* error, size_t error_size);

#endif // ASSET_DB_H
