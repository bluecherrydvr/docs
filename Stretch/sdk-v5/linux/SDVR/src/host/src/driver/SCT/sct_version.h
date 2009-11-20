#define SCT_MAJOR_VERSION 5
#define SCT_MINOR_VERSION 0
#define SCT_MINUS_VERSION 1
#define SCT_BUILD_VERSION 0

/*when modify version number, make sure the SCT_VERION STRING matches the individual numbers*/
#define SCT_VERSION_STRING "5.0.1.0\0"


#define SCT_VERSION ((SCT_BUILD_VERSION << 24) | \
                     (SCT_MINUS_VERSION << 16) | \
                     (SCT_MINOR_VERSION << 8)  | \
                     (SCT_MAJOR_VERSION) )

