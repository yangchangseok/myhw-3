
#include "run.h"
#include "util.h"

void
print_block_list(/* arguments */) {
   int i = 0;
   p_meta cur = base;

   printf("Current block metatdata \n");
   printf("-------------------------------------\n");
   while(cur)
   {
      printf("%d) size: %d | free %d\n", i, cur->size, cur->free);
      if(print_flag && !cur->free )
        printf("%s\n", cur->data);
      cur = cur->next;
      i++;
   }
   printf("\n");
}

