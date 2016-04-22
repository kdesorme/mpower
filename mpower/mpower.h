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
#ifndef _MPOWER_H
#define _MPOWER_H

struct mPowerDev;

#define MPOWER_NUM_OUTPUTS 6

struct mPowerStatus {
	int port;
	int relay;
	char *id;
	char *label;
	double power;
	double energy;
	double current;
	double voltage;
	double powerfactor;
};

struct mPowerDev *mPowerInit(const char *);
void mPowerEnd(struct mPowerDev *);
int mPowerLogin(struct mPowerDev *, const char *, const char *);
int mPowerSetOutput(struct mPowerDev *, int, int);
int mPowerQueryOutputs(struct mPowerDev *);
void mPowerPrintOutputs(struct mPowerDev *, FILE *);
void mPowerGetOutputs(struct mPowerDev *, struct mPowerStatus *);

#endif
