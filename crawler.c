#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <threads.h>
#include <stdbool.h>
#include <time.h>



/*
 * output chunks
 *
 * The output is synchronised. The synchronisation is done via a ring queue which allows a
 * semi-free locking output. That is, a maximum amount of chunks can be held in the queue
 * before all push calls are locked and held until the queue has at least one entry free.
 */
struct output_chunks
{
	mtx_t barrier;
	cnd_t condition;
	bool closed;
	size_t max;
	size_t index;
	size_t length;
	(const char *) * chunks;
};



/*
 * push output lines
 *
 **/
struct output_chunks * push_output (struct output_chunks * restrict chunks, const char * chunk)
{
	assert(chunks != NULL);
	assert(chunk != NULL);

	mtx_lock(chunks->barrier);
	bool closed = chunks->closed;

	while (chunks->length >= chunks->max && closed == false)
	{
		cnd_wait(chunks->condition, chunks->barrier);
		closed = chunks->closed;
	}

	if (closed == false)
	{
		const size_t max = chunks->max;
		const size_t index = chunks->index;
		const size_t length = chunks->length;
		const size_t next = index + length - (index + length >= max ? max : 0);
		
		
		chunks->chunks[next] = chunk;
		chunks->length++;
		
		cnd_signal(chunks->condition);
	}

	mtx_unlock(chunks->barrier);

	return chunks;
}

/*
 * pop output lines
 *
 * 
 */
void pop_output (struct output_chunks * restrict chunks)
{
	assert (chunks != NULL);
	const char * chunk;
	bool keepGoing = true;

	while (keepGoing)
	{

		mtx_lock(chunks->barrier);
		keepGoing = chunks->closed == false ? true : false;

		while (chunks->length == 0 && keepGoing == true)
		{
			cnd_wait(chunks->condition, chunks->barrier);
			keepGoing = chunks->closed == false ? true : false;
		}

		if (keepGoing == true)
		{
			const size_t max = chunks->max;
			const size_t index = chunks->index;
			
			chunk = chunks->chunks[index++];
			chunks->index = index >= max ? 0 : index; 
			--chunks->length;

			cnd_signal(&(chunks->condition));
		}

		mtx_unlock(chunks->barrier);

		if (keepGoing == true)
			puts(chunk);
	}

}


struct domain
{
	uint64_t * visits;
	size_t visitsLength;
	size_t visitsMax;
	size_t * queuePages;
	size_t beginQueuePage;
	size_t endQueuePage;
	size_t maxQueuePage;
	char * frontPage;
	char * beginFrontPage;
	char * endFrontPage;
	char * backPage;
	char * beginBackPage;
	char * endBackPage;
	char * endFrontPage;
};


inline uint64_t hash_fnv (const char * text, const size_t length)
{
	uint64_t hashValue = 0xcbf29ce484222325;
	for (size_t i = 0; i < length && text[i] != 0; ++i)
	{
		hashValue = (hashValue * 0x100000001b3) ^ ((unsigned char) text[i]);
	}
	return hashValue;
}

bool test_and_set_page_visit (struct domain * d, const char * u, const size_t length)
{
	const uint64_t hash = hash_fnv(u, length);

	bool found = false;
	const size_t length = d->visitsLength;
	const size_t max = d->visitsMax;

	for (size_t i = 0; i < length && found == false; ++i)
	{
		found = hash == d->visits[i] ? true : false;
	}

	if (found == false)
	{
		if (length >= max)
		{
			uint64_t * newVisits = (uint64_t*) malloc(sizeof(uint64_t) * (max + 1024));
			memcpy(newVisit, d->visits, sizeof(uint64_t) * max);
			free(d->visits);
			d->visits = newVisits;
			d->visitsMax = max + 1024;
		}

		d->visits[length] = hash;
		d->visitsLength = length + 1;
	}

	return found;
}





size_t strnsrch (const char * restrict str, const int ch, const size_t n)
{
	size_t i = 0;
	while (i < n && str[i] != 0 && str[i] != ch)
		++i;
	return i;
}

size_t _strsrch (const char * restrict str, const int ch, const size_t n)
{
	size_t i = 0;
	while (i < n && str[i] != ch)
		++i;
	return i;
}

int _strcmp (const char * restrict lhs, const char * restrict rhs, const size_t n)
{
	int comparison = 0;
	size_t i = 0;
	while (i < n && comparison == 0)
	{
		const char lhc = lhs[i];
		const char rhc = rhs[i];

		if (lhc < rhc)      comparison = -1;
		else if (lhc > rhc) comparison = 1;

		++i;
	}
	return comparison;
}



struct queue_node
{
	struct queue_node * next;
	char * begin;
	char * end;
	char * max;
};

/* pops entry from the queue
 *
 */
struct queue_node * pop_queue (struct queue_node * node, char * restrict entry, size_t length)
{
	int nextNode = 1;

