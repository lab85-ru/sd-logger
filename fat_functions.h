#ifndef _FAT_FUNCTIONS_H_
#define _FAT_FUNCTIONS_H_

FRESULT scan_files ( char* path );
void rm_files( const char * path, const char * fname );
void cd_dir( char * path, const char * fname );


#endif