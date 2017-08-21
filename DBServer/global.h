/* ----------------------------------------------------------------------------- 
 * global.h
 * 
 * iPDC - Phasor Data Concentrator
 *
 * Copyright (C) 2011 Nitesh Pandit
 * Copyright (C) 2011 Kedar V. Khandeparkar
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Authors: 
 *		Nitesh Pandit <panditnitesh@gmail.com>
 *		Kedar V. Khandeparkar <kedar.khandeparkar@gmail.com>			
 *
 * ----------------------------------------------------------------------------- */


/* #include <mysql.h> */
#include <pthread.h>


/* ---------------------------------------------------------------- */
/*                 	Database variables						   */
/* ---------------------------------------------------------------- */

char *server;
char *user;
char *password ;
char *database ;
int BUFF_LEN;

// MYSQL *conn_data; // Connection to database for data insertion
// MYSQL *conn_cfg;  // Connection to database for cfg insertion

pthread_mutex_t mutex_cfg;  // To lock cfg objects
pthread_mutex_t mutex_file;  // To lock cfg.txt
pthread_mutex_t mutex_MYSQL_CONN_ON_DATA;  // To lock 'conn_data' connection for DATA insertion
pthread_mutex_t mutex_MYSQL_CONN_ON_CFG;   // To lock 'conn_cfg' connection for CFG insertion

pthread_mutex_t mutex_phasor_buffer;
/*
char *phasorBuff;
char *frequencyBuff;
char *digitalBuff;
char *analogBuff;
char *delayBuff;
char *statBuff;
*/
struct BUFFER {
	char buffer[10000000];
	size_t curPos;
}phasorBuff,frequencyBuff,digitalBuff,analogBuff,delayBuff,statBuff;


// Coded by Kedar K on 05-07-13 v1.4

char *phasorBuffBakupCopy;
char *frequencyBuffBakupCopy;
char *digitalBuffBakupCopy;
char *analogBuffBakupCopy;
char *delayBuffBakupCopy;
char *statBuffBakupCopy;

// Added by Gopal on 2014-02-22
//
FILE **binData; //, *binData1;
unsigned long int *srNo;
unsigned int *endSampPosition;
char **cfgFileName;
unsigned long tStartArchive;

// Added by Gopal on 2014-03-09
//
unsigned int cfgInProcess; // A flag to capture that DB is engaged in prosessing the CFG data.
/**************************************** End of File *******************************************************/
