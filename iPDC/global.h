/* ----------------------------------------------------------------------------- 
 * global.h
 *
 * iPDC - Phasor Data Concentrator
 *
 * Copyright (C) 2011-2012 Nitesh Pandit
 * Copyright (C) 2011-2012 Kedar V. Khandeparkar
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


#include <pthread.h>
#include  <netinet/in.h>
#include  <stdio.h>

#define MAX_STRING_SIZE 65536


/* ---------------------------------------------------------------- */
/*                         global variables                         */
/* ---------------------------------------------------------------- */

pthread_mutex_t mutex_cfg;  /* To lock cfg data objects */
pthread_mutex_t mutex_file;  /* To lock PMU Setup File */
pthread_mutex_t mutex_Lower_Layer_Details;  /* To lock objects of connection table that hold lower layer PMU/PDC ip and protocol */
pthread_mutex_t mutex_Upper_Layer_Details;  /* To lock objects of connection table that hold upper layer PDC ip and protocol */
pthread_mutex_t mutex_status_change;
pthread_mutex_t mutex_on_TSB;

unsigned char *cfgframe,*dataframe;

struct sockaddr_in UDP_my_addr,TCP_my_addr; /* my address information */
struct sockaddr_in UL_UDP_addr,UL_TCP_addr; /* connector’s address information */
int UL_UDP_sockfd,UL_TCP_sockfd; /* socket descriptors */
pthread_t UDP_thread,TCP_thread,p_thread, DB_thread;

FILE *fp_log;

// FILE *fp_updc;
// FILE *nldc_file;
char tname[20];

/* iPDC Setup File path globaly */
char ipdcFolderPath[200];
char ipdcFilePath[200];


/* --------------------------------------------------------------------	*/
/*				global DataBase variables		*/
/* --------------------------------------------------------------------	*/

struct sockaddr_in DB_Server_addr; // address information
int DB_sockfd,DB_addr_len;

int PDC_IDCODE,TCPPORT,UDPPORT;
char dbserver_ip[20];

int old_fsize;

unsigned char DATASYNC[3],CFGSYNC[3],CMDSYNC[3];
unsigned char CMDDATASEND[3],CMDDATAOFF[3],CMDCFGSEND[3];
unsigned char CMDINSTSEND[3];

// Kedar 22-09-2013
char *logTimeBuff;
char *smallBuff;
char *logTimeBuffBakup;
pthread_mutex_t mutex_log_write;
unsigned int logIndex,pmuCount,tempPmuCount;

// Gopal 2013-12-15
uint16_t previous_crc;
/**************************************** End of File *******************************************************/
