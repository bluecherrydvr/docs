#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cfg.h"

/****************************************************************************
    This routine reads the configuration information from "fname".
    The list of valid keys are defined in "entries".  It returns
    -2 if "fname" is not readable, -1 if file contrains entries not
    defined in "entries", and 0 if "fname" is successfully read.
****************************************************************************/
int 
cfg_read(cfg_entry_t *entries, int num_entry, char *fname)
{
    FILE *fp;
    char key[512], value[512];
    int i, found, error;

    fp = fopen(fname, "r");
    if (fp == 0) {
        fprintf(stderr, "CFG Error: cannot read %s\n", fname);
        return -2;
    }

	error = 0;
    while (!feof(fp)) {
        if (fscanf(fp, "%s %s", key, value) == 2) {
            found = 0;
            for (i = 0; i < num_entry; i++) {
                if (strcmp(entries[i].key, key) == 0) {
                    found = 1;
                    switch (entries[i].type) {
                    case CFG_TYPE_STR:
                        strncpy(entries[i].value, value, strlen(entries[i].value));
                        break;
                    case CFG_TYPE_INT32:
                        *((int *) entries[i].value) = atoi(value);
                        break;
                    case CFG_TYPE_INT16:
                        *((short *) entries[i].value) = atoi(value);
                        break;
                    case CFG_TYPE_INT8:
                        *((char *) entries[i].value) = atoi(value);
                        break;
                    default:
                        fprintf(stderr, "Unknown entry type: %d\n", entries[i].type);
                    }
					break;
                }
            }
			if (!found) {
				error = 1;
			    fprintf(stderr, "key %s not defined\n", key);
			}
        }
    }

    fclose(fp);
	return error;
}

/****************************************************************************
    Print "entries" to "fp".
****************************************************************************/
int 
cfg_print(FILE *fp, cfg_entry_t *entries, int num_entries)
{
    int i;

    fprintf(fp, "----------------------------------------------------------\n");
    for (i = 0; i < num_entries; i++) {
	    fprintf(fp, "%-20s ", entries[i].key);
		switch (entries[i].type) {
		case CFG_TYPE_STR:
			fprintf(fp, "%s\n", (char *) entries[i].value);
			break;
		case CFG_TYPE_INT32:
			fprintf(fp, "%d\n", *((int *) entries[i].value));
			break;
		case CFG_TYPE_INT16:
			fprintf(fp, "%d\n", *((short *) entries[i].value));
			break;
		case CFG_TYPE_INT8:
			fprintf(fp, "%d\n", *((char *) entries[i].value));
			break;
		}
	}
    fprintf(fp, "----------------------------------------------------------\n");

	return 0;
}
