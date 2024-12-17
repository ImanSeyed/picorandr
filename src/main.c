#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pci/pci.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <linux/limits.h>

#include "list.h"

#define DRI_PATH "/dev/dri/by-path"

struct dri_card {
	char *pci_address;
	char devtmpfs_path[PATH_MAX];
	struct list_head list;
};
static struct list_head dri_cards_list;

static const char *conn_type[] = {
	[DRM_MODE_CONNECTOR_Unknown] = "None",
	[DRM_MODE_CONNECTOR_VGA] = "VGA",
	[DRM_MODE_CONNECTOR_DVII] = "DVI-I",
	[DRM_MODE_CONNECTOR_DVID] = "DVI-D",
	[DRM_MODE_CONNECTOR_DVIA] = "DVI-A",
	[DRM_MODE_CONNECTOR_Composite] = "Composite",
	[DRM_MODE_CONNECTOR_SVIDEO] = "SVIDEO",
	[DRM_MODE_CONNECTOR_LVDS] = "LVDS",
	[DRM_MODE_CONNECTOR_Component] = "Component",
	[DRM_MODE_CONNECTOR_9PinDIN] = "DIN",
	[DRM_MODE_CONNECTOR_DisplayPort] = "DP",
	[DRM_MODE_CONNECTOR_HDMIA] = "HDMI",
	[DRM_MODE_CONNECTOR_HDMIB] = "HDMI-B",
	[DRM_MODE_CONNECTOR_TV] = "TV",
	[DRM_MODE_CONNECTOR_eDP] = "eDP",
	[DRM_MODE_CONNECTOR_VIRTUAL] = "Virtual",
	[DRM_MODE_CONNECTOR_DSI] = "DSI",
	[DRM_MODE_CONNECTOR_DPI] = "DPI",
};

static const char *conn_mode[] = {
	[DRM_MODE_CONNECTED] = "connected",
	[DRM_MODE_DISCONNECTED] = "disconnected",
	[DRM_MODE_UNKNOWNCONNECTION] = "unknown",
};

static int ends_with(const char *str, const char *suffix)
{
	size_t len_str, len_suffix;

	if (!str || !suffix)
		return 0;

	len_str = strlen(str);
	len_suffix = strlen(suffix);

	if (len_suffix > len_str)
		return 0;

	return strncmp(str + len_str - len_suffix, suffix, len_suffix) == 0;
}

static void drm_lookup_connectors(struct dri_card *card)
{
	int drm_fd = open(card->devtmpfs_path, O_RDWR | O_NONBLOCK);
	drmModeRes *resources = drmModeGetResources(drm_fd);

	for (int i = 0; i < resources->count_connectors; ++i) {
		drmModeConnector *conn =
			drmModeGetConnector(drm_fd, resources->connectors[i]);
		if (!conn)
			continue;

		printf("\t%s-%u (%s)\n", conn_type[conn->connector_type],
		       conn->connector_type_id, conn_mode[conn->connection]);

		if (conn->modes)
			printf("\t\t* %ux%u\n", conn->modes->hdisplay,
			       conn->modes->vdisplay);

		drmModeFreeConnector(conn);
	}

	drmModeFreeResources(resources);
}

static void get_pci_info(const struct dri_card *card)
{
	uint8_t bus, device, func;
	struct pci_access *pacc;
	struct pci_dev *dev;
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
			pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES);

			pci_lookup_name(pacc, devbuf, sizeof(devbuf),
					PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE,
					dev->vendor_id, dev->device_id);

			printf("%s (%04x:%04x)\n", devbuf, dev->vendor_id,
			       dev->device_id);
			goto out;
		}
	}

	fprintf(stderr, "Device not found for address: %s\n",
		card->pci_address);
out:
	pci_cleanup(pacc);
}

static void extract_pci_address(char *address, const char *pci_card_path)
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

static void init_dri_cards()
{
	struct dri_card *card;
	struct dirent *entry;
	DIR *dp;

	list_head_init(&dri_cards_list);

	dp = opendir(DRI_PATH);
	if (dp == NULL) {
		fprintf(stderr, "Opening %s failed: %s\n", DRI_PATH,
			strerror(errno));
		return;
	}

	while ((entry = readdir(dp))) {
		if (ends_with(entry->d_name, "-card")) {
			card = calloc(1, sizeof(*card));
			card->pci_address = malloc(strlen(entry->d_name) + 1);

			extract_pci_address(card->pci_address, entry->d_name);
			snprintf(card->devtmpfs_path,
				 sizeof(card->devtmpfs_path), "%s/%s", DRI_PATH,
				 entry->d_name);

			list_add_tail(&card->list, &dri_cards_list);
		}
	}

	closedir(dp);
}

static void destroy_dri_cards()
{
	struct dri_card *card, *tmp;

	list_for_each_entry_safe(card, tmp, &dri_cards_list, list)
	{
		list_del(&card->list);
		free(card->pci_address);
		free(card);
	}
}

int main()
{
	struct dri_card *card;

	init_dri_cards();

	list_for_each_entry(card, &dri_cards_list, list)
	{
		get_pci_info(card);
		drm_lookup_connectors(card);
	}

	destroy_dri_cards();
}
