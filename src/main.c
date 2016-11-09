/*
 * Copyright (C) 2016  Roel Janssen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <glib.h>

#ifdef ENABLE_MTRACE
#include <mcheck.h>
#endif

#include "nanosvc.h"
#include "breakpoint.h"
#include "segment.h"
#include "read.h"
#include "trie.h"

/* Program-wide configuration variables.  Do not assign new values to these
 * variables.  These variables can be updated at run-time with command-line
 * switches. */
extern struct nsv_config_t nsv_config;

static void
show_version ()
{
  printf ("Version: %s\n", VERSION);
}

static void
show_help ()
{
  puts ("\nAvailable options:\n"
        " --max-threads, -m   Maximum number of threads to use.\n"
        " --split,       -s   Maximum number of segments per read.\n"
        " --distance,    -d   Maximum distance to cluster SVs together.\n"
        " --min-pid,     -p   Minimum percentage identity to reference.\n"
        " --file,        -f   A valid path to a session file.\n"
        " --log-file     -l   A log file to store the program's output.\n"
        " --version,     -v   Show versioning information.\n"
        " --help,        -h   Show this message.\n");
}

void
parse_reads (GList *reads_list)
{
  if (reads_list == NULL)
    return;

  while (reads_list->next != NULL)
    {
      struct nsv_read_t *read_obj = reads_list->data;
      if (read_obj == NULL)
        {
          reads_list = reads_list->next;
          continue;
        }

      GList *segments = read_obj->segments;
      uint32_t segments_len = g_list_length (segments);
      if (segments_len > 1 && segments_len < nsv_config.max_split)
        {
          while (segments->next != NULL)
            {
              struct nsv_segment_t *first = segments->data;
              struct nsv_segment_t *second = segments->next->data;

              //uint32_t gap = ...;
              //uint32_t breakpoint_1 = ...;
              //uint32_t breakpoint_2 = ...;
            }
        }

      reads_list = reads_list->next;
    }
}

void
parse_sam_output (char *filename)
{
  printf ("Parsing '%s'.\n", filename);
  GList *reads;
  reads = nsv_reads_from_bam (filename);
  printf ("Parsed %d reads.\n", g_list_length (reads));

  /* TODO: Free the reads.. */
  g_list_free_full (reads, nsv_read_destroy);
}
  
int
main (int argc, char **argv)
{
  #ifdef ENABLE_MTRACE
  mtrace ();
  #endif

  if (argc < 2)
    {
      show_help ();
      return 1;
    }

  int arg = 0;
  int index = 0;

  /*----------------------------------------------------------------------.
   | OPTIONS                                                              |
   | An array of structs that list all possible arguments that can be     |
   | provided by the user.                                                |
   '----------------------------------------------------------------------*/
  static struct option options[] =
  {
    { "max-threads",       required_argument, 0, 't' },
    { "split",             required_argument, 0, 's' },
    { "distance",          required_argument, 0, 'd' },
    { "min-pid",           required_argument, 0, 'p' },
    { "mate-distance",     required_argument, 0, 'r' },
    { "max-window-size",   required_argument, 0, 'w' },
    { "cluster",           required_argument, 0, 'n' },
    { "min-mapq",          required_argument, 0, 'm' },
    { "file",              required_argument, 0, 'f' },
    { "log-file",          required_argument, 0, 'l' },
    { "help",              no_argument,       0, 'h' },
    { "version",           no_argument,       0, 'v' },
    { "test",              required_argument, 0, 'z' },
    { 0,                   0,                 0, 0 }
  };
  
  while (arg != -1)
    {
      /* Make sure to list all short options in the string below. */
      arg = getopt_long (argc, argv, "t:s:d:p:r:w:n:m:f:l:z:vh", options, &index);
      switch (arg)
        {
        case 't': nsv_config.max_threads = atoi (optarg); break;
        case 's': nsv_config.max_split = atoi (optarg); break;
        case 'd': break;
        case 'p': nsv_config.min_identity = atof (optarg); break;
        case 'r': break;
        case 'w': nsv_config.max_window_size = atoi (optarg); break;
        case 'n': break;
        case 'm': nsv_config.min_map_quality = atof (optarg); break;
        case 'f': break;
        case 'l': nsv_config.logger = infra_logger_new (optarg); break;
        case 'z': parse_sam_output (optarg); break;
        case 'v': show_version (); break;
        case 'h': show_help (); break;
        }
    }

  #ifdef ENABLE_MTRACE
  muntrace ();
  #endif

  if (nsv_config.logger)
    infra_logger_destroy (nsv_config.logger);

  return 0;
}