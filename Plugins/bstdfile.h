#ifndef BSTDFILE_H__
#define BSTDFILE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define BFILE_BUFSIZE	(8192U)

typedef struct bstdfile
{
	char	buffer[BFILE_BUFSIZE];
	char* live;
	size_t	live_size;

	FILE	*fp;

	int   eof;
	int   error;
} bstdfile_t;

extern bstdfile_t	*NewBstdFile(FILE *fp);
extern int			BstdFileDestroy(bstdfile_t *BstdFile);
extern int			BstdFileEofP(const bstdfile_t *BstdFile);
extern int			BstdFileErrorP(const bstdfile_t *BstdFile);
extern size_t		BstdRead(void *UserBuffer, size_t ElementSize, size_t ElementsCount, bstdfile_t *BstdFile);

#ifdef __cplusplus
}
#endif

#endif /* BSTDFILE_H__ */
