/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#include "lwip/def.h"
#include "fs.h"
#include "fsdata.h"
#include "fsdata.c"
#include <io.h>
#include <minilib.h>
#include <paging.h>
#include <demap.h>
#include <output.h>
#include <state.h>

static char http_output_buffer[1280];

/*-----------------------------------------------------------------------------------*/

void handle_regs(struct fs_file *file)
{
  int i;
  int len;

  len = snprintf(http_output_buffer, sizeof(http_output_buffer), "<html><pre>");

  for (i = 0; i < NUM_REGISTERS; i++) {
    len += state_dump_reg(http_output_buffer + len, sizeof(http_output_buffer) - len, i);
  }

  file->len = len;
  file->data = http_output_buffer;
}

#define LEFT (sizeof(http_output_buffer) - len)

void handle_backtrace(struct fs_file *file)
{
  int i = 10;
  int len;
  void *pebp, *peip;
  uint64_t bp, next;

  int longmode = (get_operating_mode() == LONG_64BIT);

  char * buf = http_output_buffer;
  
  strcpy(buf, "<html><head><title>Backtrace</title></head><body><tt><pre>");
  len = strlen(buf);

  bp = state_get_reg(STATE_REG_RIP);

  if (longmode)
    len += snprintf(buf + len, LEFT, "0x%08x%08x, from\n", (uint32_t)(bp >> 32), (uint32_t)bp);
  else
    len += snprintf(buf + len, LEFT, "0x%08x, from\n", (uint32_t)bp);
  
  bp = state_get_reg(STATE_REG_RBP);
  
  /* I never thought I'd do this again. */
  while ((peip = demap(bp+(longmode?8:4))) != 0x0 && i--)
  {
    if (longmode) {
      next = *(uint64_t *)peip;
      len += snprintf(buf + len, LEFT, "0x%08x%08x, from\n", (uint32_t)(next >> 32), (uint32_t)next);
    } else {
      next = *(uint32_t *)peip;
      len += snprintf(buf + len, LEFT, "0x%08x, from\n", (uint32_t)next);
    }

    pebp = demap(bp);

    if (!pebp)
    {
      len += snprintf(buf + len, LEFT, "&lt;unreadable frame&gt;\n");
      break;
    }

    if (longmode)
      next = *(uint64_t *)pebp;
    else
      next = *(uint32_t *)pebp;

    if (bp >= next && next)
    {
      len += snprintf(buf + len, LEFT, "&lt;recursive frame&gt;\n");
      break;
    }

    bp = next;
  }

  if (i == -1)
    len += snprintf(buf + len, LEFT, "...\n");
  else
    len += snprintf(buf + len, LEFT, "&lt;root&gt;");

  len += snprintf(buf + len, LEFT, "</pre></tt></body></html>");
  
  file->data = buf;
  file->len = len;
}

void handle_reboot(struct fs_file *file)
{
  outb(0xCF9, 0x4);
  file->data = "So long!";
  file->len = 8;
}


/*-----------------------------------------------------------------------------------*/
int
fs_open(const char *name, struct fs_file *file)
{
  const struct fsdata_file *f;
  
  /* /registers.html is CGI */
  if (!strcmp(name, "/registers.html"))
  {
    handle_regs(file);
    return 1;
  }
  if (!strcmp(name, "/backtrace.html"))
  {
    handle_backtrace(file);
    return 1;
  }
  if (!strcmp(name, "/reboot"))
  {
    handle_reboot(file);
    return 1;
  }

  for(f = FS_ROOT;
      f != NULL;
      f = f->next) {
    if (!strcmp(name, (const char*)f->name)) {
      file->data = f->data;
      file->len = f->len-1;
      return 1;
    }
  }
  file->data = "You clown...";
  file->len = 9;
  return 0;
}
/*-----------------------------------------------------------------------------------*/
