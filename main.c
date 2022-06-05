#include <malloc.h>

typedef struct string_appender_t string_appender_t;

typedef struct string_buffer_t string_buffer_t;
struct string_buffer_t {
	char buffer[100];
	string_buffer_t* next;
}; 

struct string_appender_t {
	string_buffer_t *head;
	string_buffer_t **cur;
	int tot_len;
	int cur_len;
};

void string_appender_init(string_appender_t *str_app)
{
	str_app->head = 0;
	str_app->cur = &str_app->head;
	str_app->tot_len = 0;
	str_app->cur_len = 0;
}

void string_appender_clear(string_appender_t *str_app)
{
	str_app->tot_len = 0;
	str_app->cur_len = 0;
}

void string_appender_close(string_appender_t *str_app)
{
	string_buffer_t *buf = str_app->head;
	while (buf != 0)
	{
		string_buffer_t *cur = buf;
		buf = buf->next;
		free(cur);
	}
}

void string_appender_append(string_appender_t *str_app, char ch)
{
	if (*str_app->cur == 0) {
		*str_app->cur = (string_buffer_t*)malloc(sizeof(string_buffer_t));
		if (str_app->cur == 0) {
			return;
		}
		(*str_app->cur)->next = 0;
	}
	(*str_app->cur)->buffer[str_app->cur_len++] = ch;
	if (str_app->cur_len == 100)
	{
		str_app->cur = &(*str_app->cur)->next;
		str_app->cur_len = 0;
	}
	str_app->tot_len++;
}

char *string_appender_copy(string_appender_t *str_app)
{
	char *result = (char*)malloc((str_app->tot_len + 1)*sizeof(char));
	if (result == 0) {
		return 0;
	}
	int tot_len = str_app->tot_len;
	int cur_pos = 0;
	string_buffer_t *buf = str_app->head;
	for (int i = 0; i < tot_len; i++) {
		result[i] = buf->buffer[cur_pos++];
		if (cur_pos == 100) {
			buf = buf->next;
			cur_pos = 0;
		}
	}
	result[tot_len] = '\0';
	return result;
}

int string_appender_compare(string_appender_t *str_app, const char *s)
{
	int tot_len = str_app->tot_len;
	int cur_pos = 0;
	string_buffer_t *buf = str_app->head;
	for (int i = 0; i < tot_len; i++) {
		int c = *s++ - buf->buffer[cur_pos++];
		if (c != 0) {
			return c;
		}
		if (cur_pos == 100) {
			buf = buf->next;
			cur_pos = 0;
		}
	}
	return 0;
}

typedef enum json_cb_parser_state_t json_parser_state_t;
enum json_cb_parser_state_t {
	json_cb_int,
	json_cb_string,
	json_cb_open_object,
	json_cb_close_object,
	json_cb_open_array,
	json_cb_close_array,
};

typedef struct json_cb_parser_t json_cb_parser_t;
typedef void (*json_cb_parser_callback_t)(json_cb_parser_t *parser, json_parser_state_t state);
struct json_cb_parser_t
{
	int lc;
	string_appender_t string_appender;
	int int_value;
	json_cb_parser_callback_t callback;
	void *data;
};

void json_cb_parser_init(json_cb_parser_t *parser, json_cb_parser_callback_t callback, void *data)
{
	parser->lc = 0;
	string_appender_init(&parser->string_appender);
	parser->callback = callback;
	parser->data = data;
}

void json_cb_parser_close(json_cb_parser_t *parser)
{
	string_appender_close(&parser->string_appender);
}

