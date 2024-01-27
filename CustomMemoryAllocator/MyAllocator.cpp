#include <cstdint>
#include <utility>
#include <cassert>
#include <iostream>
#include <windows.h>

using word_t = intptr_t;

struct Block {
	size_t size;

	bool used;

	Block* prev;

	Block* next;

	word_t data[1];
};

class MyAllocator {
public:
	inline size_t allocSize(size_t size) {
		return size + sizeof(Block) - sizeof(std::declval<Block>().data);
	}

	inline size_t align(size_t n) {
		return (n + sizeof(word_t) - 1) & ~(sizeof(word_t) - 1);
	}

	Block* getHeader(word_t* data) {
		return (Block*)((char*)data + sizeof(std::declval<Block>().data) - sizeof(Block));
	}

	word_t* alloc(size_t size) {
		size = align(size);

		auto temp = this->blockStart;
		while (temp != NULL) {
			if (!temp->used && temp->size >= size) {
				temp->used = true;
				return temp->data;
			}
			temp = temp->next;
		}

		auto block = requestFromOS(size);

		if (block == NULL)
			return NULL;

		block->size = size;
		block->used = true;

		// Create "linkedList of block"
		if (this->blockStart == NULL) {
			this->blockStart = block;
			this->blockStart->prev = NULL;
			this->blockStart->next = NULL;
			this->blockEnd = block;
		}
		else {
			// chain blocks together
			if (this->blockEnd != NULL) {
				this->blockEnd->next = block;
			}
			block->prev = this->blockEnd;
			this->blockEnd = block;
		}
		
		return block->data;
	}

	Block* requestFromOS(size_t size) {
		auto mem = (Block*)VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);

		return mem;
	}

	void immediate_coalesing(Block* currentBlock) {
		auto next = currentBlock->next;
		auto prev = currentBlock->prev;

		if (next != NULL && !next->used) {
			currentBlock->size += next->size;
			currentBlock->next = next->next;
		}
		
		if (prev != NULL && !prev->used) {
			prev->size += currentBlock->size;
			prev->next = next;
		}
	}

	void free(word_t* size) {
		auto block = getHeader(size);
		//auto success = VirtualFree(block, 0, MEM_RELEASE);
		block->used = false;

		this->immediate_coalesing(block);
		//if (!success) {
		//	std::cout << "You fucked up..." << "\n";
		//}
	}

private:
	Block* blockStart;
	Block* blockEnd;
};

int main(int argc, char const* argv[]) {

	// --------------------------------------
	// Test case 1: Alignment
	//
	// A request for 3 bytes is aligned to 8.
	//
	
	MyAllocator allocator = MyAllocator();

	auto p1 = allocator.alloc(3);                        // (1)
	auto p1b = allocator.getHeader(p1);
	assert(p1b->size == sizeof(word_t));

	// --------------------------------------
	// Test case 2: Exact amount of aligned bytes
	//

	auto p2 = allocator.alloc(8);                        // (2)
	auto p2b = allocator.getHeader(p2);
	assert(p2b->size == 8);

	auto p3 = allocator.alloc(16);
	auto p3b = allocator.getHeader(p3);

	allocator.free(p2);
	allocator.free(p3);

	auto p4 = allocator.alloc(24);
	auto p4b = allocator.getHeader(p4);


	puts("\nAll assertions passed!\n");
}