#include <sys/types.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "run.h"
#include "util.h"
#include "hexdump.h"
void *base = 0;

p_meta find_meta(p_meta *last, size_t size) {
    int lastsize = -1;
    p_meta index = base;
    p_meta result = NULL;
    size += META_SIZE;

    switch(fit_flag){
        case FIRST_FIT:
            {
                while(index)
                {
                    if(index->free && index->size >= size)
                    {
                        result = index;
                        break;
                    }
                    index = index->next;
                }
            }
            break;

        case BEST_FIT:
            {
                while(index)
                {
                    if(index->free && index->size >= size && ((lastsize == -1) || (lastsize > index->size)))
                    {
                        lastsize = index->size;
                        result = index;
                    }
                    index = index->next;
                }
            }
            break;

        case WORST_FIT:
            {
                while(index)
                {
                    if(index->free && index->size >= size && ((lastsize == -1) || (lastsize < index->size)))
                    {
                        lastsize = index->size;
                        result = index;
                    }
                    index = index->next;
                }
            }
            break;
    }
    return result;
}

void *m_malloc(size_t size) {
    if(size % 4)
    {
        size = size + (4 - size % 4);
    }
    p_meta newMeta = sbrk(META_SIZE + size), target, last, fitMeta;
    newMeta->size = size;
    newMeta->next = newMeta->prev = 0;
    newMeta->free = 0;
    last = base;

    if(base == 0)
    {
        base = newMeta;
    }
    else
    {
        if(last->next != 0)
        {
            while(last->next)
            {
                last = last->next;
            }
        }
        target = find_meta(base, size);
        if(target == 0)
        {
            if(last->next == 0)
            {
                last->next = newMeta;
                newMeta->prev = last;
            }
            else
            {
                newMeta->next = last->next;
                newMeta->prev = last;
                last->next = newMeta;
            }
        }
        else
        {
            target->free = 0;
            memset(target->data, 0x00, size);
            if(target->size <= size + META_SIZE)
            {
                return target->data;
            }
            fitMeta = target + size + META_SIZE;
            fitMeta->size = target->size - size - META_SIZE;
            fitMeta->prev = target;
            fitMeta->next = target->next;
            fitMeta->free = 1;
            target->size = size;
            target->next = fitMeta;
            return target->data;
        }
    }
    memset(newMeta->data, 0x00, size);
    return newMeta->data;
}

void m_free(void *ptr) {
    int i;
    p_meta target = base;
    while(target)
    {
        if(target->data == ptr)
        {
            break;
        }
        target = target->next;
    }
    if(target->data == ptr)
    {
        target->free = 1;
        if(target->next == 0)
        {
            if(target->prev)
            {
                target->prev->next = 0;
            }
            return;
        }
        if(target->prev && target->prev->free)
        {
            target = target->prev;
        }
        while(target->next && target->next->free) //재조립
        {
            target->size += target->next->size + META_SIZE;
            target->next = target->next->next;
            if(target->next)
            {
                target->next->prev = target;
            }
        }
    }
}

void* m_realloc(void* ptr, size_t size)
{
    p_meta target = base, newMeta;
    struct metadata backup;
    int i, prevsize;
    prevsize = size;
    if(size % 4)
    {
        size = size + (4 - size % 4);
    }
    while(target)
    {
        if(target->data == ptr)
        {
            break;
        }
        target = target->next;
    }
    if(target->data == ptr)
    {
        if(target->size < size)
        {
            if(!target->next || target->next->size < size - target->size)
            {
                newMeta = m_malloc(size);
                target->next = newMeta;
                newMeta->prev = target;
                newMeta->size = size;
                newMeta->free = 0;
                for(i=0; i<target->size; i++)
                {
                    newMeta->data[i] = target->data[i];
                }
                m_free(ptr);
                return newMeta->data;
            }
            else
            {
                backup.size = target->next->size;
                backup.next = target->next->next;
                backup.prev = target->next->prev;
                backup.free = target->next->free;
                backup.size -= size - target->size;
                target->next = target->next + (size - target->size);
                target->next->size = backup.size;
                target->next->next = backup.next;
                target->next->prev = backup.prev;
                target->next->free = backup.free;
                target->size = size;
                return target->next->data;
            }
        }
        else
        {
            if(!target->next && (target->size - size) < META_SIZE)
            {
                for(i=prevsize; i<target->size; i++)
                {
                    target->data[i] = '\0';
                }
                return target->data;
            }
            newMeta = target->data + size;
            newMeta->next = target->next;
            newMeta->prev = target;
            newMeta->free = 1;
            newMeta->size = target->size - size - META_SIZE;
            target->next = newMeta;
            target->size = size;
            target->data[prevsize] = '\0';
            return newMeta->data;
        }
    }
    else
    {
        return NULL;
    }
}


