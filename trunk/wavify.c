#include <stdio.h>

typedef union
{
  short int word;
  char      little;
} endian_t;

int main(int argc, char** argv)
{
  FILE* filpin = NULL;
  FILE* filpout = NULL;
  endian_t endian;
  size_t DataSize;
  size_t ReadSize;
  char  buffer[1000];

  endian.word = 0x0001;

  if (argc == 3)
  {
    filpin = fopen(argv[1], "rb");
    if (!filpin)
    {
      return 1;
    }

    fseek(filpin, 0, SEEK_END);
    DataSize = ftell(filpin);
    fseek(filpin, 0, SEEK_SET);

    filpout = fopen(argv[2], "wb");
    if (!filpout)
    {
      fclose(filpin);
      return 1;
    }

    fwrite("RIFF", 1, 4, filpout);

    if (endian.little)
    {
      DataSize += 36;
      fwrite((char*)&DataSize, 4, 1, filpout);
      DataSize -= 36;
    }
    else
    {
      fprintf(stderr, "Big Endian?\n");
    }

    fwrite("WAVE", 1, 4, filpout);

    fwrite("fmt ", 1, 4, filpout);

    fprintf(filpout, "%c%c%c%c", 0x10, 0x00, 0x00, 0x00);

    fwrite("\1\0", 1, 2, filpout);

    fwrite("\2\0", 1, 2, filpout);

    fprintf(filpout, "%c%c%c%c", 0x44, 0xAC, 0x00, 0x00);

    fprintf(filpout, "%c%c%c%c", 0x10, 0xB1, 0x02, 0x00);

    fprintf(filpout, "%c%c", 0x04, 0x00);

    fprintf(filpout, "%c%c", 0x10, 0x00);

    fwrite("data", 1, 4, filpout);

    fwrite((char*)&DataSize, 4, 1, filpout);

    while (!feof(filpin))
    {
      ReadSize = fread(buffer, 1, 1000, filpin);
      fwrite(buffer, 1, ReadSize, filpout);
    }

    fclose(filpout);
    fclose(filpin);
  }
}
