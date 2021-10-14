/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

 Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sigmastar Technology Corp. and be kept in strict confidence
(Sigmastar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sigmastar Confidential
Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifndef __MI_SCATTERLIST_H__
#define __MI_SCATTERLIST_H__

#ifdef CAM_OS_RTK
#include "mi_sys_internal.h"
#include "cam_os_wrapper.h"
#include "cam_os_util.h"


typedef unsigned long phys_addr_t;
typedef unsigned  gfp_t;

struct page {
	unsigned long long dummy;
};

#define PAGE_SHIFT		12
#define PAGE_MASK	(~(PAGE_SIZE-1))
#define GFP_KERNEL          0 //RTK_ToDo
#define PAGE_ALIGN(x)       x //RTK_ToDo

#define	__phys_to_pfn(paddr)	((unsigned long)((paddr) >> PAGE_SHIFT))
#define	__pfn_to_phys(pfn)	((phys_addr_t)(pfn) << PAGE_SHIFT)

#define page_to_phys(page)	(unsigned long)(__pfn_to_phys(page_to_pfn(page)))
#define phys_to_page(phys)	(pfn_to_page(__phys_to_pfn(phys)))

#define page_to_pfn(x)  (x)
#define pfn_to_page(x) (struct page *)(x)


struct scatterlist {
	unsigned long	page_link;
	unsigned int	offset;
	unsigned int	length;
};


struct sg_table {
	struct scatterlist sgl[1];
	unsigned int nents;
};


#define mi_sg_is_chain(sg)		((sg)->page_link & 0x01)
#define mi_sg_is_last(sg)		((sg)->page_link & 0x02)
#define mi_sg_chain_ptr(sg)	\
	((struct scatterlist *) ((sg)->page_link & ~0x03))


static inline void mi_sg_assign_page(struct scatterlist *sg, struct page *page)
{
	//unsigned long page_link = sg->page_link & 0x3;

	sg->page_link = (unsigned long) page;
}

static inline void sg_set_page(struct scatterlist *sg, struct page *page,
			       unsigned int len, unsigned int offset)
{
	mi_sg_assign_page(sg, page);
	sg->offset = offset;
	sg->length = len;
}

static inline struct page *sg_page(struct scatterlist *sg)
{
	return (struct page *)((sg)->page_link);
}


#define for_each_sg(sglist, sg, nr, __i)	\
    MI_SYS_BUG_ON((nr) != 1);	\
	for (__i = 0, sg = (sglist); __i < (nr); __i++)

void sg_free_table(struct sg_table * table);
int sg_alloc_table(struct sg_table *table, unsigned int nents, gfp_t gfp_mask);
#endif/*CAM_OS_RTK*/

#endif //__MI_SCATTERLIST_H__
