#include <stdio.h>

int main(int argc, char** argv)
{
  FILE* filpin = NULL;
  FILE* filpout = NULL;
  unsigned char buffer[400];
  unsigned char temp;
  int i, j, ReadSize;
  if (argc == 3)
  {
    filpin = fopen(argv[1], "rb");
    if (!filpin)
      return 1;

    filpout = fopen(argv[2], "wb");
    if (!filpout)
    {
      fclose(filpin);
      return 1;
    }

    while (!feof(filpin))
    {
      ReadSize = fread(buffer, 1, 400, filpin);

      for (j=0;j<ReadSize;j+=2)
      {
        temp = buffer[j];
        buffer[j] = buffer[j+1];
        buffer[j+1] = temp;
      }

      fwrite(buffer, 1, ReadSize, filpout);
    }

    fclose(filpin);
    fclose(filpout);
  }
  else
  {
    printf("Usage: swapend <File to process> <Output file name>\n");
  }

  return 0;
}
