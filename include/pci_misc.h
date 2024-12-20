#pragma once

#include "drm_misc.h"

void get_pci_info(const struct dri_card *card);
void extract_pci_address(char *address, const char *pci_card_path);
