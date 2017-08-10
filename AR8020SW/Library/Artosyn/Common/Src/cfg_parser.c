#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cfg_parser.h"
#include "debuglog.h"

/*
 * return 0:  data not valid
 *        1:  data valid
*/
static uint8_t CFGBIN_CheckMd5Valid(STRU_cfgBin *ptr_cfg)
{
    uint8_t md5[16];
    STRU_cfgBin *p_cfghead = ptr_cfg;
    if ( p_cfghead->headflag != CFG_DATA_HEAD_FLAG)
    {
        DLOG_Error("head Fail \r\n");
        return 0;
    }

#if 0
    uint8_t *ptr_data = (uint8_t *)(ptr_cfg + 1);   //STRU_cfgBin is 4byte aligned
    MD5Check(ptr_data, p_cfghead->dataSize, md5);

    if (0 != memcmp( (void *)md5, (void *)p_cfghead->dataMd5, 16))
    {
        dlog_error("md5 Fail");
        return 0;
    }
#endif
    return 1;
}


void CFGBIN_LoadFromFlash(uint32_t dstAddr, uint32_t srcAddr)
{
    uint8_t valid;
    uint8_t headsize = sizeof(STRU_cfgBin);
    memcpy((void *)dstAddr, (void *)srcAddr, headsize);
    valid = CFGBIN_CheckMd5Valid( (STRU_cfgBin *)srcAddr);

    if (valid)
    {
        uint32_t datasize = ((STRU_cfgBin *)srcAddr)->dataSize;
        memcpy( (void *)(dstAddr+headsize), (void *)(srcAddr+headsize), datasize);
    }
}

/*
 * ptr_cfg: pointer to the config head
 * return: pointer to the Node data
*/
STRU_cfgNode * CFGBIN_GetNode(STRU_cfgBin *ptr_cfg, uint32_t u32_nodeId)
{
    uint32_t datasize = 0;
    uint32_t totaldatasize = ptr_cfg->dataSize;

    STRU_cfgNode *ptr_node = (STRU_cfgNode *)(ptr_cfg + 1);     //STRU_cfgBin is 4byte aligned

    while (datasize < totaldatasize)
    {
        uint32_t nodesize;

        if (ptr_node->nodeId == u32_nodeId)
        {
            return ptr_node;
        }

        nodesize = ptr_node->nodeDataSize + sizeof(STRU_cfgNode);
        if (nodesize & 0x03)
        {
            nodesize = (nodesize + 4) & (~0x03);  //alignment
        }

        datasize += nodesize;
        ptr_node =  (STRU_cfgNode *)((uint8_t *)ptr_node  + nodesize); //move 1 node and nodeDataSize data
    }

    DLOG_Error("Not find Node %d", u32_nodeId);

    return NULL;
}
