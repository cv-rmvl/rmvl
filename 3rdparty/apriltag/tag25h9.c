/* Copyright (C) 2013-2016, The Regents of The University of Michigan.
All rights reserved.
This software was developed in the APRIL Robotics Lab under the
direction of Edwin Olson, ebolson@umich.edu. This software may be
available under alternative licensing terms; contact the address above.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the Regents of The University of Michigan.
*/

#include <stdlib.h>
#include "tag25h9.h"

static uint64_t codedata[36] = {
   0x0000000000f38f39UL,
   0x0000000000c81ec9UL,
   0x0000000000e23ec8UL,
   0x0000000000e2ae29UL,
   0x000000000028e9a6UL,
   0x0000000001e4ae4bUL,
   0x0000000000e4ae3bUL,
   0x0000000001e22280UL,
   0x0000000000e2ae2bUL,
   0x0000000000e3ae0bUL,
   0x000000000051f073UL,
   0x0000000001e2ae7bUL,
   0x0000000000e28e38UL,
   0x0000000001e38e78UL,
   0x0000000001e43e7bUL,
   0x0000000001e4207bUL,
   0x0000000000e1ae39UL,
   0x000000000107b07bUL,
   0x0000000001ec1ec1UL,
   0x0000000000078e30UL,
   0x000000000104517bUL,
   0x0000000001001e78UL,
   0x000000000117d079UL,
   0x0000000001179179UL,
   0x0000000000e38e38UL,
   0x0000000001e2207bUL,
   0x0000000000e31738UL,
   0x0000000001e2317bUL,
   0x0000000000e4ae4bUL,
   0x0000000001ec0481UL,
   0x0000000001078e38UL,
   0x000000000107051cUL,
   0x0000000001078ab9UL,
   0x0000000001145145UL,
   0x0000000001144481UL,
   0x0000000001e45e45UL,
};
apriltag_family_t *tag25h9_create()
{
   apriltag_family_t *tf = calloc(1, sizeof(apriltag_family_t));
   tf->name = strdup("tag25h9");
   tf->h = 9;
   tf->ncodes = 36;
   tf->codes = codedata;
   tf->nbits = 25;
   tf->bit_x = calloc(25, sizeof(uint32_t));
   tf->bit_y = calloc(25, sizeof(uint32_t));
   tf->bit_x[0] = 1;
   tf->bit_y[0] = 1;
   tf->bit_x[1] = 2;
   tf->bit_y[1] = 1;
   tf->bit_x[2] = 3;
   tf->bit_y[2] = 1;
   tf->bit_x[3] = 4;
   tf->bit_y[3] = 1;
   tf->bit_x[4] = 2;
   tf->bit_y[4] = 2;
   tf->bit_x[5] = 3;
   tf->bit_y[5] = 2;
   tf->bit_x[6] = 5;
   tf->bit_y[6] = 1;
   tf->bit_x[7] = 5;
   tf->bit_y[7] = 2;
   tf->bit_x[8] = 5;
   tf->bit_y[8] = 3;
   tf->bit_x[9] = 5;
   tf->bit_y[9] = 4;
   tf->bit_x[10] = 4;
   tf->bit_y[10] = 2;
   tf->bit_x[11] = 4;
   tf->bit_y[11] = 3;
   tf->bit_x[12] = 5;
   tf->bit_y[12] = 5;
   tf->bit_x[13] = 4;
   tf->bit_y[13] = 5;
   tf->bit_x[14] = 3;
   tf->bit_y[14] = 5;
   tf->bit_x[15] = 2;
   tf->bit_y[15] = 5;
   tf->bit_x[16] = 4;
   tf->bit_y[16] = 4;
   tf->bit_x[17] = 3;
   tf->bit_y[17] = 4;
   tf->bit_x[18] = 1;
   tf->bit_y[18] = 5;
   tf->bit_x[19] = 1;
   tf->bit_y[19] = 4;
   tf->bit_x[20] = 1;
   tf->bit_y[20] = 3;
   tf->bit_x[21] = 1;
   tf->bit_y[21] = 2;
   tf->bit_x[22] = 2;
   tf->bit_y[22] = 4;
   tf->bit_x[23] = 2;
   tf->bit_y[23] = 3;
   tf->bit_x[24] = 3;
   tf->bit_y[24] = 3;
   tf->width_at_border = 7;
   tf->total_width = 9;
   tf->reversed_border = false;
   return tf;
}

void tag25h9_destroy(apriltag_family_t *tf)
{
   free(tf->bit_x);
   free(tf->bit_y);
   free(tf->name);
   free(tf);
}