#include "g_local.h"

// ==============================================================
// PAK FILES
// ==============================================================

static void InitPakHeader(pak_header_t *header)
{
	memset(header, 0, sizeof(*header));

	header->ident[0] = 'P';
	header->ident[1] = 'A';
	header->ident[2] = 'C';
	header->ident[3] = 'K';
}

static void SetPakStruct(PFILE *pakfile, FILE *fd, size_t start, size_t end, qboolean text)
{
	pakfile->fd = fd;
	pakfile->start = start;
	pakfile->end = end;
	pakfile->text = text;
}

static qboolean pfindfile(FILE *fd, const char *filename, pak_tocentry_t *entry)
{
	size_t			oldpos;
	pak_header_t	header;
	int				toc_count;

	if (!fd)
		return false;

	oldpos = ftell(fd);
	InitPakHeader(&header);
	if ( fseek(fd, 0, SEEK_SET) )
		return false;

	if ( !fread(&header, sizeof(header), 1, fd) || DL_strcmp(header.ident, "PACK", 4, true))
	{
		fseek(fd, oldpos, SEEK_SET);
		return false;
	}
	header.toc_offset = LittleLong(header.toc_offset);
	header.toc_size = LittleLong(header.toc_size);

	// we probably have a valid pak file
	toc_count = header.toc_size / TOC_SIZE;
	if ( fseek(fd, header.toc_offset, SEEK_SET) )
	{
		fseek(fd, oldpos, SEEK_SET);
		return false;
	}

	while (toc_count--)
	{
		if ( !fread(entry, sizeof(*entry), 1, fd) )
		{
			memset(entry, 0, sizeof(*entry));
			fseek(fd, oldpos, SEEK_SET);
			return false;
		}
		entry->offset = LittleLong(entry->offset);
		entry->size = LittleLong(entry->size);

		if ( !DL_strcmp(entry->filename, filename, -1, false) )
		{	// found it
			fseek(fd, oldpos, SEEK_SET);
			return true;
		}
	}
	fseek(fd, oldpos, SEEK_SET);
	memset(entry, 0, sizeof(*entry));
	return false;
}

PFILE *pfopen(const char *name, const char *mode)
{
	char	*basedir, *gamedir, *cddir;
	FILE	*fd;
	PFILE	*pakfile;
	char	buf[MAX_QPATH], path[MAX_QPATH];
	int		i, pak_num;
	pak_tocentry_t entry;
	size_t	pos;
	qboolean text;

	basedir	= gi.cvar("basedir", "baseq2", 0)->string;
	gamedir	= gi.cvar("game", "baseq2", 0)->string;
	cddir	= gi.cvar("cddir", "baseq2", 0)->string;

	text = strchr(mode, 't') ? true : false;

	for (i = 0; i < 3; i++)
	{
		// order of search: gamedir, baseq2, cddir
		Com_sprintf(path, sizeof(path), "%s/", basedir);
		switch(i)
		{
		case 0:		Com_sprintf(path, sizeof(path), "%s/%s", basedir, gamedir); break;
		case 1:		Com_sprintf(path, sizeof(path), "%s/%s", basedir, "baseq2"); break;
		case 2:		DL_strcpy(path, cddir, sizeof(path)); break;
		}
		
		// look in pakfiles first
		for (pak_num = 0; pak_num < 10; pak_num++)
		{
			Com_sprintf(buf, sizeof(buf), "%s/pak%d.pak", path, pak_num);
			fd = fopen(buf, "rb");
			if (!fd)
				continue;
			// gi.dprintf("Searching %s\n", buf);

			if ( pfindfile(fd, name, &entry) )
			{
				if (text)	// need to reopen file in text mode?
				{
					fclose(fd);
					assert(fd = fopen(buf, "rt"));
				}
				pakfile = gi.TagMalloc(sizeof(*pakfile), TAG_GAME);
				SetPakStruct(pakfile, fd, entry.offset, entry.offset+entry.size, text);
				fseek(pakfile->fd, entry.offset, SEEK_SET);
				return pakfile;
			}
			fclose(fd);
		}
		// then regular files
		Com_sprintf(buf, sizeof(buf), "%s/%s", path, name);
		fd = fopen(buf, "rb");
		if (!fd)
			continue;
		if (text)	// need to reopen file in text mode?
		{
			fclose(fd);
			assert(fd = fopen(buf, "rt"));
		}
		pakfile = gi.TagMalloc(sizeof(*pakfile), TAG_GAME);
		fseek(fd, 0, SEEK_END);
		pos = ftell(fd);
		fseek(fd, 0, SEEK_SET);
		SetPakStruct(pakfile, fd, 0, pos, text);
		return pakfile;
	}
	return NULL;
}

int pfseek(PFILE *pakfile, long offset, int mode)
{
	switch (mode)
	{
	case SEEK_SET:
		if (!inbounds(offset, 0, pakfile->end - pakfile->start)) return 1;
		return fseek(pakfile->fd, pakfile->start+offset, SEEK_SET);
		break;
	case SEEK_END:
		if (!inbounds(offset, (signed)pakfile->start - (signed)pakfile->end, 0)) return 1;
		return fseek(pakfile->fd, pakfile->end+offset, SEEK_SET);
		break;
	case SEEK_CUR:
		if (!inbounds(ftell(pakfile->fd), pakfile->start, pakfile->end)) return 1;
		return fseek(pakfile->fd, offset, SEEK_CUR);
		break;
	default:
		return 1;
	}
}

int pftell(PFILE *pakfile)
{
	return ftell(pakfile->fd) - pakfile->start;
}

size_t pfread(void *buffer, size_t size, size_t count, PFILE *pakfile)
{
	size_t nbytes = size*count;

	if (!inbounds(ftell(pakfile->fd), pakfile->start, pakfile->end)) return 1;
	if (!inbounds(ftell(pakfile->fd)+nbytes, pakfile->start, pakfile->end)) return 1;
	return fread(buffer, size, count, pakfile->fd);
}

char *pfgets(char *string, int n, PFILE *pakfile)
{
	return fgets(string, n, pakfile->fd);
}

int pfclose(PFILE *pakfile)
{
	FILE *fd;
	fd = pakfile->fd;
	gi.TagFree(pakfile);
	return fclose(fd);
}
