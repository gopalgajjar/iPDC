/* ----------------------------------------------------------------------------- 
 * function.c
 * 
 * PMU Simulator - Phasor Measurement Unit Simulator
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


key_t MyKey;
/* ------------------------------------------------------------------ */
/*                  global Data Structure for Shared Memory           */
/* ------------------------------------------------------------------ */

struct P_id {

	pid_t pid;               	/* Single pid */
    pid_t pidMain;

	int dataFileVar;		/* For data source options : 0-Auto & 1-File Measurements */
	char filePath[400];		/* File path of user selected measurement file */
	int UdpPort;			/* User given UDP port number for server */
	int TcpPort;			/* User given TCP port number for server */
	int cfg_bit_change_info;	/* STAT Word change bit status if any otherwise default set to 0 */
	char cfgPath[200];       	/* Its a PMU Setup File path given by use */
	int phasor_type[50];		/* save the phasor type (currently not useful) */
    int MulPort;                /* User given port for Multicasting */
    char Mulip[20];             /* Useg given Group IP for Multicasting */
    int statusMsgSet;
    char statusMsg[200];

}p1, *ShmPTR;

typedef struct _pmuStruct pmuStruct;

struct _pmuStruct
{
     /* Main application window */
	GtkWidget *Pmu_Simulator;  

     /* Main window button widgets */
	GtkWidget *create_cfg_button;
	GtkWidget *header_frm_button;
	GtkWidget *pmu_details_button;
	GtkWidget *stat_modification_button;
	GtkWidget *cfg_modification_button;
	GtkWidget *manage_data_source;

     /* Manubar button widgets */
	GtkWidget *pmu_menubar;
	GtkWidget *start_server;
	GtkWidget *open_cfg;
	GtkWidget *pmu_properties;
	GtkWidget *exit_menuitem;
	GtkWidget *about_menuitem;

	GtkWidget * menuitem2;
	GtkWidget * menu_setup_cfg;
	GtkWidget * menu_data_source;
	GtkWidget * menu_cfg_modify;
	GtkWidget * menu_stat_modify;
	GtkWidget * menu_header_frm;

     /* Main window extra widgets */
	GtkWidget *rights_lable;
	GtkWidget *admin_label;
	GtkWidget *E_button;
	GtkWidget *text_view;
	GtkWidget *text_view1;
	GtkWidget *time_lable;
    GtkWidget *statusbar;
};
pmuStruct *pmu_data;
/**************************************** End of File *******************************************************/
