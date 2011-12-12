/*
Make sure the app info is truly bad and we don't crash or something.

Copyright 2011 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License version 3, as published 
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <unistd.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib-object.h>
#include "load-app-info.h"
#include "load-app-info.c"

static void
build_db (sqlite3 * db)
{
	/* Create the table */
	int exec_status = SQLITE_OK;
	gchar * failstring = NULL;
	exec_status = sqlite3_exec(db,
	                           "create table usage (application text, entry text, timestamp datetime);",
	                           NULL, NULL, &failstring);
	if (exec_status != SQLITE_OK) {
		g_warning("Unable to create table: %s", failstring);
	}

	/* Import data from the system */

	return;
}

int
main (int argv, char * argc[])
{
	gboolean passed = TRUE;

	if (argv != 3) {
		g_printerr("Usage: %s <db path> <app-info file path>\n", argc[0]);
		return 1;
	}

	g_type_init();

	gchar * filename = argc[1];

	sqlite3 * db = NULL;
	int open_status = sqlite3_open(filename, &db); 

	if (open_status != SQLITE_OK) {
		g_warning("Error opening usage DB: %s", filename);
		passed = FALSE;
		goto cleanup;
	}

	/* Create the table in the DB */
	build_db(db);

	passed = load_app_info(argc[2], db);

cleanup:
	if (db != NULL) {
		sqlite3_close(db);
	}

	if (passed) {
		return 0;
	} else {
		return 1;
	}
}