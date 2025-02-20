#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <xf86drm.h>
#include <sys/types.h>
#include <xf86drmMode.h>

#include "list.h"
#include "drm_misc.h"
#include "pci_misc.h"

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

void drm_lookup_connectors(const struct dri_card *card)
{
	int drm_fd;
	drmModeRes *resources;

	drm_fd = open(card->devtmpfs_path, O_RDWR | O_NONBLOCK);
	if (drm_fd == -1) {
		perror("drm_lookup_connectors");
		return;
	}

	resources = drmModeGetResources(drm_fd);
	if (!resources) {
		fprintf(stderr, "%s: Couldn't get DRM resources.",
			card->pci_address);
		return;
	}

	printf("\t  Connectors:\n");
	for (int i = 0; i < resources->count_connectors; ++i) {
		drmModeConnector *conn =
			drmModeGetConnector(drm_fd, resources->connectors[i]);
		if (!conn)
			continue;

		printf("\t\t  %s-%u (%s)\n", conn_type[conn->connector_type],
		       conn->connector_type_id, conn_mode[conn->connection]);

		if (conn->modes)
			printf("\t\t\t  * %ux%u\n", conn->modes->hdisplay,
			       conn->modes->vdisplay);

		drmModeFreeConnector(conn);
	}

	drmModeFreeResources(resources);
}

struct list_head *init_dri_cards()
{
	struct list_head *dri_cards_list;
	size_t pci_address_len;
	struct dri_card *card;
	struct dirent *entry;
	DIR *dp;

	dp = opendir(DRI_PATH);
	dri_cards_list = NULL;

	if (dp == NULL) {
		fprintf(stderr, "Opening %s failed: %s\n", DRI_PATH,
			strerror(errno));
		goto out;
	}

	dri_cards_list = malloc(sizeof(*dri_cards_list));
	if (!dri_cards_list)
		goto out;

	list_head_init(dri_cards_list);

	while ((entry = readdir(dp))) {
		if (ends_with(entry->d_name, "-card")) {
			pci_address_len = strlen(entry->d_name) + 1;
			card = calloc(1, sizeof(*card));
			if (!card)
				goto out;

			card->pci_address = malloc(pci_address_len);
			if (!card->pci_address)
				goto out;

			extract_pci_address(card->pci_address, entry->d_name,
					    pci_address_len);
			snprintf(card->devtmpfs_path,
				 sizeof(card->devtmpfs_path), "%s/%s", DRI_PATH,
				 entry->d_name);

			list_add_tail(&card->list, dri_cards_list);
		}
	}

out:
	closedir(dp);
	return dri_cards_list;
}

void destroy_dri_cards(struct list_head *dri_cards_list)
{
	struct dri_card *card, *tmp;

	list_for_each_entry_safe(card, tmp, dri_cards_list, list)
	{
		list_del(&card->list);
		free(card->pci_address);
		free(card);
	}

	free(dri_cards_list);
}