	assert (node != NULL);
	assert (length > 0);
	assert (entry != NULL);

	while (nextNode == 1)
	{
		while (node->begin != node->end )
		const size_t nodeRestLength = node->end - node->begin;
		const size_t maxCopyLength = length < nodeRestLength ? length : nodeRestLength;
		strncpy(entry, node->begin, maxCopyLength);
	}

	if (node != NULL)
	{
		const size_t entrySize = node->end - node->begin;
		if (
		strncpy(entry, node->begin, );
	}
	return node;
}

/* allocates memory for a queue node and initialises it
 *
 * Memory for node head (struct queue_node) as well as space for \a chunkSize bytes is allocated
 * from the heap. If the allocation fails, null pointer is returned. For a valid allocation, all
 * but the \a next field is correctly initialised and pointer to the node is returned.
 */
struct queue_node * create_queue_node (const size_t chunkSize)
{
	char * memoryChunk = (char*) malloc(sizeof(struct queue_node) + chunkSize);
	struct queue_node * node = (struct queue_node *) memoryChunk;
	if (node != NULL)
	{
		node->begin = memoryChunk + sizeof(struct queue_node);
		node->end = node->begin;
		node->max = node->begin + chunkSize;
	}
	return node;
}

/* push some url into the queue
 *
 * A sequence, pointed at \a entry with length \a lenth, is pushed into the queue with last node
 * \a node. If some new nodes should be created, they are going to have a capacity of \a chunkSize 
 * respectively. After the push, the last node in the queue is returned.
 */
struct queue_node * push_queue (struct queue_node * node, const char * restrict entry, size_t length, const size_t chunkSize)
{
	assert(chunkSize > 0);
	assert(node != NULL);
	
	while (length > 0)
	{
		const size_t freeLength = node->max - node->end;
		const size_t copyLength = length < freeLength ? length : freeLength;
		memcpy(node->end, entry, copyLength);
		length -= copyLength;
		entry += copyLength;
		node->end += copyLength
		
		if (length == 0)
		{
			if (node->end == node->max)
			{
				node->next = create_queue_node(chunkSize);
				if (node->next == NULL) abort(...);
				node = node->next;
				node->next = NULL;
			}
			*(node->end)++ = 0;
		}
		else
		{
			node->next = create_queue_node(chunkSize);
			if (node->next == NULL) abort(...);
			node = node->next;
			node->next = NULL;
		}
	}
	
	return node;
}

enum html_parsing_state
{
	html_parsing_text,

	// "<!DOCTYPE" S (NC | S | '"' [^"]* '"' | "'" [^']* "'")*
	html_parsing_doctype_content,

	// "<!DOCTYPE" S (NC | S | '"' [^"]* '"' | "'" [^']* "'")* '"' [^"]*
	html_parsing_doctype_content_double_quote,
	
	// "<!DOCTYPE" S (NC | S | '"' [^"]* '"' | "'" [^']* "'")* "'" [^']*/ 
	html_parsing_doctype_content_single_quote,
	
	// "<!DOCTYPE" S (NC | S | '"' [^"]* '"' | "'" [^']* "'")* "[" ([^]"'] | "'" [^']* "'" | '"' [^"]* '"')*
	html_parsing_doctype_subset,

	// "<!DOCTYPE" S (NC | S | '"' [^"]* '"' | "'" [^']* "'")* "[" ([^]"'] | "'" [^']* "'" | '"' [^"]* '"')* '"' [^"]*
	html_parsing_doctype_subset_double_quote,

	// "<!DOCTYPE" S (NC | S | '"' [^"]* '"' | "'" [^']* "'")* "[" ([^]"'] | "'" [^']* "'" | '"' [^"]* '"')* "'" [^']*
	html_parsing_doctype_subset_single_quote,

	// "<!DOCTYPE" S (NC | S | '"' [^"]* '"' | "'" [^']* "'")* "[" ([^]"'] | "'" [^']* "'" | '"' [^"]* '"')* "]" S*
	html_parsing_doctype_after_subset,

	// "<!--" (Char - "-" | "-" (Char - "-") | "--" (Char - ">"))*
	html_parsing_comment_text,

	// "<!--" (Char - "-" | "-" (Char - "-") | "--" (Char - ">"))* "-"
	html_parsing_comment_bar,

	// "<!--" (Char - "-" | "-" (Char - "-") | "--" (Char - ">"))* "--"
	html_parsing_comment_bar_bar,

	// "<?" (Char - "?" | "?" (Char - ">"))* 
	html_parsing_pi,

	// "<?" (Char - "?" | "?" (Char - ">"))* "?"
	html_parsing_pi_qm,

	// "<![CDATA[" (Char - "]" | "]" (Char - "]") | "]]" (Char - ">"))*
	html_parsing_cdata,

