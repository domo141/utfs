
#if ! USE_EXTERNAL_GPL_CODE
#error not allowed to use external gpl code
#endif

#if W32X_EXT_HC
#error already included
#endif
#define W32X_EXT_HC 1

#if ! WIN32
#error only for w32 builds
#endif

/* this file contains external code licensed with
   LGPL and GPL (at least) */

#if 0
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#endif

#include <ctype.h>


/*  --8<----8<----8<----8<----8<----8<----8<----8<----8<----8<----8<--  */

/* setenv() and unsetenv() from gnuwin32.sourceforge.net */

int setenv (const char *name, const char *value, int replace)
{
	char string0[MAX_PATH];
	char *string;

	if (getenv(name)!=NULL && replace==0)
		return 1;
	strcpy(string0, name);
	strcat(string0, "=");
	strcat(string0, value);
	string=strdup(string0);
	if (putenv(string))
		return 1;
	else
		return 0;
	
}

int unsetenv (const char *name)
{
	if (putenv(name))
		return 1;
	else
		return 0;
}

/*  --8<----8<----8<----8<----8<----8<----8<----8<----8<----8<----8<--  */

/* *statvfs*() code from gnuwin32.sourceforge.net */

#define MFSNAMELEN  16   /* length of fs type name, including null */
#define   MNAMELEN  90   /* length of buffer for returned name */

typedef unsigned __uid_t; /* XXX */
typedef  struct { int __val[2]; } __fsid_t; // grep FSID_T /usr/include/**/*.h

#define __set_errno(Val) errno = (Val) // stdio-common/tempname.c in gnuwin32
#define set_errno(v) __set_errno(v)
#define set_werrno set_errno(GetLastError()) // google search

struct statfsx64
  {
    unsigned int f_type;           /* type of filesystem (see fsinfo.h) */
    unsigned int f_bsize;               /* file system block size */ 
    unsigned long int f_frsize;    /* fragment size: fundamental filesystem block */
    unsigned long int f_iosize;    /* optimal transfer block size */
    __fsblkcnt64_t f_blocks;       /* total number of blocks on file system
in units of f_frsize */
    __fsblkcnt64_t f_bfree;             /* total number of free blocks */    
                          
    __fsblkcnt64_t f_bavail;       /* number of free blocks available to non-privileged
process */ 
    __fsblkcnt64_t f_files;             /* total number of file serial numbers
*/                       
    __fsblkcnt64_t f_ffree;             /* total number of free file serial numbers
*/                  
    __fsfilcnt_t f_favail;         /* number of file serial numbers available
to non-privileged process */
    __fsid_t f_fsid;                    /* file system id */         
    __uid_t     f_owner;       /* user that mounted the filesystem */
    unsigned long int f_flag; /* bit mask of f_flag values */
    char f_fstypename[MFSNAMELEN]; /* fs type name */
    char f_mntonname[MNAMELEN];   /* directory on which mounted */
    char f_mntfromname[MNAMELEN];/* mounted filesystem */
    unsigned int f_namelen;             /* maximum filename length */
  };

/* XXX */
#define PATH_MAX 256 // POSIX_PATH_MAX in posix/bits/posix1_lim.h of GnuWin32

enum { // from /usr/include/bits/startvfs.h
  ST_RDONLY = 1,                /* Mount read-only.  */
  ST_NOSUID = 2                 /* Ignore suid and sgid bits.  */
}; 

// google search
#define MSDOS_SUPER_MAGIC     0x4d44
#define NTFS_SUPER_MAGIC      0x5346544E
#define ISOFS_SUPER_MAGIC     0x9660

// XXX XXX 
#define CDFS_SUPER_MAGIC ISOFS_SUPER_MAGIC
#define FAT_SUPER_MAGIC MSDOS_SUPER_MAGIC
#define FAT32_SUPER_MAGIC MSDOS_SUPER_MAGIC

#define ISDIRSEP(c) (c == '/')

/* Return information about the filesystem on which FILE resides.  */

/* Return information about the filesystem with rootdir FILE   */
/* struct statfsx is the smallest common multiple of statfs, statvfs, statfs_bsd */
static int
__rstatfsx64 (const char *RootDirectory, struct statfsx64 *buf)

