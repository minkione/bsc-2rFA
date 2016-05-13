/* Code used by both libbsc and libmsc (xsc means "BSC or MSC").
 *
 * (C) 2016 by sysmocom s.m.f.c. <info@sysmocom.de>
 * (C) 2008-2010 by Harald Welte <laforge@gnumonks.org>
 * (C) 2014 by Holger Hans Peter Freyther
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdbool.h>

#include <osmocom/gsm/gsm0480.h>

#include <openbsc/xsc.h>
#include <openbsc/gsm_data.h>
#include <openbsc/gsm_subscriber.h>

/* Warning: if bsc_network_init() is not called, some of the members of
 * gsm_network are not initialized properly and must not be used! (In
 * particular the llist heads and stats counters.)
 * The long term aim should be to have entirely separate structs for libbsc and
 * libmsc with some common general items.
 */
struct gsm_network *gsm_network_init(void *ctx,
				     uint16_t country_code,
				     uint16_t network_code,
				     mncc_recv_cb_t mncc_recv)
{
	struct gsm_network *net;

	const char *default_regexp = ".*";

	net = talloc_zero(ctx, struct gsm_network);
	if (!net)
		return NULL;

	net->subscr_group = talloc_zero(net, struct gsm_subscriber_group);
	if (!net->subscr_group) {
		talloc_free(net);
		return NULL;
	}

	if (gsm_parse_reg(net, &net->authorized_regexp, &net->authorized_reg_str, 1,
			  &default_regexp) != 0)
		return NULL;

	net->subscr_group->net = net;
	net->auto_create_subscr = true;
	net->auto_assign_exten = true;

	net->country_code = country_code;
	net->network_code = network_code;

	INIT_LLIST_HEAD(&net->trans_list);
	INIT_LLIST_HEAD(&net->upqueue);
	INIT_LLIST_HEAD(&net->subscr_conns);

	/* init statistics */
	net->msc_ctrs = rate_ctr_group_alloc(net, &msc_ctrg_desc, 0);

	net->mncc_recv = mncc_recv;
	net->ext_min = GSM_MIN_EXTEN;
	net->ext_max = GSM_MAX_EXTEN;

	net->dyn_ts_allow_tch_f = true;

	return net;
}