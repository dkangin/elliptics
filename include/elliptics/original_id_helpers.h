#ifndef __ORIGINAL_ID_HELPERS_H
#define __ORIGINAL_ID_HELPERS_H

#include <eblob/blob.h>
#include "elliptics/packet.h"
#include "elliptics/interface.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void copy_identifiers_cmd (struct eblob_key *src_key, uint8_t* original_id)
{
	memset((void*)src_key->id, 0, EBLOB_ID_SIZE);
	int id_size = sizeof (uint64_t);
	memcpy(src_key->id, original_id + EBLOB_ID_SIZE - sizeof (uint64_t), id_size);
}


#ifdef __cplusplus
}
#endif

#endif /* __ORIGINAL_ID_HELPERS_H */