{
        DWORD VolumeSerialNumber, MaximumComponentLength,
                FileSystemFlags, SectorsPerCluster, BytesPerSector,
                BytesPerCluster, FreeClusters, Clusters, Attributes;
        ULARGE_INTEGER FreeBytesAvailableToCaller, TotalNumberOfBytes,
                TotalNumberOfFreeBytes;
        TCHAR VolumeName[PATH_MAX];
        TCHAR FileSystemNameBuffer[PATH_MAX];
        HINSTANCE hinst = LoadLibrary ("KERNEL32");
        FARPROC pfnGetDiskFreeSpaceEx = GetProcAddress (hinst, "GetDiskFreeSpaceEx");
        int retval = 0;
        
        if (RootDirectory == NULL ) { //|| access (RootDirectory, F_OK)) {
                buf = NULL;
                __set_errno(ENOENT);
                return -1;
        }
//      fprintf(stderr, "__rstatfsx64: RootDirectory: %s\n", RootDirectory);

        if (!GetVolumeInformation (RootDirectory, (LPTSTR) &VolumeName, PATH_MAX,
                &VolumeSerialNumber, &MaximumComponentLength, &FileSystemFlags,
                (LPTSTR) &FileSystemNameBuffer, PATH_MAX)) {
//                      fprintf (stderr, "%s\n", "Cannot obtain volume information");
                        set_werrno;
                        return -1;
                }
/*      fprintf(stderr, "%s: %s\n", "VolumeName", VolumeName);
        fprintf(stderr, "%s: %s\n", "FileSystemNameBuffer", FileSystemNameBuffer);
        fprintf(stderr, "%s: %u\n", "VolumeSerialNumber", VolumeSerialNumber);
        fprintf(stderr, "%s: %d\n", "MaximumComponentLength", MaximumComponentLength);
*/
        if (!GetDiskFreeSpace (RootDirectory, &SectorsPerCluster, &BytesPerSector,
                &FreeClusters, &Clusters)) {
//                      fprintf (stderr, "%s\n", "Cannot obtain free disk space");
                        SectorsPerCluster = 1;
                        BytesPerSector = 1;
                        FreeClusters = 0;
                        Clusters = 0;
                }
//      fprintf(stderr, "%s: %10u\n", "SectorsPerCluster", SectorsPerCluster);
//      fprintf(stderr, "%s: %10u\n", "BytesPerSector   ", BytesPerSector);
//      fprintf(stderr, "%s: %10u\n", "FreeClusters     ", FreeClusters);
//      fprintf(stderr, "%s: %10u\n", "Clusters         ", Clusters);

        if (pfnGetDiskFreeSpaceEx) {
                if (!pfnGetDiskFreeSpaceEx (RootDirectory, &FreeBytesAvailableToCaller, &TotalNumberOfBytes,
                        &TotalNumberOfFreeBytes)) {
//                      fprintf (stderr, "Cannot obtain free disk space ex\n");
                        }
        } else {
                BytesPerCluster = SectorsPerCluster * BytesPerSector;
//              fprintf (stderr, "NoGetDiskFreeSpaceEx\n"); 
                TotalNumberOfBytes.QuadPart = Int32x32To64(Clusters, BytesPerCluster);
                TotalNumberOfFreeBytes.QuadPart = Int32x32To64(FreeClusters, BytesPerCluster);
                FreeBytesAvailableToCaller=TotalNumberOfFreeBytes;
        }
        if (hinst) 
                FreeLibrary(hinst);
//      fprintf(stderr, "%s: %20I64u      \n", "TotalNumberOfBytes        ", TotalNumberOfBytes);
//      fprintf(stderr, "%s: %20Lu %20Lu\n", "TotalNumberOfBytes        ",
//              TotalNumberOfBytes.HighPart, TotalNumberOfBytes.LowPart);
//      fprintf(stderr, "%s: %20I64u      \n", "FreeBytesAvailableToCaller", FreeBytesAvailableToCaller);
//      fprintf(stderr, "%s: %20Lu %20Lu\n", "FreeBytesAvailableToCaller",
//              FreeBytesAvailableToCaller.HighPart, FreeBytesAvailableToCaller.LowPart);
//      fprintf(stderr, "%s: %20I64u      \n", "TotalNumberOfFreeBytes    ", TotalNumberOfFreeBytes);
//      fprintf(stderr, "%s: %20Lu %20Lu\n", "TotalNumberOfFreeBytes    ",
//              TotalNumberOfFreeBytes.HighPart, TotalNumberOfFreeBytes.LowPart);
//      fflush(stderr);

        if ((Attributes = GetFileAttributes (RootDirectory)) == INVALID_FILE_ATTRIBUTES) {
                set_werrno;
                retval = -1;
/*              fprintf (stderr, "Cannot obtain file attributes\n"); */
        }
        buf->f_flag = 0;
        if (Attributes & FILE_ATTRIBUTE_READONLY){
                buf->f_flag |= ST_RDONLY;
        }

        if (!strcmp (FileSystemNameBuffer, "FAT32")){
                buf->f_type = FAT32_SUPER_MAGIC;
                buf->f_flag |= ST_NOSUID;
        }
        else if (!strcmp (FileSystemNameBuffer, "FAT")) {
                buf->f_type = FAT_SUPER_MAGIC;
                buf->f_flag |= ST_NOSUID;
        }
        else if (!strcmp (FileSystemNameBuffer, "NTFS"))
                buf->f_type = NTFS_SUPER_MAGIC;
        else if (!strcmp (FileSystemNameBuffer, "CDFS")) {
                buf->f_type = CDFS_SUPER_MAGIC;
                buf->f_flag |= ST_NOSUID;
        }
        else {
//              fprintf(stderr, "%s: %s\n", "Unknown Filesystem", FileSystemNameBuffer);
                buf->f_flag |= ST_NOSUID;
        }
//              fprintf(stderr, "Flag: %X\n", buf->f_flag);

        buf->f_bsize = BytesPerSector;
        buf->f_frsize = BytesPerSector;
        buf->f_iosize = BytesPerSector;
        buf->f_blocks = TotalNumberOfBytes.QuadPart / BytesPerSector;
        buf->f_bfree = TotalNumberOfFreeBytes.QuadPart / BytesPerSector;
        buf->f_bavail = FreeBytesAvailableToCaller.QuadPart / BytesPerSector;
        buf->f_files = buf->f_blocks / SectorsPerCluster;
        buf->f_ffree = buf->f_bfree / SectorsPerCluster;
        buf->f_favail = buf->f_bavail / SectorsPerCluster; 
        buf->f_namelen = (unsigned int) MaximumComponentLength;
        buf->f_fsid.__val[0] = HIWORD(VolumeSerialNumber);
        buf->f_fsid.__val[1] = LOWORD(VolumeSerialNumber);
        buf->f_owner = (__uid_t) -1;
//      fprintf(stderr, "FileSystemNameBuffer: %s\n", FileSystemNameBuffer);
//      fprintf(stderr, "RootDirectory: %s\n", RootDirectory);
    strncpy (buf->f_fstypename, FileSystemNameBuffer, MFSNAMELEN);
    strncpy (buf->f_mntonname, RootDirectory, MNAMELEN);
    strncpy (buf->f_mntfromname, RootDirectory, MNAMELEN);
//      fprintf(stderr, "buf->f_fstypename: %s\n", buf->f_fstypename);
//      fprintf(stderr, "buf->f_mntonname: %s\n", buf->f_mntonname);
//      fprintf(stderr, "buf->f_mntfromname: %s\n", buf->f_mntfromname);
        return retval;
}

