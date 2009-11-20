#ifndef CONFIG_HEADER
#define CONFIG_HEADER

/****************************************************************************
    This is a generic config file read utility.  The config file must
    have the format of
    {
        <key1> <value1>
        <key2> <value2>
        <key3> <value3>
        ....
    }
    where the list of valid keys are supplied by the user of this package.
****************************************************************************/

/****************************************************************************
    List of valid key value types
****************************************************************************/
typedef enum cfg_type_enum {
    CFG_TYPE_INT32,
    CFG_TYPE_INT16,
    CFG_TYPE_INT8,
    CFG_TYPE_STR
} cfg_type_e;

/****************************************************************************
    Struct used to define one "key" "value" pair in the config file
****************************************************************************/
typedef struct cfg_entry_struct {
    char *key;
    cfg_type_e type;
    void *value;
    int min_value;
    int max_value;
} cfg_entry_t;

extern int cfg_read(cfg_entry_t *entries, int num_entries, char *fname);
extern int cfg_print(FILE *fp, cfg_entry_t *entries, int num_entries);

#endif
