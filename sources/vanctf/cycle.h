typedef struct map_s
{
	char	*mapname;
	struct map_s *next;
} map_t;

char *cycle_get_next_map(char *currentmap);
