#include "asset_db.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    HEADER_SIZE = 16,
    INDEX_ENTRY_SIZE = 32,
    CELL32_RECORD_SIZE = 20,
};

typedef struct {
    uint16_t asset_id;
    uint8_t type;
    uint8_t reserved;
    char name[ASSET_DB_NAME_LEN];
    uint32_t record_offset;
    uint32_t record_size;
    uint32_t crc32;
} IndexEntry;

static void set_error(char* error, size_t error_size, const char* message) {
    if (!error || error_size == 0 || !message) {
        return;
    }
    strncpy(error, message, error_size - 1);
    error[error_size - 1] = '\0';
}

static uint16_t read_u16_le(const uint8_t* bytes) {
    return (uint16_t)(bytes[0] | ((uint16_t)bytes[1] << 8));
}

static uint32_t read_u32_le(const uint8_t* bytes) {
    return (uint32_t)(bytes[0] | ((uint32_t)bytes[1] << 8) | ((uint32_t)bytes[2] << 16) |
                      ((uint32_t)bytes[3] << 24));
}

static void write_u16_le(uint8_t* out, uint16_t value) {
    out[0] = (uint8_t)(value & 0xFFu);
    out[1] = (uint8_t)((value >> 8) & 0xFFu);
}

static void write_u32_le(uint8_t* out, uint32_t value) {
    out[0] = (uint8_t)(value & 0xFFu);
    out[1] = (uint8_t)((value >> 8) & 0xFFu);
    out[2] = (uint8_t)((value >> 16) & 0xFFu);
    out[3] = (uint8_t)((value >> 24) & 0xFFu);
}

