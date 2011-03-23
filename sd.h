#ifndef _SD_H_
#define _SD_H_

	void sd_init(void);
	void sd_shutdown(void);
	int sd_play_file(const char * file);
	
	void sd_start_record(const char * file);
	void sd_stop_record(void);
	
	int sd_test_file_present(void);
#endif
