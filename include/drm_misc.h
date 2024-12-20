#pragma once

#include <linux/limits.h>

#include "list.h"

#define DRI_PATH "/dev/dri/by-path"

struct dri_card {
	char *pci_address;
	char devtmpfs_path[PATH_MAX];
	struct list_head list;
};

void drm_lookup_connectors(const struct dri_card *card);
struct list_head *init_dri_cards();
void destroy_dri_cards(struct list_head *head);
