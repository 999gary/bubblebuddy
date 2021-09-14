#ifndef MEMORY_ARENA_H
#define MEMORY_ARENA_H

typedef struct memory_arena_block memory_arena_block;
struct memory_arena_block {
    u32 capacity;
    u32 size_used;
    u8 *memory;
    memory_arena_block *prev_block;
    memory_arena_block *next_block;
};

typedef struct {
    u32 block_count;
    memory_arena_block *first_block;
    memory_arena_block *current_block;
} memory_arena;

const u32 MEMORY_ARENA_BLOCK_DEFAULT_SIZE = 1024*1024;

// TODO(jelly): replace malloc with low-level allocator (VirtualAlloc)
memory_arena_block *memory_arena_new_block(u32 size) {
    memory_arena_block *result = (memory_arena_block *)virtual_alloc(sizeof(memory_arena_block) + size);
    if (result) {
        result->memory = (u8 *)result + sizeof(memory_arena_block);
        result->capacity = size;
        result->size_used = 0;
        result->prev_block = 0;
        result->next_block = 0;
    } else {
        assert(!"RAN OUT OF MEMORY!");
    }
    return result;
}

memory_arena memory_arena_new(u32 size) {
    memory_arena result = {0};
    result.first_block = memory_arena_new_block(size);
    result.current_block = result.first_block;
    return result;
}

int memory_arena_is_current_block_big_enough(memory_arena *memory, u32 size) {
    return memory->current_block->size_used + size < memory->current_block->capacity;
}

void memory_arena_ensure_minimum_size(memory_arena *memory, u32 size) {
    if (!memory->current_block) {
        memory->first_block = memory_arena_new_block(Maximum(size, MEMORY_ARENA_BLOCK_DEFAULT_SIZE));
        memory->current_block = memory->first_block;
        memory->block_count = 1;
    }
    
    if (!memory_arena_is_current_block_big_enough(memory, size)) {
        int big_enough_block_found = 0;
        while (memory->current_block->next_block) {
            memory->current_block = memory->current_block->next_block;
            if (memory_arena_is_current_block_big_enough(memory, size)) {
                big_enough_block_found = 1;
                break;
            }
        }
        
        if (!big_enough_block_found) {
            memory->current_block->next_block = memory_arena_new_block(Maximum(size, MEMORY_ARENA_BLOCK_DEFAULT_SIZE));
            memory->current_block->next_block->prev_block = memory->current_block;
            memory->current_block = memory->current_block->next_block;
            memory->block_count++;
        }
    }
}

u8 *memory_arena_alloc(memory_arena *memory, u32 size) {
    memory_arena_ensure_minimum_size(memory, size);
    
    u32 alignment = 8; // TODO(jelly): any reason to change this? prob not?
    u32 padding = size % alignment;
    
    memory_arena_block *c = memory->current_block;
    u8 *result = c->memory + c->size_used;
    c->size_used += size + padding;
    return result;
}

u32 memory_arena_get_size_unused(memory_arena *memory) {
    u32 result = 0;
    if (memory->current_block) {
        result = memory->current_block->capacity - memory->current_block->size_used;
    }
    return result;
}

// NOTE(jelly): this probably won't get called ever.
void memory_arena_free(memory_arena *memory) {
    memory_arena_block *block = memory->first_block;
    while (block) {
        virtual_free(block->memory);
        block = block->next_block;
    }
}

void memory_arena_reset(memory_arena *memory) {
    if (memory->current_block) {
        memory_arena_block *block = memory->first_block;
        while (block) {
            block->size_used = 0;
            block = block->next_block;
        }
    }
}

#define MemoryArenaAllocType(m, type) (type *)memory_arena_alloc(m, sizeof(type))
#define MemoryArenaAllocTypeCount(m, type, count) (type *)memory_arena_alloc(m, (count)*sizeof(type))

typedef struct {
    u32 size_used;
    memory_arena_block *current_block;
} memory_arena_state;

memory_arena_state memory_arena_begin_scratch_memory(memory_arena *memory) {
    memory_arena_state result = {0};
    if (memory->current_block) {
        result.size_used = memory->current_block->size_used;
        result.current_block = memory->current_block;
    }
    return result;
}

void memory_arena_end_scratch_memory(memory_arena *memory, memory_arena_state state) {
    if (state.size_used && state.current_block) {
        memory_arena_block *current = memory->current_block->next_block;
        while (current && current != state.current_block) {
            current->size_used = 0;
            current = current->prev_block;
        }
        memory->current_block = state.current_block;
        memory->current_block->size_used = state.size_used;
    } else {
        memory_arena_reset(memory);
    }
}

#endif //MEMORY_ARENA_H
