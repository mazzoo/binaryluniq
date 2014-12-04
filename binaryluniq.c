#include <sys/types.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>


#define BUF_SZ 4096
char buf[BUF_SZ];

void usage(void)
{
  printf("usage:\n");
  printf("\tbinaryluniq image [n_min]\n");
  printf("\n");
  printf("\tsearch the longes sequence of unique bytes in image\n");
  printf("\tby default only uniq sequences of 256 bytes will be dumped\n");
  printf("\tn_min is optional: reduce the threshold of dumping from 256 to n_min\n");
  printf("\n");
}

void usage_exit(char * err)
{
  printf("\nERROR: ");
  printf(err);
  printf("\n\n");
  usage();
  exit(1);
}

void dump_bytes(uint8_t * image, int offset, int uniqcount)
{
  int i;
  printf("### %d unique bytes at offset 0x%.8x :\n", uniqcount, offset);
  for (i = 0; i < uniqcount; i++)
  {
    if ( !(i % 16) )
    {
      printf("0x%.8x :  ", offset + i);
    }
    printf("%.2x ", image[i]);
    if ( (i % 16) == 7)
    {
      printf(" ");
    }
    if ( (i % 16) == 15)
    {
      printf("\n");
    }
  }
  if ( uniqcount % 16 )
  {
    printf("\n");
  }
}

int main(int argc, char ** argv)
{
  int ret = 0;

  if ( (argc < 2) || (argc > 3) )
  {
    usage_exit("one or two arguments required");
  }

  int fimage;
  int fsize;
  uint8_t * image;

  fimage = open(argv[1], O_RDONLY);
  if (fimage < 0)
  {
    snprintf(buf, BUF_SZ, "cannot open file %s", argv[1]);
    usage_exit(buf);
  }
  fsize = lseek(fimage, 0, SEEK_END);
  if (fimage < 0)
  {
    snprintf(buf, BUF_SZ, "cannot seek file %s", argv[1]);
    usage_exit(buf);
  }
  ret = lseek(fimage, 0, SEEK_SET);
  if (ret < 0)
  {
    snprintf(buf, BUF_SZ, "cannot rewind file %s", argv[1]);
    usage_exit(buf);
  }

  image = malloc(fsize);
  if (!image)
  {
    usage_exit("not enough memory");
  }

  ret = read(fimage, image, fsize);
  if (ret != fsize)
  {
    snprintf(buf, BUF_SZ, "read: expected %d but got %d", fsize, ret);
    usage_exit(buf);
  }
  close(fimage);


  char * end;
  int n_min = 256;

  if (argc > 2)
  {
    end = argv[2];
    n_min = strtoul(argv[2], &end, 0);
    if ((errno == ERANGE && (n_min == LONG_MAX || n_min == LONG_MIN))
        || (errno != 0 && n_min == 0))
    {
      usage_exit("cannot parse your n_min");
    }
    if (end == argv[2])
    {
      usage_exit("cannot parse your n_min");
    }
    if ( (n_min > 256) || (n_min < 1) )
    {
      usage_exit("n_min not in range 1..256");
    }
  }

  printf("### searching for a minimum of %d unique bytes in %s\n", n_min, argv[1]);

  char uniq[256];
  int uniqcount;

  /* scan */
  int i;
  for (i = 0; i < fsize-256; i++)
  {

    memset(uniq, 0, 256 * sizeof(char));
    uniqcount = 0;

    int j;
    for (j = 0; j <= 256; j++)
    {
      if (!uniq[image[i+j]])
      {
        uniq[image[i+j]] = 1;
        uniqcount++;
      }else{
        goto next_round;
      }
    }
next_round:
    if (uniqcount >= n_min)
    {
      dump_bytes(&image[i], i, uniqcount);
      i += uniqcount-1;
    }
  }

  return 0;
}
