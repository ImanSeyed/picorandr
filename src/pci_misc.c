#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <pci/pci.h>

#include "drm_misc.h"

void get_pci_info(const struct dri_card *card)
{
	uint8_t bus, device, func;
	struct pci_access *pacc;
	struct pci_dev *dev;
	const char *driver;
	char devbuf[1024];
	int domain;

	pacc = pci_alloc();
	pci_init(pacc);
	pci_scan_bus(pacc);

	if (sscanf(card->pci_address, "%x:%hhx:%hhx.%hhx", &domain, &bus,
		   &device, &func) != 4) {
		fprintf(stderr, "Invalid PCI address format: %s\n",
			card->pci_address);
		goto out;
	}

	for (dev = pacc->devices; dev; dev = dev->next) {
		if (dev->domain == domain && dev->bus == bus &&
		    dev->dev == device && dev->func == func) {
			pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES |
						   PCI_FILL_DRIVER);

			pci_lookup_name(pacc, devbuf, sizeof(devbuf),
					PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE,
					dev->vendor_id, dev->device_id);

			printf("%04x:%04x %s\n", dev->vendor_id, dev->device_id,
			       devbuf);

			driver = pci_get_string_property(dev, PCI_FILL_DRIVER);
			if (driver)
				printf("\t  Kernel driver in use: %s\n",
				       driver);

			goto out;
		}
	}

	fprintf(stderr, "Device not found for address: %s\n",
		card->pci_address);
out:
	pci_cleanup(pacc);
}

void extract_pci_address(char *address, const char *pci_card_path)
{
	char *tmp, *pos;

	tmp = calloc(strlen(pci_card_path) + 1, sizeof(char));
	strcpy(tmp, pci_card_path);

	pos = strtok(tmp, "-");

	if (pos != NULL) {
		pos = strtok(NULL, "-");
		strcpy(address, pos);
	}

	free(tmp);
}