void json_cb_process(json_cb_parser_t *parser, char ch)
{
#define JSON_NEXT_CH parser->lc = __LINE__; return; case __LINE__:;

	switch(parser->lc) { case 0:

		for (;;) {
			if ('0' <= ch && ch <= '9') {
				parser->int_value = 0;
				do {
					parser->int_value = 10 * parser->int_value + ch - '0';
					JSON_NEXT_CH
				} while ('0' <= ch && ch <= '9');
				parser->callback(parser, json_cb_int);
			}
			else if (ch == '"') {
				string_appender_clear(&parser->string_appender);
				JSON_NEXT_CH
				while (ch != '"') {
					if (ch == '\\') {
						JSON_NEXT_CH
						if (ch == 't')
							ch = '\t';
						else if (ch == 'n')
							ch = '\n';
						else if (ch == 'r')
							ch = '\r';
						string_appender_append(&parser->string_appender, ch);
					}
					else {
						string_appender_append(&parser->string_appender, ch);
					}
					JSON_NEXT_CH
				}
				parser->callback(parser, json_cb_string);				
			}
			else if (ch == '{') {
				parser->callback(parser, json_cb_open_object);
			}
			else if (ch == '}') {
				parser->callback(parser, json_cb_close_object);
			}
			else if (ch == '[') {
				parser->callback(parser, json_cb_open_array);
			}
			else if (ch == ']') {
				parser->callback(parser, json_cb_close_array);
			}
			else {
				// skip
			}
			JSON_NEXT_CH
		}
	}
	
#undef JSON_NEXT_CH
}		


typedef struct category_t category_t;
struct category_t {
	char *slug;
	char *name;
	category_t *next;
};

category_t* category_new() {
	category_t *result = (category_t*)malloc(sizeof(category_t));
	if (result == 0) {
		return 0;
	}
	result->slug = 0;
	result->name = 0;
	result->next = 0;
	return result;
}

void category_print(category_t *category) {
	for (; category != 0; category = category->next) {
		printf("slug '%s', name '%s'\n",
				category->slug == 0 ? "<NULL>" : category->slug, 
				category->name == 0 ? "<NULL>" : category->name);
	}
}
 
void category_free(category_t *category) {

	while (category != 0)
	{
		if (category->slug != 0) {
			free(category->slug);
		}
		if (category->name != 0) {
			free(category->name);
		}
		category_t *next = category->next;
		free(category);
		category = next;
	}
}

typedef struct process_categories_t process_categories_t;
struct process_categories_t {
	int cl;
	category_t **cur;
};




void process_categories(json_cb_parser_t *parser, enum json_cb_parser_state_t state)
{
	process_categories_t *process_categories = (process_categories_t*)(parser->data);
#define NEXT process_categories->cl = __LINE__; return; case __LINE__:;
	
	switch(process_categories->cl) { case 0:
	
		if (state == json_cb_open_array) {
			NEXT
			while (state == json_cb_open_object) {
				*process_categories->cur = category_new();
				NEXT
				while (state == json_cb_string) {
					if (string_appender_compare(&parser->string_appender, "slug") == 0) {
						NEXT
						if (state == json_cb_string) {
							if ((*process_categories->cur) != 0 && (*process_categories->cur)->slug == 0) {
								(*process_categories->cur)->slug = string_appender_copy(&parser->string_appender);
							}
							//printf("slug '%s'\n", slug);
							NEXT
						}
					}
					else if (string_appender_compare(&parser->string_appender, "name") == 0) {
						NEXT
						if (state == json_cb_string) {
							if ((*process_categories->cur) != 0 && (*process_categories->cur)->name == 0) {
								(*process_categories->cur)->name = string_appender_copy(&parser->string_appender);
							}
							//printf("name '%s'\n", name);
							NEXT
						}
					}
				}
				printf("state = %d\n", state);
				if (state == json_cb_close_object) {
					if ((*process_categories->cur) != 0) {
						process_categories->cur = &(*process_categories->cur)->next;
					}
					NEXT
				}
			}
			if (state == json_cb_close_array) {
				NEXT
			}
		}
	}

#undef NEXT
}

void push_string(json_cb_parser_t *parser, const char *s)
{
	while (*s != '\0')
		json_cb_process(parser, *s++);
}

int main(int argc, char *argv[])
{
	category_t *categories = 0;
	
	process_categories_t process_categories_data;
	process_categories_data.cl = 0;
	process_categories_data.cur = &categories;
	
	json_cb_parser_t parser;
	json_cb_parser_init(&parser, process_categories, &process_categories_data);
	
	push_string(&parser, "[ { \"slug\" : \"a\", \"name\", \"a\" }, { \"slug\" : \"b\", \"name\", \"bjhfalksdfhalskdfjhalskdfjhalskdfjhaslkdfjhaslkdfjhaslkfdjhaslkdfjhalskdfjhalskdfhalskdjfhlaskjdfhlaskjfhalskdfhalskdjfhalskdfjhalsdkfhasldkfhasldkfh_end\" } ]");
	
	category_print(categories);
	
	category_free(categories);
}
