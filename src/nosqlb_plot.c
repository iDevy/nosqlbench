
/*
 * Copyright (C) 2011 Mail.RU
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <tnt.h>

#include <nosqlb_stat.h>
#include <nosqlb_func.h>
#include <nosqlb_test.h>
#include <nosqlb_opt.h>
#include <nosqlb.h>
#include <nosqlb_plot.h>

static int
nosqlb_plot_data(struct nosqlb *bench, struct nosqlb_test *test)
{
	char file[512];
	snprintf(file, sizeof(file), "%s/bench-%s.data",
		bench->opt->plot_dir, test->func->name);

	FILE *f = fopen(file, "w");
	if (f == NULL)
		return -1;

	char data[2048];
	int pos = 0;
	struct nosqlb_test_buf *b;
	STAILQ_FOREACH(b, &test->list, next) {
		pos  = snprintf(data, sizeof(data), "%d\t", b->buf);
		pos += snprintf(data + pos,
			sizeof(data) - pos, "%f\t", b->avg.rps);
		fputs(data, f);
		fputc('\n', f);
	}
	fclose(f);
	return 0;
}

static int
nosqlb_plot_cfg(struct nosqlb *bench, struct nosqlb_test *test)
{
	char file[512];
	snprintf(file, sizeof(file), "%s/bench-%s.plot",
		bench->opt->plot_dir, test->func->name);

	char *head =
		"set terminal png nocrop enhanced size 800,600 fon Tahoma 8\n"
		"set output '%s'\n"
		"set ytics border in scale 1,0.5 mirror norotate  offset character 0, 0, 0\n"
		"set xtics (%s)\n"
		"set title \"Tarantool '%s' benchmark (count: %d, reps: %d)\"\n"
		"set xlabel \"Buffers\"\n"
		"set ylabel \"Requests\"\n";

	FILE *f = fopen(file, "w");
	if (f == NULL)
		return -1;

	fputs("# autogenerated by nosqlb.\n\n", f);
	snprintf(file, sizeof(file), "bench-%s.png",
		test->func->name);
	fprintf(f, head, file,
		nosqlb_test_buf_list(test),
		test->func->name,
		bench->opt->count, bench->opt->reps);

	char *head_xrange =
		"set xrange [%d : %d] noreverse nowriteback\n\n";

	int bmin = nosqlb_test_buf_min(test),
	    bmax = nosqlb_test_buf_max(test);
	if (bmin == bmax) {
		bmin -= 10;
		bmax += 10;
	}

	fprintf(f, head_xrange, bmin, bmax);
	snprintf(file, sizeof(file), "bench-%s.data", test->func->name);

	char plotfmt[512];
	snprintf(plotfmt, sizeof(plotfmt),
		"plot '%s' using 1:2 with lines t \"%s\", "
		"'%s' using 1:2:2 with labels notitle\n",
			file, test->func->name, file);
	fputs(plotfmt, f);
	fputs("\n# vim: syntax=gnuplot", f);
	fclose(f);
	return 0;
}

int
nosqlb_plot(struct nosqlb *bench)
{
	if (bench->opt->plot_dir) {
		struct stat st;
		if (stat(bench->opt->plot_dir, &st) == -1) {
			if (mkdir(bench->opt->plot_dir, 0755) == -1)
				return -1;
		}
	}
	struct nosqlb_test *t;
	STAILQ_FOREACH(t, &bench->tests.list, next) {
		if (nosqlb_plot_data(bench, t) == -1)
			return -1;
		if (nosqlb_plot_cfg(bench, t) == -1)
			return -1;
	}
	return 0;
}
