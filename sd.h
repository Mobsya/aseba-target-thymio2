/*
        Thymio-II Firmware

        Copyright (C) 2011 Philippe Retornaz <philippe dot retornaz at epfl dot ch>,
        Mobots group (http://mobots.epfl.ch), Robotics system laboratory (http://lsro.epfl.ch)
        EPFL Ecole polytechnique federale de Lausanne (http://www.epfl.ch)

        See authors.txt for more details about other contributors.

        This program is free software: you can redistribute it and/or modify
        it under the terms of the GNU Lesser General Public License as published
        by the Free Software Foundation, version 3 of the License.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License
        along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SD_H_
#define _SD_H_

	void sd_init(void);
	void sd_shutdown(void);
	int sd_play_file(const char * file, int loop);
	
	void sd_start_record(const char * file);
	void sd_stop_record(void);
	unsigned long sd_read_duration(const char *file);
	
	int sd_test_file_present(void);
	int sd_load_aseba_code(void);
	
	void sd_log_file(void);

        int sd_user_open(char * name);
        int sd_user_seek(unsigned long offset);
        unsigned int sd_user_read(unsigned char * data, unsigned int size);
        unsigned int sd_user_write(unsigned char * data, unsigned int size);
#endif
