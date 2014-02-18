#ifndef __ORIGINAL_ID_HELPERS_H
#define __ORIGINAL_ID_HELPERS_H

#include <eblob/blob.h>
#include "elliptics/packet.h"
#include "elliptics/interface.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void copy_identifiers_cmd (uint8_t *src_key, uint8_t* original_id)
{
    memset((void*)src_key, 0, EBLOB_ID_SIZE);
    int id_size = sizeof (uint64_t);
    int i, j;
    for (i = 0, j = EBLOB_ID_SIZE - 1; i < id_size; i++, j--)
    {
        src_key [i] = original_id [j];
    }
}

static inline void reverse_copy_identifiers_cmd (uint8_t *dst_key, uint8_t *src_key)
{
    int id_size = sizeof (int64_t);
    int i, j;
    for (i = 0, j = EBLOB_ID_SIZE - 1; i < id_size; i++)
    {
        dst_key [j] = src_key [i];
    }
}

int dnet_crypto_direct (const void *src, uint64_t size, void *dst, unsigned int *dsize);

#ifdef __cplusplus
}
#endif

#endif /* __ORIGINAL_ID_HELPERS_H */
