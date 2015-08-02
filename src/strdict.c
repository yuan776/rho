#include <stdlib.h>
#include <string.h>
#include "object.h"
#include "util.h"
#include "strdict.h"

#define STRDICT_INIT_TABLE_SIZE  32
#define STRDICT_LOAD_FACTOR      0.75f

#define HASH(key)  (secondary_hash(str_hash((key))))

typedef StrDictEntry Entry;

static Entry **make_empty_table(const size_t capacity);
static void strdict_resize(StrDict *dict, const size_t new_capacity);

void strdict_init(StrDict *dict)
{
	dict->table = make_empty_table(STRDICT_INIT_TABLE_SIZE);
	dict->count = 0;
	dict->capacity = STRDICT_INIT_TABLE_SIZE;
	dict->threshold = (size_t)(STRDICT_INIT_TABLE_SIZE * STRDICT_LOAD_FACTOR);
}

static Entry *make_entry(Str *key, const int hash, Value *value)
{
	Entry *entry = rho_malloc(sizeof(Entry));
	entry->key = *key;
	entry->hash = hash;
	entry->value = *value;
	entry->next = NULL;
	return entry;
}

Value strdict_get(StrDict *dict, Str *key)
{
	const int hash = HASH(key);
	for (Entry *entry = dict->table[hash & (dict->capacity - 1)];
	     entry != NULL;
	     entry = entry->next) {

		if (hash == entry->hash && str_eq(key, &entry->key)) {
			return entry->value;
		}
	}

	return makeempty();
}

Value strdict_get_cstr(StrDict *dict, const char *key)
{
	Str key_str = STR_INIT(key, strlen(key), 0);
	return strdict_get(dict, &key_str);
}

void strdict_put(StrDict *dict, const char *key, Value *value, bool key_freeable)
{
	Str key_str = STR_INIT(key, strlen(key), (key_freeable ? 1 : 0));
	Entry **table = dict->table;
	const size_t capacity = dict->capacity;
	const int hash = HASH(&key_str);
	const size_t index = hash & (capacity - 1);

	for (Entry *entry = table[index];
	     entry != NULL;
	     entry = entry->next) {

		if (hash == entry->hash && str_eq(&key_str, &entry->key)) {
			entry->value = *value;
			return;
		}
	}

	Entry *entry = make_entry(&key_str, hash, value);
	entry->next = table[index];
	table[index] = entry;

	if (dict->count++ >= dict->threshold) {
		const size_t new_capacity = 2 * capacity;
		strdict_resize(dict, new_capacity);
		dict->threshold = (size_t)(new_capacity * STRDICT_LOAD_FACTOR);
	}
}

void strdict_put_copy(StrDict *dict, const char *key, size_t len, Value *value)
{
	if (len == 0) {
		len = strlen(key);
	}
	char *key_copy = rho_malloc(len + 1);
	strcpy(key_copy, key);
	strdict_put(dict, key_copy, value, true);
}

void strdict_apply_to_all(StrDict *dict, void (*fn)(Value *v, void *args), void *args)
{
	Entry **tab = dict->table;
	const size_t cap = dict->capacity;
	for (size_t i = 0; i < cap; i++) {
		for (Entry *e = tab[i]; e != NULL; e = e->next) {
			fn(&e->value, args);
		}
	}
}

void strdict_dealloc(StrDict *dict)
{
	Entry **table = dict->table;
	const size_t capacity = dict->capacity;

	for (size_t i = 0; i < capacity; i++) {
		Entry *entry = table[i];
		while (entry != NULL) {
			Entry *temp = entry;
			entry = entry->next;
			release(&temp->value);
			if (temp->key.freeable) {
				str_dealloc(&temp->key);
			}
			free(temp);
		}
	}

	free(table);
}

static Entry **make_empty_table(const size_t capacity)
{
	Entry **table = rho_malloc(capacity * sizeof(Entry *));
	for (size_t i = 0; i < capacity; i++) {
		table[i] = NULL;
	}
	return table;
}

static void strdict_resize(StrDict *dict, const size_t new_capacity)
{
	const size_t old_capacity = dict->capacity;
	Entry **old_table = dict->table;
	Entry **new_table = make_empty_table(new_capacity);

	for (size_t i = 0; i < old_capacity; i++) {
		for (Entry *entry = old_table[i];
		     entry != NULL;
		     entry = entry->next) {

			const size_t index = entry->hash & (new_capacity - 1);
			entry->next = new_table[index];
			new_table[index] = entry;
		}
	}

	free(old_table);
	dict->table = new_table;
	dict->capacity = new_capacity;
}