char *win2unixpath(char *FileName)
{
        char *s = FileName;
        while (*s) {
                if (*s == '\\')
                        *s = '/';
                s++;
        }
        return FileName;
}


/* Return the canonical absolute name of file NAME.  A canonical name
   does not contain any `.', `..' components nor any repeated path
   separators ('/') or symlinks.  All path components must exist.  If
   RESOLVED is null, the result is malloc'd; otherwise, if the
   canonical name is PATH_MAX chars or more, returns null with `errno'
   set to ENAMETOOLONG; if the name fits in fewer than PATH_MAX chars,
   returns the name in RESOLVED.  If the name cannot be resolved and
   RESOLVED is non-NULL, it contains the path of the first component
   that cannot be resolved.  If the path can be resolved, RESOLVED
   holds the same value as the value returned.
   RESOLVED must be at least PATH_MAX long */

/* note: renamed canonicalze() as canonicalize_file_name ()
   and removed 'resolved' (marked NULL and then removed)  */

static char *
canonicalize_file_name (const char *name)
{
  char *rpath, *dest, *extra_buf = NULL;
  const char *start, *end, *rpath_limit;
  long int path_max;
  int num_links = 0, old_errno;

  if (name == NULL)
    {
      /* As per Single Unix Specification V2 we must return an error if
	 either parameter is a null pointer.  We extend this to allow
	 the RESOLVED parameter to be NULL in case the we are expected to
	 allocate the room for the return value.  */
      __set_errno (EINVAL);
      return NULL;
    }

  if (name[0] == '\0')
    {
      /* As per Single Unix Specification V2 we must return an error if
	 the name argument points to an empty string.  */
      __set_errno (ENOENT);
      return NULL;
    }
#ifdef __WIN32__
	{
	char *lpFilePart;
	int len;
	DWORD attr;
//  fprintf(stderr, "name: %s\n", name);
	rpath = malloc (MAX_PATH);
//	unix2winpath (name);
//  fprintf(stderr, "name: %s\n", name);
	len = GetFullPathName(name, MAX_PATH, rpath, &lpFilePart);
//  fprintf(stderr, "rpath: %s\n", rpath);
	if (len == 0) {
		set_werrno;
		return NULL;
	}
	if (len > MAX_PATH)	{
			rpath = realloc(rpath, len + 2);
			GetFullPathName(name, len, rpath, &lpFilePart);
//  fprintf(stderr, "rpath: %s\n", rpath);
	}
//	if ( ISDIRSEP(name[strlen(name)]) && !ISDIRSEP(rpath[len]) ) {
//		rpath[len] = '\\';
//		rpath[len + 1] = 0;
//	}
	old_errno = errno;
	
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
	if ((attr = GetFileAttributes (rpath)) == INVALID_FILE_ATTRIBUTES) {
        	set_werrno; /* file does not exist or attributes cannot be read */
		SetErrorMode(0);
		return NULL;
	}
	SetErrorMode(0);
		
	if ((attr & FILE_ATTRIBUTE_DIRECTORY) && !ISDIRSEP(rpath[len - 1]) ){
		rpath[len] = '\\';
		rpath[len + 1] = 0;
	}
	errno = old_errno;
	win2unixpath (rpath);
//  fprintf(stderr, "rpath: %s\n", rpath);
	return rpath ;
	}
#endif /* __WIN32__ */
}