static uint32_t crc32_compute(const uint8_t* data, size_t size) {
    uint32_t crc = 0xFFFFFFFFu;
    if (!data || size == 0) {
        return 0u;
    }

    for (size_t i = 0; i < size; ++i) {
        crc ^= (uint32_t)data[i];
        for (int bit = 0; bit < 8; ++bit) {
            const uint32_t mask = (uint32_t)-(int)(crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return ~crc;
}

static bool parse_index_entry(const uint8_t* bytes, IndexEntry* out) {
    if (!bytes || !out) {
        return false;
    }

    out->asset_id = read_u16_le(bytes + 0);
    out->type = bytes[2];
    out->reserved = bytes[3];
    memcpy(out->name, bytes + 4, ASSET_DB_NAME_LEN);
    out->record_offset = read_u32_le(bytes + 20);
    out->record_size = read_u32_le(bytes + 24);
    out->crc32 = read_u32_le(bytes + 28);
    return true;
}

static void encode_index_entry(const IndexEntry* entry, uint8_t* out_bytes) {
    if (!entry || !out_bytes) {
        return;
    }

    memset(out_bytes, 0, INDEX_ENTRY_SIZE);
    write_u16_le(out_bytes + 0, entry->asset_id);
    out_bytes[2] = entry->type;
    out_bytes[3] = 0;
    memcpy(out_bytes + 4, entry->name, ASSET_DB_NAME_LEN);
    write_u32_le(out_bytes + 20, entry->record_offset);
    write_u32_le(out_bytes + 24, entry->record_size);
    write_u32_le(out_bytes + 28, entry->crc32);
}

void asset_db_init_empty(AssetDb* db) {
    if (!db) {
        return;
    }
    memset(db, 0, sizeof(*db));
}

void asset_db_init_default(AssetDb* db) {
    if (!db) {
        return;
    }

    asset_db_init_empty(db);
    (void)asset_db_add_default_cell(db, 0);
}

bool asset_db_add_default_cell(AssetDb* db, uint16_t asset_id) {
    if (!db || db->count >= ASSET_DB_MAX_ASSETS) {
        return false;
    }

    AssetRecord* record = &db->records[db->count];
    memset(record, 0, sizeof(*record));
    record->asset_id = asset_id;
    record->type = ASSET_TYPE_CELL32;
    snprintf(record->name, sizeof(record->name), "CELL_%04u", (unsigned)asset_id);
    for (int i = 0; i < ASSET_DB_CELL_TILE_COUNT; ++i) {
        record->tile_refs[i] = 0;
    }

    db->count++;
    return true;
}

bool asset_db_remove_at(AssetDb* db, size_t index) {
    if (!db || index >= db->count) {
        return false;
    }

    if (index + 1 < db->count) {
        memmove(&db->records[index], &db->records[index + 1],
                (db->count - index - 1) * sizeof(db->records[0]));
    }
    db->count--;
    return true;
}

bool asset_db_load(AssetDb* db, const char* path, char* error, size_t error_size) {
    if (!db || !path || path[0] == '\0') {
        set_error(error, error_size, "Invalid load input");
        return false;
    }

    FILE* file = fopen(path, "rb");
    if (!file) {
        set_error(error, error_size, "File not found");
        return false;
    }

    uint8_t header[HEADER_SIZE] = {0};
    if (fread(header, 1, sizeof(header), file) != sizeof(header)) {
        fclose(file);
        set_error(error, error_size, "Header read failed");
        return false;
    }

    if (memcmp(header, ASSET_DB_MAGIC, 4) != 0) {
        fclose(file);
        set_error(error, error_size, "Invalid magic");
        return false;
    }

    const uint16_t version = read_u16_le(header + 4);
    const uint16_t asset_count = read_u16_le(header + 6);
    const uint32_t index_offset = read_u32_le(header + 8);

    if (version != ASSET_DB_VERSION) {
        fclose(file);
        set_error(error, error_size, "Unsupported version");
        return false;
    }
    if (asset_count > ASSET_DB_MAX_ASSETS) {
        fclose(file);
        set_error(error, error_size, "Asset count too large");
        return false;
    }

    if (fseek(file, (long)index_offset, SEEK_SET) != 0) {
        fclose(file);
        set_error(error, error_size, "Index seek failed");
        return false;
    }

    asset_db_init_empty(db);
    for (uint16_t i = 0; i < asset_count; ++i) {
        uint8_t index_bytes[INDEX_ENTRY_SIZE] = {0};
        if (fread(index_bytes, 1, sizeof(index_bytes), file) != sizeof(index_bytes)) {
            fclose(file);
            set_error(error, error_size, "Index read failed");
            asset_db_init_empty(db);
            return false;
        }

        IndexEntry entry;
        if (!parse_index_entry(index_bytes, &entry)) {
            fclose(file);
            set_error(error, error_size, "Index parse failed");
            asset_db_init_empty(db);
            return false;
        }

        if (entry.type != ASSET_TYPE_CELL32) {
            continue;
        }
        if (entry.record_size < CELL32_RECORD_SIZE) {
            fclose(file);
            set_error(error, error_size, "CELL32 record size invalid");
            asset_db_init_empty(db);
            return false;
        }

        if (fseek(file, (long)entry.record_offset, SEEK_SET) != 0) {
            fclose(file);
            set_error(error, error_size, "Record seek failed");
            asset_db_init_empty(db);
            return false;
        }

        uint8_t* record_bytes = (uint8_t*)malloc(entry.record_size);
        if (!record_bytes) {
            fclose(file);
            set_error(error, error_size, "Out of memory");
            asset_db_init_empty(db);
            return false;
        }

        const size_t bytes_read = fread(record_bytes, 1, entry.record_size, file);
        if (bytes_read != entry.record_size) {
            free(record_bytes);
            fclose(file);
            set_error(error, error_size, "Record read failed");
            asset_db_init_empty(db);
            return false;
        }

        const uint32_t actual_crc = crc32_compute(record_bytes, entry.record_size);
        if (entry.crc32 != 0u && actual_crc != entry.crc32) {
            free(record_bytes);
            fclose(file);
            set_error(error, error_size, "CRC mismatch");
            asset_db_init_empty(db);
            return false;
        }

        AssetRecord* record = &db->records[db->count];
        memset(record, 0, sizeof(*record));
        record->asset_id = entry.asset_id;
        record->type = entry.type;
        memcpy(record->name, entry.name, ASSET_DB_NAME_LEN);
        record->name[ASSET_DB_NAME_LEN] = '\0';
        record->flags = read_u16_le(record_bytes + 2);
        memcpy(record->tile_refs, record_bytes + 4, ASSET_DB_CELL_TILE_COUNT);
        db->count++;

        free(record_bytes);
        if (db->count >= ASSET_DB_MAX_ASSETS) {
            break;
        }
        if (fseek(file, (long)(index_offset + (uint32_t)(i + 1) * INDEX_ENTRY_SIZE), SEEK_SET) != 0) {
            break;
        }
    }

    fclose(file);
    set_error(error, error_size, "");
    return true;
}

bool asset_db_save(const AssetDb* db, const char* path, char* error, size_t error_size) {
    if (!db || !path || path[0] == '\0') {
        set_error(error, error_size, "Invalid save input");
        return false;
    }
    if (db->count > ASSET_DB_MAX_ASSETS) {
        set_error(error, error_size, "Too many assets");
        return false;
    }

    FILE* file = fopen(path, "wb");
    if (!file) {
        set_error(error, error_size, "Open for write failed");
        return false;
    }

    const uint32_t index_offset = HEADER_SIZE;
    const uint32_t records_offset = index_offset + (uint32_t)(db->count * INDEX_ENTRY_SIZE);

    uint8_t header[HEADER_SIZE] = {0};
    memcpy(header + 0, ASSET_DB_MAGIC, 4);
    write_u16_le(header + 4, ASSET_DB_VERSION);
    write_u16_le(header + 6, (uint16_t)db->count);
    write_u32_le(header + 8, index_offset);
    write_u32_le(header + 12, 0u);

    if (fwrite(header, 1, sizeof(header), file) != sizeof(header)) {
        fclose(file);
        set_error(error, error_size, "Header write failed");
        return false;
    }

    uint32_t current_record_offset = records_offset;
    for (size_t i = 0; i < db->count; ++i) {
        const AssetRecord* record = &db->records[i];

        uint8_t record_bytes[CELL32_RECORD_SIZE] = {0};
        record_bytes[0] = 4;
        record_bytes[1] = 4;
        write_u16_le(record_bytes + 2, record->flags);
        memcpy(record_bytes + 4, record->tile_refs, ASSET_DB_CELL_TILE_COUNT);

        IndexEntry entry;
        memset(&entry, 0, sizeof(entry));
        entry.asset_id = record->asset_id;
        entry.type = record->type == ASSET_TYPE_CELL32 ? ASSET_TYPE_CELL32 : ASSET_TYPE_CELL32;
        size_t name_len = strnlen(record->name, ASSET_DB_NAME_LEN);
        if (name_len > 0) {
            memcpy(entry.name, record->name, name_len);
        }
        entry.record_offset = current_record_offset;
        entry.record_size = CELL32_RECORD_SIZE;
        entry.crc32 = crc32_compute(record_bytes, sizeof(record_bytes));

        uint8_t index_bytes[INDEX_ENTRY_SIZE] = {0};
        encode_index_entry(&entry, index_bytes);
        if (fwrite(index_bytes, 1, sizeof(index_bytes), file) != sizeof(index_bytes)) {
            fclose(file);
            set_error(error, error_size, "Index write failed");
            return false;
        }

        current_record_offset += CELL32_RECORD_SIZE;
    }

    for (size_t i = 0; i < db->count; ++i) {
        const AssetRecord* record = &db->records[i];
        uint8_t record_bytes[CELL32_RECORD_SIZE] = {0};
        record_bytes[0] = 4;
        record_bytes[1] = 4;
        write_u16_le(record_bytes + 2, record->flags);
        memcpy(record_bytes + 4, record->tile_refs, ASSET_DB_CELL_TILE_COUNT);

        if (fwrite(record_bytes, 1, sizeof(record_bytes), file) != sizeof(record_bytes)) {
            fclose(file);
            set_error(error, error_size, "Record write failed");
            return false;
        }
    }

    fclose(file);
    set_error(error, error_size, "");
    return true;
}
