/*
 * Copyright (c) 2014 CNRS/LAAS
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <stdio.h>
#include <mpower/mpower.h>

int
main(int argc, char *argv[])
{
	struct mPowerDev *dev;
	int retval = 0;
	char *host = "mpower";

	if (argc > 1)
		host = argv[1];
	dev = mPowerInit(host);
	if (dev == NULL)
		return 1;

	if (mPowerLogin(dev, "ubnt", "ubnt") != 0) {
		retval = 1;
		goto fail;
	}
	printf("login ok\n");
	if (mPowerSetOutput(dev, 3, 1) != 0) {
		retval = 1;
		goto fail;
	}
	sleep(5);
	if (mPowerQueryOutputs(dev) != 0) {
		retval = 1;
		goto fail;
	}
	mPowerPrintOutputs(dev, stdout);
	if (mPowerSetOutput(dev, 3, 0) != 0) {
		retval = 1;
		goto fail;
	}
fail:
	mPowerEnd(dev);
	return retval;
}
