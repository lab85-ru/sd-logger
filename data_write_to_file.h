#ifndef _DATA_WRITE_TO_FILE_H_
#define _DATA_WRITE_TO_FILE_H_

void data_processing( void );
void generate_file_name(char *path, int id, char *fname, const uint16_t fname_size_max);
void close_file( void );
FRESULT write_data_to_file( uint8_t file_n, uint16_t d_size, uint32_t *w_size );

#endif