/* Return the root directory of a file */
char * rootdir (const char *file)
{
        char *path, *RootDirectory, *p;
        int len;

        path = canonicalize_file_name (file);
#ifdef TEST
        fprintf (stderr, "Path: %s\n", path);
#endif /* TEST */
        if (path && strlen (path) >= 3 && path[1] == ':' && path[2] == '/') {
                RootDirectory = strdup (" :/");
                RootDirectory[0] = toupper (path[0]);
        }
        else if (path && (len = strlen (path)) >= 5 && path[0] == '/' && path[1] == '/') {
                p = strchr (path+2, '/');
                if (p)
                        p = strchr (p+1, '/');
                if (p)
                        len = p - path + 1;
                else
                        len++;
                RootDirectory = calloc (len + 1, sizeof (char));
                strncpy (RootDirectory, path, len+1);
                RootDirectory[len-1] = '/';
                RootDirectory[len] = 0;
        }
        else
                RootDirectory = NULL;
        free (path);
        return (RootDirectory);
}

static int
__statfsx64 (const char *file, struct statfsx64 *buf)
{
        char *RootDirectory = rootdir (file);
        int res = 0;
        
        if ((RootDirectory == NULL) ) {// || access (file, F_OK)) { // || (GetDriveType(RootDirectory) == DRIVE_REMOVABLE)) {
//      fprintf(stderr, "__statfsx64: RootDirectory: %s\n", RootDirectory);
                buf = NULL;
                __set_errno(ENOENT);
                res = -1;
        }
        else
                res = __rstatfsx64 ((const char *) RootDirectory, buf);
        free (RootDirectory);
        return res;
}


int statvfs (const char *file, struct statvfs *buf)
{
        struct statfsx64 xbuf;
        int res;
        res = __statfsx64(file, &xbuf);
        buf->f_bsize = xbuf.f_bsize;
        buf->f_frsize = xbuf.f_frsize;
        buf->f_blocks = xbuf.f_blocks;
        buf->f_bfree = xbuf.f_bfree;
        buf->f_bavail = xbuf.f_bavail;
        buf->f_files = xbuf.f_files;
        buf->f_ffree = xbuf.f_ffree;
        buf->f_favail = xbuf.f_favail;
#if 0
        buf->f_fsid = xbuf.f_fsid; // see also http://www.linuxcertif.com/man/2/statfs64/
#else
        buf->f_fsid = xbuf.f_fsid.__val[1]; // XXX
#endif
        buf->f_flag = xbuf.f_flag;
        buf->f_namemax = xbuf.f_namelen;
#if 0
        buf->f_spare[0] = 0;
        buf->f_spare[1] = 0;
        buf->f_spare[2] = 0;
        buf->f_spare[3] = 0;
        buf->f_spare[4] = 0;
        buf->f_spare[5] = 0;
#endif
        return res;
}

/*  --8<----8<----8<----8<----8<----8<----8<----8<----8<----8<----8<--  */
#if 0
/* Create a directory named PATH with protections MODE.  */
int mkdir (const char * path, unsigned int mode)
{
  if (path == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }
  if (!CreateDirectory (path, NULL)) {
        set_werrno;
        return -1;
  }
  (void)mode;
  return 0;
  //return chmod (path, mode);
}
#endif
/*  --8<----8<----8<----8<----8<----8<----8<----8<----8<----8<----8<--  */
