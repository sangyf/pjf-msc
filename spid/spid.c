/*
 * spid: Statistical Packet Inspection
 * Copyright (C) 2011 Paweł Foremski <pawel@foremski.pl>
 * This software is licensed under GNU GPL version 3
 */

#include <sys/time.h>
#include <libpjf/lib.h>
#include <event2/event.h>

#include "settings.h"
#include "datastructures.h"
#include "spid.h"
#include "source.h"
#include "ep.h"
#include "flow.h"

/** Setup default options */
static void _options_defaults(struct spid *spid)
{
	spid->options.N = SPI_DEFAULT_N;
	spid->options.P = SPI_DEFAULT_P;
	spid->options.C = SPI_DEFAULT_C;
}

/** Setup options according to command-line */
static void _options_argv(struct spid *spid, int argc, const char *argv[])
{
	/* TODO */
}

/** Garbage collector */
static void _gc(int fd, short evtype, void *arg)
{
	struct spid *spid = arg;
	const char *key;
	struct flow *flow;
	struct ep *ep;
	struct timeval timeout;

	dbg(3, "garbage collector\n");

	/* TODO: compute timeout moment */
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	thash_iter_loop(spid->flows, key, flow) {
		if (!timercmp(&flow->last, &timeout, >)) {
			/* TODO: destroy flow (lookout for the iterator) */
		}
	}

	thash_iter_loop(spid->eps, key, ep) {
		if (!timercmp(&ep->last, &timeout, >)) {
			/* TODO: destroy ep (lookout for the iterator) */
		}
	}
}

/** Handler for new spid events */
static void _new_spid_event(int fd, short evtype, void *arg)
{
	struct spid_event *se = arg;
	struct spid_subscriber *ss;

	tlist_iter_loop(se->spid->subscribers[se->code], ss) {
		ss->handler(se->spid, se->code, se->data);
	}

	mmatic_freeptr(se);
}

/*******************************/

struct spid *spid_init(int argc, const char *argv[], struct spid_options *so)
{
	int i;
	mmatic *mm;
	struct spid *spid;
	struct timeval tv;

	/* data structure */
	mm = mmatic_create();
	spid = mmatic_zalloc(mm, sizeof *spid);
	spid->mm = mm;
	spid->eb = event_base_new();
	spid->sources = tlist_create(source_destroy, mm);
	spid->eps = thash_create_strkey(ep_destroy, mm);
	spid->flows = thash_create_strkey(flow_destroy, mm);

	for (i = 0; i < SPI_EVENT_MAX; i++)
		spid->subscribers[i] = tlist_create(mmatic_freeptrs, mm);

	/* options */
	if (so)
		memcpy(&spid->options, so, sizeof *so);
	else
		_options_defaults(spid);

	_options_argv(spid, argc, argv);

	/*
	 * setup events
	 */

	/* garbage collector */
	tv.tv_sec = SPI_GC_INTERVAL;
	tv.tv_usec = 0;
	event_add(event_new(spid->eb, -1, EV_PERSIST, _gc, spid), &tv);

	/* TODO: read sources */

	/* TODO: statistics / diagnostics? */

	return spid;
}

int spid_loop(struct spid *spid)
{
	return event_base_loop(spid->eb, EVLOOP_ONCE);
}

void spid_announce(struct spid *spid, spid_event_t code, void *data, uint32_t delay_ms)
{
	struct spid_event *se;
	struct timeval tv;

	se = mmatic_alloc(spid->mm, sizeof *se);
	se->spid = spid;
	se->code = code;
	se->data = data;

	tv.tv_sec  = delay_ms / 1000;
	tv.tv_usec = (delay_ms % 1000) * 1000;

	/* XXX: queue instead of instant handler call */
	event_base_once(spid->eb, -1, 0, _new_spid_event, se, &tv);
}

void spid_subscribe(struct spid *spid, spid_event_t code, spid_event_cb_t *cb)
{
	struct spid_subscriber *ss;

	ss = mmatic_alloc(spid->mm, sizeof *ss);
	ss->handler = cb;

	tlist_push(spid->subscribers[code], ss);
}

/*
 * vim: path=.,/usr/include,/usr/local/include,~/local/include
 */
