#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

#include "list.h"
#include "drm_misc.h"
#include "pci_misc.h"
#include "klog_misc.h"

/* OPTIONS */

static int opt_klog;

_Noreturn void display_help()
{
	printf("Usage: picorandr [OPTION]\n"
	       "Detect GPUs, list their vendor/device details, connected display resolutions,\n"
	       "connectors, and drivers using the Linux kernel's DRM interface.\n\n"
	       "Options:\n"
	       "  -k          include Linux kernel log output for each GPU driver\n"
	       "  -h          display this help message and exit\n");

	exit(0);
}

int main(int argc, char **argv)
{
	struct list_head *dri_cards_list;
	struct dri_card *card;
	int opt;

	while ((opt = getopt(argc, argv, "kh")) != -1) {
		switch (opt) {
		case 'k':
			opt_klog = 1;
			break;
		case 'h':
			display_help();
			break;
		}
	}

	dri_cards_list = init_dri_cards();

	if (!dri_cards_list) {
		printf("No DRI card detected.\n");
		return 0;
	}

	list_for_each_entry(card, dri_cards_list, list)
	{
		get_pci_info(card);
		drm_lookup_connectors(card);
		if (opt_klog && card->driver_name)
			klog_driver(card->driver_name);
		printf("\n");
	}

	destroy_dri_cards(dri_cards_list);
}
