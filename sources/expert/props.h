/**
 * Prototypes for the properties system
 *
 * A "property" is a "key" and "value" pair, where both 
 * key and value are strings.  For example,
 * "dog" "bowl"
 *
 * This subsystem defines a struct for holding a set of
 * properties, and various operation on that set of properties.
 */

typedef struct {
	list_t keys;
	list_t values;
} props_t;

props_t *newProps();
void freeProps(props_t *props);
void printProps(props_t *props);
char *getProp(props_t *props, char* getKey);
void addProp(props_t *props, char* newKey, char* newValue);
void removeProp(props_t *props, char* deleteKey);

