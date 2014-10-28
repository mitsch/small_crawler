/**
 * @file external_queue.c
 * @author Michael Koch
 * @copyright CC BY 3.0
 **/

#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

struct external_queue_action
{
	char * chunk;
	size_t size;
	unsigned int mode;
};

struct external_queue
{
	FILE * descriptor;
	size_t chunkSize;
	size_t chunkAmount;
	unsigned char * occupied;
	
	mtx_t guardActionQueue;
	cnd_t condActionQueue;
	ex
};

void handle_disk (struct external_queue * queue)
{
	assert(queue != NULL);

	
}


size_t push_external_queue (struct external_queue * queue, const char * chunk)
{
	mtx_lock(queue->guard);

	// find some free chunk
	size_t iChunk = 0;
	while (iChunk < queue->chunkAmount && (queue->occupied[iChunk] & 0x01 == 0))
		++iChunk;

	if (iChunk == queue->chunkAmount)
	{
		// we need more chunks
		unsigned char * newOccupied = (unsigned char*) malloc(queue->chunkAmount + 512);
		assert(newOccupied != NULL);

		memcpy(newOccupied, queue->occupied, queue->chunkAmount);
		memset(newOccupied + queue->chunkAmount, 0, 512);

		
	}

	mtx_unlock(queue->guard);
}

