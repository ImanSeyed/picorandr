#include <stdio.h>

#include "list.h"
#include "drm_misc.h"
#include "pci_misc.h"

int main()
{
	struct list_head *dri_cards_list;
	struct dri_card *card;

	dri_cards_list = init_dri_cards();

	list_for_each_entry(card, dri_cards_list, list)
	{
		get_pci_info(card);
		drm_lookup_connectors(card);
		printf("\n");
	}

	destroy_dri_cards(dri_cards_list);
}
