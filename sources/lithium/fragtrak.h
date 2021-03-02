
typedef struct trak_time_t {
	int year, month, day, hour, minute, second;
} trak_time_t;

typedef struct trak_header_t {
	char magic[4];
	int version;
	char mapname[64];
	int traks, frags;
	trak_time_t begin, end;
} trak_header_t;

typedef struct trak_t {
	char netname[16];
	float begin, end;
	int ping_total;
	int ping_count;
} trak_t;

typedef struct frag_t {
	char killer, killee;
} frag_t;

