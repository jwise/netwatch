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
#include <output.h>

/*-----------------------------------------------------------------------------------*/

void handle_regs(struct fs_file *file)
{
  static char buf[2048];
  
  sprintf(buf,
    "<html><head><title>Registers</title></head><body>"
    "<p>At the time you requested this page, the system's registers were:</p>"
    "<tt><pre>"
    "%%eax: 0x%08x    %%ebx: 0x%08x    %%ecx: 0x%08x    %%edx: 0x%08x\n"
    "%%ebp: 0x%08x    %%esi: 0x%08x    %%edi: 0x%08x    %%esp: 0x%08x\n"
    "%%cr0: 0x%08x    %%cr3: 0x%08x    %%eip: 0x%08x    %%eflags: 0x%08x\n"
    "</pre></tt></body></html>",
    *(unsigned long*)0xAFFD0,
    *(unsigned long*)0xAFFDC,
    *(unsigned long*)0xAFFD4,
    *(unsigned long*)0xAFFD8,
    *(unsigned long*)0xAFFE4,
    *(unsigned long*)0xAFFE8,
    *(unsigned long*)0xAFFEC,
    *(unsigned long*)0xAFFE0,
    *(unsigned long*)0xAFFFC,
    *(unsigned long*)0xAFFF8,
    *(unsigned long*)0xAFFF0,
    *(unsigned long*)0xAFFF4);
    
  
  file->data = buf;
  file->len = strlen(buf)-1;
}

void handle_backtrace(struct fs_file *file)
{
  static char buf[2048];
  static char buf2[64];
  int i = 10;
  unsigned long *pebp, *peip;
  unsigned long ebp;
  unsigned long cr3;
  
  strcpy(buf, "<html><head><title>Backtrace</title></head><body><tt><pre>");
  ebp = *(unsigned long *)0xAFFE4;
  cr3 = *(unsigned long *)0xAFFF8;
  
  sprintf(buf2, "0x%08x, from\n", *(unsigned long*)0xAFFF0);
  strcat(buf, buf2);
  
  /* I never thought I'd do this again. */
  while ((peip = demap(cr3, ebp+4)) != 0x0 && i--)
  {
    sprintf(buf2, "0x%08x, from\n", *peip);
    strcat(buf, buf2);
    pebp = demap(cr3, ebp);
    if (!pebp)
    {
      strcat(buf, "&lt;unreadable %ebp&gt;\n");
      break;
    }
    if (ebp >= *pebp && *pebp)
    {
      strcat(buf, "&lt;recursive %ebp&gt;\n");
      break;
    }
    ebp = *pebp;
  }
  if (i == -1)
    strcat(buf, "...\n");
  else
    strcat(buf, "&lt;root&gt;");
  strcat(buf, "</pre></tt></body></html>");
  
  file->data = buf;
  file->len = strlen(buf)-1;
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