	// "<![CDATA[" (Char - "]" | "]" (Char - "]") | "]]" (Char - ">"))* "]"
	html_parsing_cdata_bracket,

	// "<![CDATA[" (Char - "]" | "]" (Char - "]") | "]]" (Char - ">"))* "]]"
	html_parsing_cdata_bracket_bracket,

	// "</"
	html_parsing_closed_tag,

	// "</" NameStartChar NameChar*
	html_parsing_closed_tag_name,

	// "</" Name 
};

struct html_parse
{
	const char * document;
	enum html_parsing_state state;
};


html_parse extract_references (const char * document, const size_t document_length, enum html_parsing_state state,
                               char * url, const size_t maxUrl, char * ...)
{
	assert(document != NULL);

	const char * documentEnd = document + document_length;

	while (document != documentEnd)
	{
		if (state == html_parsing_initial)
		{
			
		}
	}
}



struct http_document
{
	unsigned int majorVersion;
	unsigned int minorVersion;
	unsigned int code;
	size_t bodyLength;
	const char * beginBody;
	const char * beginLocation;
	const char * endLocation;
	const char * beginContentType;
	const char * endContentType;
	const char * beginContentEncoding;
	const char * endContentEncoding;
};

int parse_http_header (const char * restrict buffer, const size_t length, struct http_document * restrict document)
{
	assert(document != NULL);

	int validHeader = 0;
	size_t index = 0;

	// skip empty lines which are "\r\n"
	while (index + 1 < length && buffer[index] == '\r' && buffer[index + 1] == '\n')
		index += 2;

	if (index + 15 < length)
	{
		// first line is response: "HTTP/" Digit+ "." Digit+ " " Digit3 " " Reason CRLF
		const int correctName = !strncmp(buffer + index, "HTTP/", 5);
		const int correctVersion = isdigit(buffer[index + 6]) && (buffer[index + 7] == '.') && isdigit(buffer[index + 8]);
		document->majorVersion = buffer[index + 6] - '0';
		document->minorVersion = buffer[index + 8] - '0';
		const int correctCode = isdigit(buffer[index + 10]) && isdigit(buffer[index + 11]) && isdigit(buffer[index + 12]);
		document->code = (buffer[index + 10] - '0') * 100 + (buffer[index + 11]) * 10 + (buffer[index + 12]);
		document->beginLocation = NULL;
		document->endLocation = NULL;
		document->beginContentType = NULL;
		document->endContentType = NULL;
		document->beginContentEncoding = NULL;
		document->endContentEncoding = NULL;
		document->bodyLength = 0;

		index = index + 15 + _strsrch(buffer + index + 14, '\n', length - index - 14);

		while (index < length && validHeader == 0)
		{
			const size_t lineLength = _strsrch(buffer + index, '\r', length - index);

			if (lineLength > 0)
			{
				const size_t fieldNameLength = _strsrch(buffer + index, ':', lineLength);
				const int isLocation = !strncmp(buffer + index, "Location", fieldNameLength);
				const int isContentType = !strncmp(buffer + index, "Content-Type", fieldNameLength);
				const int isContentEncoding = !strncmp(buffer + index, "Content-Encoding", fieldNameLength);
				const int isContentLength = !strncmp(buffer + index, "Content-Length", fieldNameLength);

				if (fieldNameLength < lineLength && isLocation)
				{
					document->beginLocation = buffer + index + 10;
					document->endLocation = buffer + index + lineLength + 1;
				}
				else if (fieldNameLength < lineLength && isContentType)
				{
					document->beginContentType = buffer + index + 14;
					document->endContentType = buffer + lineLength + 1;
				}
				else if (fieldNameLength < lineLength && isContentEncoding)
				{
					document->beginContentEncoding = buffer + index + 14;
					document->endContentEncoding = buffer + lineLength + 1;
				}
				else if (fieldNameLength < lineLength && isContentLength)
				{
					size_t length = 0;
					for (size_t i = 15; i < lineLength; ++i)
					{
						if (isdigit(buffer[index + i]))
							length = length * 10 + (buffer[index + i] - '0')
					}
					document->bodyLength = length;
				}

				index += lineLength + 2;
			}
			else
			{
				validHeader = 1;
			}
		}
		document->beginBody = buffer + index;
	}
	return validHeader;
}


int main (const int argc, const char * argv [])
{
	size_t maxURLLength;

	// read options
	// TODO
	
	// main loop going through the queue which contains urls for crawling
	while (queueFront = pop_queue(queueFront, hostUrl, maxUrlSize))
	{
		hostURL[maxUrlSize] = 0;
				
		struct addrinfo * hostAddress = resolve_url(hostURL, maxUrlSize);
		if (hostAddress != NULL)
		{
		
			// connect to hostAdress
			// TODO

			if ( ... )
			{
			
			}			
		}
	}

}

