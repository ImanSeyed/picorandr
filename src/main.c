#include <stdio.h>

#include "list.h"
#include "drm_misc.h"
#include "pci_misc.h"
#include "klog_misc.h"

int main()
{
	struct list_head *dri_cards_list;
	struct dri_card *card;

	dri_cards_list = init_dri_cards();

	if (!dri_cards_list) {
		printf("No DRI card detected.\n");
		return 0;
	}

	list_for_each_entry(card, dri_cards_list, list)
	{
		get_pci_info(card);
		drm_lookup_connectors(card);
		if (card->driver_name)
			klog_driver(card->driver_name);
		printf("\n");
	}

	destroy_dri_cards(dri_cards_list);
}
