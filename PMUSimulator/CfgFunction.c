/* ----------------------------------------------------------------------------- 
 * CfgFunction.c
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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include "PmuGui.h"
#include "CfgFunction.h"
#include "function.h"
#include "ShearedMemoryStructure.h"
#include <stdint.h> ////for uint16_t
#include <stdbool.h>
 
#define max_data_rate 200
#define MAX_STRING_SIZE 5000


/*----------------------------------------------------------------------------------------*/
/*                            Functions in CfgFunction.c        			               */
/*----------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------
/*                                                                    
/*   1. void header_frm_gen(int len);                                 
/*   2. void reconfig_cfg_CC();                                       
/*   3. int  create_cfg();                                            
/*   4. void show_pmu_details (GtkWidget *widget, gpointer udata);    
/*   5. bool is_empty();*
/*   6. void delete_one();
/*   7. void delete_all();
/*   8. void insert_element(char dat[]);
/*   9. void print_buffer(); 
/*   10. void read_config();
/*   11. void make_config();
/*   12. void write_data();
/*   13. void write_data_helper(char* data,long int i,FILE* dat_file);
/*                                                                    */
/*--------------------------------------------------------------------*/

/* ------------------------------------------------------------------ */
/*                            global variables                        */
/* ------------------------------------------------------------------ */

double t1;
int frmt, fdf, af, pf, pn, phnmr, annmr, dgnmr, num_pmu = 1; 
int cfg2_frm_size, data_frm_size = 18, data_rate, cfgcnt = 0;  		
int i, j, n, pmu_id, indx = 0, global_data_frm_size=0, hdrFrmSize=0;
long int soc, fracsec = 0;
long int phunit = 915527, anunit = 1, digunit = 0;

const long int TB = 16777215;  // roundf(powf(2,24) - 1);

char temp_5[16];
unsigned char temp[2], temp_1[4], temp_6[16];
unsigned char cfg2_frm[MAX_STRING_SIZE];
unsigned char header[MAX_STRING_SIZE];

struct timeval tim;
uint16_t chk;

int bufferlength=50000;       //Can be changed during runtime.Buffer will be reset if length changed.
int buffercount=0;
FILE *fp;
int size_buffer=1000;
int lines;
int option;
float end_sample;
char start_time[50],end_time[50]; //Put proper values later 50 ?

int f;

struct node 
{
   char *data;
   struct node *next;
};

struct node *head = NULL;
struct node *tail = NULL;
struct node *current = NULL;

/* ------------------------------------------------------------------------*/
/* FUNCTION  header_frm_gen(int len)                                       */
/* It generates the Header Frame from user given information about PMU and	*/
/* store in a PMU Setup File.                                              */
/* ------------------------------------------------------------------------*/

void header_frm_gen(int len)
{
	/* local variables */
	int i, indx = 0;
	FILE *fp1;

	memset(header, '\0', sizeof(header));

	/* Insert the fields in Header Frame: sync word */
	header[indx++] = 0xAA; 
	header[indx++] = 0x001; 

	/* Insert frame size in Header Frame */
	i2c(0, temp);
	B_copy(header, temp, indx, 2);
	indx = indx + 2;

	/* Insert user given PMU ID in Header Frame  */
	i2c(pmu_id, temp);
	B_copy(header, temp, indx, 2);
	indx = indx + 2;

	/* Insert current SOC value in Header Frame */				
	//time_t tm = time(NULL);
	gettimeofday(&tim, NULL);
	soc = (long) tim.tv_sec;
	li2c(soc, temp_1);
	B_copy(header, temp_1, indx, 4);
	indx = indx + 4;

	/* Insert the time quality fields & fraction of second value in Header Frame */
	//gettimeofday(&tim, NULL);
	//t1=tim.tv_sec+(tim.tv_usec/1000000.0);
	//fracsec = (t1-soc)*TB;
    fracsec = (long) (tim.tv_usec*TB/1e6);
	li2c(fracsec, temp_1);
	B_copy(header, temp_1, indx, 4);
	indx = indx + 4;

	/* Insert the PMU Information given by user in Header Frame */					
	for (i = 0; i < len; i++)
	{
		header[indx+i] = hdr_frame[i];
	}
	indx = indx + i;

	/* Calculation & insert the checksum VALUE of Header frame (up to now) */
	chk = compute_CRC(header,indx);
	header[indx++] = (chk >> 8) & ~(~0<<8);  	/* CHKSUM high byte */
	header[indx++] = (chk ) & ~(~0<<8);     	/* CHKSUM low byte */

	/* Insert frame size in Header Frame */
	header[2] = indx>>8;;
	header[3] = indx;

	/* Open the PMU Setup File */
	fp1 = fopen(pmuFilePath,"ab");

	if (fp1 != NULL)		
	{
		hdrFrmSize = indx;

		char buff[15];
		sprintf(buff, "%d", indx);

		/* Write the size of Header Frame & then Header Frame into file */
		fputs("HdrFrame 1 ", fp1);
		fputs(buff, fp1);

		fwrite(header, sizeof(unsigned char), indx, fp1);
		fputc('\n', fp1);

		fclose(fp1);

	     printf("\n-> Size of Header frame is %d Bytes.", indx);
	     validation_result (" Headre Frame successfully generated.");
	}
}; /* end of header_frm_gen() */   


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  reconfig_cfg_CC()                                                  */
/* This function allows user to easy modify the existing Configuration Frame.	*/
/* In terms of modification user can add new phasor channels or remove.		*/
/* It also provide the option for change Data Rate. Finally replace the new     */
/* Configuration frame with old CFG in PMU Setup File and gives signal to PMU   */
/* serverchange to change the appropriate bit of STAT Word while sending        */
/* Data Frames to connected PDC.                                                */
/* ----------------------------------------------------------------------------	*/

void reconfig_cfg_CC()
{
	/* local Variables*/
	unsigned char stn[16];
	int old_data_rate, indx=2; 			
	int j, global_frmt, tmp_k;        
	FILE *fp1;

	/* Get the Station name from CFG FRM */
	memset(stn, '\0', 16);
	strncpy((char *)stn, cfg_info->cfg_STNname, 16);

	/* Get the FORMAT word from CFG FRM */
	global_frmt = frmt;

	/* Ask user's choice what type of modification he wants? */
	if(new_cfg_info->add_remove_choice == 1)	/* Add more Phasor/Analog channels in existing CFG as per user's wish */
	{
		phnmr = cfg_info->cfg_phnmr_val + new_cfg_info->new_cfg_phnmr_val;
		annmr = cfg_info->cfg_annmr_val + new_cfg_info->new_cfg_annmr_val;
	}
	else if(new_cfg_info->add_remove_choice == 2)	/* Remove Phasor/Analog channels from existing CFG as per user's wish */
	{
		phnmr = cfg_info->cfg_phnmr_val - new_cfg_info->new_cfg_phnmr_val;
		annmr = cfg_info->cfg_annmr_val - new_cfg_info->new_cfg_annmr_val;
	}

	/* Calculate the size of new CFG Frame */
	cfg2_frm_size = 0;
	cfg2_frm_size = 54 + (16*phnmr) + (16*annmr) + (256*dgnmr) + (4*phnmr) + (4*annmr) + (4*dgnmr);

	/* Calculate the corresponding data frame size */
	global_data_frm_size = data_frm_size;
	data_frm_size = 0;
	data_frm_size = 18;

	/* Calculate 4/8 bytes for each PHNMR */
	if (cfg_info->cfg_pf == 0) 
	{
		data_frm_size = data_frm_size + (4*phnmr);
	}
	else 
	{
		data_frm_size = data_frm_size + (8*phnmr);
	}

	/* Calculate 2/4 bytes for each ANNMR */
	if (cfg_info->cfg_af == 0) 
	{
		data_frm_size = data_frm_size + (2*annmr);
	}
	else 
	{
		data_frm_size = data_frm_size + (4*annmr);
	}

	/* Calculate 2/4 bytes for both (FREQ + DFREQ) */
	if (cfg_info->cfg_fdf == 0) 
	{				
		data_frm_size = data_frm_size + 4;
	}
	else 
	{
		data_frm_size = data_frm_size + 8;
	}

	/* Calculate 2 bytes for each DGNMR */
	data_frm_size = data_frm_size + (2*dgnmr);
	printf("\nCFG = %d, Data Frame = %d\n", cfg2_frm_size, data_frm_size);

	/* Insert the fields in new CFG Frame: sync word */
	memset(cfg2_frm,'\0',sizeof(cfg2_frm));
	indx = 0;
	cfg2_frm[indx++] = 0xAA; 
	cfg2_frm[indx++] = 0x31; 

	/* Insert frame size in new CFG Frame */
	i2c(cfg2_frm_size, temp);
	B_copy(cfg2_frm, temp, indx, 2);
	indx = indx + 2;

	/* Insert user given PMU ID in new CFG Frame  */
	i2c(pmu_id, temp);
	B_copy(cfg2_frm, temp, indx, 2);
	indx = indx + 2;

	/* Insert current SOC value in new CFG Frame */				
	soc = 0;
	li2c(soc, temp_1);
	B_copy(cfg2_frm, temp_1, indx, 4);
	indx = indx + 4;

	/* Insert the time quality fields & fraction of second value in new CFG Frame, here it is static "00000000" */
	li2c(fracsec, temp_1);
	B_copy(cfg2_frm, temp_1, indx, 4);
	indx = indx + 4;

	/* Insert Time Base for calculating fraction of second in new CFG Frame, here it is static 1000000μs */
	li2c(TB, temp_1);
	B_copy(cfg2_frm, temp_1, indx, 4);
	indx = indx + 4;

	/* Insert number of PMU in new CFG Frame: static "0001" */
	i2c(num_pmu, temp);
	B_copy(cfg2_frm, temp, indx, 2);
	indx = indx + 2;

	/* Insert the station name or PMU name in new CFG Frame */
	B_copy(cfg2_frm, stn, indx, 16);
	indx = indx + 16;

	/* Insert again pmu id in new CFG Frame */					
	i2c(pmu_id, temp);
	B_copy(cfg2_frm, temp, indx, 2);
	indx = indx + 2;

	/* Insert format word in new CFG Frame */
	i2c(global_frmt, temp);
	B_copy(cfg2_frm, temp, indx, 2);
	indx = indx + 2;

	/* Insert number of phasor or PHNMR in new CFG Frame */
	i2c(phnmr, temp);
	B_copy(cfg2_frm, temp, indx, 2);
	indx = indx + 2;

	/* Insert number of analog or ANNMR in new CFG Frame */
	i2c(annmr, temp);
	B_copy(cfg2_frm, temp, indx, 2);
	indx = indx + 2;

	/* Insert number of Digital status word or DGNMR in new CFG Frame */
	i2c(dgnmr, temp);
	B_copy(cfg2_frm, temp, indx, 2);
	indx = indx + 2;

	/* Insert NEW Phasor channel Names (if any) in new CFG frame */ 
	if (new_cfg_info->add_remove_choice == 1)
	{
		/* First Insert old Phasor channels in new CFG frame */
		B_copy(cfg2_frm, (unsigned char *)cfg_info->cfg_phasor_channels, indx, cfg_info->cfg_phnmr_val*16);
		indx = indx + (cfg_info->cfg_phnmr_val*16);

		if (new_cfg_info->new_cfg_phnmr_val > 0)
		{
			/* Insert New Phasor channels (if any) in new CFG frame */
			B_copy(cfg2_frm, (unsigned char *)new_cfg_info->new_cfg_phasor_channels, indx, (new_cfg_info->new_cfg_phnmr_val*16));
			indx = indx + (new_cfg_info->new_cfg_phnmr_val*16);
			free(new_cfg_info->new_cfg_phasor_channels);
		}
		cfg_info->cfg_phnmr_val = cfg_info->cfg_phnmr_val + new_cfg_info->new_cfg_phnmr_val;

		/* First Insert old Analog channels in new CFG frame */
		B_copy(cfg2_frm, (unsigned char *)cfg_info->cfg_analog_channels, indx, cfg_info->cfg_annmr_val*16);
		indx = indx + (cfg_info->cfg_annmr_val*16);
		if (new_cfg_info->new_cfg_annmr_val > 0)
		{
			/* Insert New Analog channels (if any) in new CFG frame */
			B_copy(cfg2_frm, (unsigned char *)new_cfg_info->new_cfg_analog_channels, indx, (new_cfg_info->new_cfg_annmr_val*16));
			indx = indx + (new_cfg_info->new_cfg_annmr_val*16);
			free(new_cfg_info->new_cfg_analog_channels);
		}
		cfg_info->cfg_annmr_val = cfg_info->cfg_annmr_val + new_cfg_info->new_cfg_annmr_val;
	}

	/* Remove channel Names (from availables) from new CFG frame */ 
	else if (new_cfg_info->add_remove_choice == 2)
	{
		/* Remove Phasor channels (if any) in new CFG frame */ 
		cfg_info->cfg_phnmr_val = cfg_info->cfg_phnmr_val - new_cfg_info->new_cfg_phnmr_val;
		B_copy(cfg2_frm, (unsigned char *)cfg_info->cfg_phasor_channels, indx, cfg_info->cfg_phnmr_val*16);
		indx = indx + (cfg_info->cfg_phnmr_val*16);      		

		/* Remove Analog channels (if any) in new CFG frame */ 
		cfg_info->cfg_annmr_val = cfg_info->cfg_annmr_val - new_cfg_info->new_cfg_annmr_val;
		B_copy(cfg2_frm, (unsigned char *)cfg_info->cfg_analog_channels, indx, cfg_info->cfg_annmr_val*16);
		indx = indx + (cfg_info->cfg_annmr_val*16);
	}

	/* Insert Digital channel Names in new CFG frame */
	B_copy(cfg2_frm, (unsigned char *)cfg_info->cfg_digital_channels, indx, (cfg_info->cfg_dgnmr_val*256));
	indx = indx + (cfg_info->cfg_dgnmr_val*256);

	/* Insert FACTOR VALUES for Phasor, Analog and Digital in new CFG frame */
	for(j=0, tmp_k=0; j<cfg_info->cfg_phnmr_val; j++)			/* FACTOR VALUES for Phasor */
	{
		cfg2_frm[indx++] = phunit_final_val[tmp_k++];
		cfg2_frm[indx++] = phunit_final_val[tmp_k++];
		cfg2_frm[indx++] = phunit_final_val[tmp_k++];
		cfg2_frm[indx++] = phunit_final_val[tmp_k++];
	}
	for(j=0, tmp_k=0; j<cfg_info->cfg_annmr_val; j++)			/* FACTOR VALUES for Phasor */
	{
		cfg2_frm[indx++] = anunit_final_val[tmp_k++];
		cfg2_frm[indx++] = anunit_final_val[tmp_k++];
		cfg2_frm[indx++] = anunit_final_val[tmp_k++];
		cfg2_frm[indx++] = anunit_final_val[tmp_k++];
	}

	for(j=0; j<cfg_info->cfg_dgnmr_val; j++)		/* FACTOR VALUES for Digital */
	{
		li2c(digunit, temp_1);
		B_copy(cfg2_frm, temp_1, indx, 4);
		indx = indx + 4;
	}

	/* Insert normal frequency or fnom VALUE in new CFG frame */
	i2c(cfg_info->cfg_fnom, temp);
	B_copy(cfg2_frm, temp, indx, 2);
	indx = indx + 2;

	/* Data rate change? option for user */
	old_data_rate = data_rate;
	if (new_cfg_info->data_rate_choice == 1)
	{
		data_rate = new_cfg_info->new_data_rate;
		cfg_info->cfg_dataRate = new_cfg_info->new_data_rate;			
		printf("\n-> New Data Rate = %d\n", data_rate);
	}

	/* Insert configuration count or cfgcnt VALUE in new CFG frame is any of addition or removal of channel happened??? */
	if ((global_data_frm_size != data_frm_size) || (old_data_rate != data_rate)) 
	{
		cfgcnt = cfgcnt + 1;
		printf("\n-> CFG COUNT incremented = %d\n", cfgcnt);
		i2c(cfgcnt, temp);
		B_copy(cfg2_frm, temp, indx, 2);
		indx = indx + 2;
	}
	else
	{ 
		i2c(cfgcnt, temp);
		B_copy(cfg2_frm, temp, indx, 2);
		indx = indx + 2;
	}

	/* Insert Data Rate VALUE in new CFG frame */
	i2c(data_rate, temp);
	B_copy(cfg2_frm, temp, indx, 2);
	indx = indx + 2;

	/* Insert current SOC value in new CFG Frame */				
    gettimeofday(&tim, NULL);
    soc = (long) tim.tv_sec;
    li2c(soc, temp_1);
    B_copy(cfg2_frm, temp_1, 6, 4);

    /* Insert the time quality fields & fraction of second value in new CFG Frame */
    //gettimeofday(&tim, NULL);
    //t1=tim.tv_sec+(tim.tv_usec/1000000.0);
    //fracsec = (t1-soc)*TB;
    fracsec = (long) (tim.tv_usec*TB/1e6);
    li2c(fracsec, temp_1);
    B_copy(cfg2_frm, temp_1, 10, 4);

	/* Calculation & insert the checksum VALUE of new CFG frame (up to now) */
	chk = compute_CRC(cfg2_frm,indx);
	cfg2_frm[indx++] = (chk >> 8) & ~(~0<<8);  	/* CHKSUM high byte */
	cfg2_frm[indx++] = (chk ) & ~(~0<<8);     	/* CHKSUM low byte */

	/* Open the PMU Setup File and replace new CFG frame with old CFG if and only if 
        when last two calculated data frame are not equal or data rate changed */ 
	if ((global_data_frm_size != data_frm_size) || (old_data_rate != data_rate)) 
	{
          /* Delete the old PMU Setup File and create new PMU Setup File with new CFG frame */
		if (remove(pmuFilePath) == -1)
			perror("Error in deleting a file");

		fp1 = fopen (pmuFilePath,"wb");

		char buff[50];

		sprintf(buff, "%d", PMU_uport);
		fputs(buff, fp1);
		fputc('\n', fp1);

		sprintf(buff, "%d", PMU_tport);
		fputs(buff, fp1);
		fputc('\n', fp1);

		fputs("CFG 1 ", fp1);
		sprintf(buff, "%d", indx);
		fputs(buff, fp1);
		fputc('\n', fp1);

		fwrite(cfg2_frm, sizeof(unsigned char),indx,fp1);
		fputc('\n', fp1);

		if(hdrFrmSize > 0)
		{
			fputs("HDR 1 ", fp1);
			sprintf(buff, "%d", hdrFrmSize);
			fputs(buff, fp1);
			fputc('\n', fp1);

			fwrite(header, sizeof(unsigned char),hdrFrmSize,fp1);
			fputc('\n', fp1);
		}
		else
		{
			fputs("HDR 0 0", fp1);
			fputc('\n', fp1);
		}

		fclose (fp1);
		free(new_cfg_info);
		validation_result ("Modification in Configuration frame has been successfully updated.");

          	/* Give signal to PMU Server for change in configuration frame */
		ShmPTR->cfg_bit_change_info = 1;
		p1.pid = ShmPTR->pid;

		kill(p1.pid, SIGUSR2);
		printf("Configuration frame modification signal sent to PMU Server.\n");

		/* Set raw data selection as configuration has been changed */
		strcpy(ShmPTR->filePath, "nil");
		ShmPTR->dataFileVar = 0;
		p1.pid = ShmPTR->pid;

		kill(p1.pid, SIGUSR1);
		printf("Set Raw Measurements in Data Frames because configuration of PMU has been changed.\n");

	}
	else		/* If CFG is not modified */
	{
		validation_result ("No modification has been done in Configuration frame.");
	}

	printf("\n-> New configuration frame has been generated of size %d Bytes.\nAnd the new data frame size would be %d Bytes.\n\n[", cfg2_frm_size, data_frm_size);

} /* end of reconfig_cfg_CC() */


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  create_cfg()                                                       */
/* It generate the Configuration Frame as per user entered information.		*/
/* It write in the file PMU Setup File.                                         */
/* It will also reload the saved PMU Setup File give by user.                   */
/* ----------------------------------------------------------------------------	*/

int create_cfg()
{
	/* local Variables*/
	int  j, a, x1, tmp_k;
	int tempi, indx = 0;
	char stn[17],*rline = NULL, *d1;
	unsigned char cline[MAX_STRING_SIZE];

	size_t len = 0;
	ssize_t read;
	FILE *fp1;

	/* Open the saved PMU Setup File and read the CFG frame if any? */
	fp1 = fopen (pmuFilePath,"rb");

	if (fp1 != NULL)
	{
		tempi = 1;
		while(tempi < 6)
		{
			read = getline(&rline, &len, fp1);

			if(read == 0)
				break;

			tempi++;
		}

		if(read > 0)
		{
			d1 = strtok (rline," ");
			d1 = strtok (NULL," ");
			tempi = atoi(d1);

			if (tempi == 1)
			{
				d1 = strtok (NULL,"\n");
				tempi = atoi(d1);

				/* Copy the full Configuration Frame into 'cline' array */
				memset(cline,'\0',sizeof(cline));
				fread(cline, sizeof(unsigned char), tempi, fp1);
				fclose(fp1);

				/* Allocate the memory for ConfigurationFrame object */
				cfg_info = malloc(sizeof(struct ConfigurationFrame));
				cfg_info->cfg_STNname = malloc(17);

				/* Get the CFG size & store globaly */
				temp[0] = cline[2];
				temp[1] = cline[3];
				cfg2_frm_size = c2i(temp);

				/* Get the PMU ID from CFG FRM & store globally */
				temp[0] = cline[4];
				temp[1] = cline[5];
				pmu_id = c2i(temp);
				cfg_info->cfg_pmuID = pmu_id;

				/* Get the PMU ID from CFG FRM & store globally */
				temp_1[0] = cline[14];
				temp_1[1] = cline[15];
				temp_1[2] = cline[16];
				temp_1[3] = cline[17];
				//TB = c2li(temp_1);

				/* Get the Station name from CFG FRM & store globally */
				for(a=0; a<16; a++)
				{
					stn[a] = cline[20+a];
				}
				stn[16] = '\0';
				strcpy(cfg_info->cfg_STNname, stn);

				/* Get the FORMAT word from CFG FRM */
				temp[0] = cline[38];
				temp[1] = cline[39];
				frmt = c2i(temp);

				/* Initialize the frmt bits as in his appropriate global variable */
				if(frmt == 15)
				{
					fdf=1, af=1, pf=1, pn=1;
				}
				else if(frmt == 14)
				{
					fdf=1, af=1, pf=1, pn=0;
				}
				else if(frmt == 13)
				{
					fdf=1, af=1, pf=0, pn=1;
				}
				else if(frmt == 12)
				{
					fdf=1, af=1, pf=0, pn=0;
				}
				else if(frmt == 11)
				{
					fdf=1, af=0, pf=1, pn=1;
				}
				else if(frmt == 10)
				{
					fdf=1, af=0, pf=1, pn=0;
				}
				else if(frmt == 9)
				{
					fdf=1, af=0, pf=0, pn=1;
				}
				else if(frmt == 8)
				{
					fdf=1, af=0, pf=0, pn=0;
				}
				else if(frmt == 7)
				{
					fdf=0, af=1, pf=1, pn=1;
				}
				else if(frmt == 6)
				{
					fdf=0, af=1, pf=1, pn=0;
				}
				else if(frmt == 5)
				{
					fdf=0, af=1, pf=0, pn=1;
				}
				else if(frmt == 4)
				{
					fdf=0, af=1, pf=0, pn=0;
				}
				else if(frmt == 3)
				{
					fdf=0, af=0, pf=1, pn=1;
				}
				else if(frmt == 2)
				{
					fdf=0, af=0, pf=1, pn=0;
				}
				else if(frmt == 1)
				{
					fdf=0, af=0, pf=0, pn=1;
				}
				else
				{
					fdf=0, af=0, pf=0, pn=0;
				}

				/* Store FORMAT in global data structure */
				cfg_info->cfg_fdf = fdf;
				cfg_info->cfg_af = af;
				cfg_info->cfg_pf = pf;
				cfg_info->cfg_pn = pn;

				/* Get the PHNMR from CFG FRM & store globally */
				temp[0] = cline[40];
				temp[1] = cline[41];
				phnmr = c2i(temp);
				cfg_info->cfg_phnmr_val = phnmr;

				/* Get the ANNMR from CFG FRM & store in globally */
				temp[0] = cline[42];
				temp[1] = cline[43];
				annmr = c2i(temp);
				cfg_info->cfg_annmr_val = annmr;

				/* Get the DGNMR from CFG FRM & store globally */
				temp[0] = cline[44];
				temp[1] = cline[45];
				dgnmr = c2i(temp);
				cfg_info->cfg_dgnmr_val = dgnmr;

				/* To escape the some of fields in cfg frame */
				indx = 46 + (16*phnmr) + (16*annmr) + (256*dgnmr) + (4*phnmr) + (4*annmr) + (4*dgnmr) + 2;
				temp[0] = cline[indx++];
				temp[1] = cline[indx++];
				cfgcnt = c2i(temp);

				/* Get the Data Rate from CFG FRM & store globally */
				temp[0] = cline[indx++];
				temp[1] = cline[indx++];
				data_rate = c2i(temp);
				cfg_info->cfg_dataRate = data_rate;

				/* Seprate the Phasor Channels from CFG FRM & store globally */
				cfg_info->cfg_phasor_channels = (char *)malloc((phnmr*16) * sizeof(char));

				for(x1=0, indx=46; x1< (phnmr*16); x1++,indx++)
				{
					cfg_info->cfg_phasor_channels[x1] = cline[indx];
				}

				/* Seprate the Analog Channels from CFG FRM & store globally */
				cfg_info->cfg_analog_channels = (char *)malloc((annmr*16) * sizeof(char));

				for(x1=0, indx; x1< (annmr*16); x1++,indx++)
				{
					cfg_info->cfg_analog_channels[x1] = cline[indx];
				}

				/* Seprate the Digital Channels from CFG FRM & store globally */
				cfg_info->cfg_digital_channels = (char *)malloc((dgnmr*16*16) * sizeof(char));

				for(x1=0, indx; x1< (dgnmr*16*16); x1++,indx++)
				{
					cfg_info->cfg_digital_channels[x1] = cline[indx];
				}

				/* Insert FACTOR VALUES for Phasor, Analog and Digital in new CFG frame */
				for(j=0, tmp_k=0; j<cfg_info->cfg_phnmr_val; j++)/* FACTOR VALUES for Phasor */
				{
					phunit_final_val[tmp_k++] = cline[indx++];
					phunit_final_val[tmp_k++] = cline[indx++];
					phunit_final_val[tmp_k++] = cline[indx++];
					phunit_final_val[tmp_k++] = cline[indx++];
				}
				for(j=0, tmp_k=0; j<cfg_info->cfg_annmr_val; j++)/* FACTOR VALUES for Analog */
				{
					anunit_final_val[tmp_k++] = cline[indx++];
					anunit_final_val[tmp_k++] = cline[indx++];
					anunit_final_val[tmp_k++] = cline[indx++];
					anunit_final_val[tmp_k++] = cline[indx++];
				}
			}
		}
	}
	else 	/* At the time of creating new PMU Setup File */
	{
		/* Store FORMAT bits from global data structure */
		fdf = cfg_info->cfg_fdf;
		af  = cfg_info->cfg_af;
		pf  = cfg_info->cfg_pf;
		pn  = cfg_info->cfg_pn;

		if(fdf==1 && af==1 && pf==1 && pn==1)
		{
			frmt = 15;
		}
		else if(fdf==1 && af==1 && pf==1 && pn==0)
		{
			frmt = 14;
		}
		else if(fdf==1 && af==1 && pf==0 && pn==1)
		{
			frmt = 13;
		}
		else if(fdf==1 && af==1 && pf==0 && pn==0)
		{
			frmt = 12;
		}
		else if(fdf==1 && af==0 && pf==1 && pn==1)
		{
			frmt = 11;
		}
		else if(fdf==1 && af==0 && pf==1 && pn==0)
		{
			frmt = 10;
		}
		else if(fdf==1 && af==0 && pf==0 && pn==1)
		{
			frmt = 9;
		}
		else if(fdf==1 && af==0 && pf==0 && pn==0)
		{
			frmt = 8;
		}
		else if(fdf==0 && af==1 && pf==1 && pn==1)
		{
			frmt = 7;
		}
		else if(fdf==0 && af==1 && pf==1 && pn==0)
		{
			frmt = 6;
		}
		else if(fdf==0 && af==1 && pf==0 && pn==1)
		{
			frmt = 5;
		}
		else if(fdf==0 && af==1 && pf==0 && pn==0)
		{
			frmt = 4;
		}
		else if(fdf==0 && af==0 && pf==1 && pn==1)
		{
			frmt = 3;
		}
		else if(fdf==0 && af==0 && pf==1 && pn==0)
		{
			frmt = 2;
		}
		else if(fdf==0 && af==0 && pf==0 && pn==1)
		{
			frmt = 1;
		}
		else
		{
			frmt = 0;
		}

		/* Accept Number of Phasor values in PHNMR */
		phnmr = cfg_info->cfg_phnmr_val;

		/* Accept Number of Phasor values in ANNMR */
		annmr = cfg_info->cfg_annmr_val;

		/* Accept Number of Phasor values in DGNMR */
		dgnmr = cfg_info->cfg_dgnmr_val;

		/* Calculate the total size of CFG Frame */
		cfg2_frm_size = 54 + (16*phnmr) + (16*annmr) + (4*phnmr) + (4*annmr) + (4*dgnmr) + (256*dgnmr);

		/* Calculate 4/8 bytes for each PHNMR & store globally */
		if (pf == 0)
		{
			data_frm_size = data_frm_size + (4*phnmr);
		}
		else
		{
			data_frm_size = data_frm_size + (8*phnmr);
		}

		/* Calculate 2/4 bytes for each ANNMR & store globally */
		if (af == 0)
		{
			data_frm_size = data_frm_size + (2*annmr);
		}
		else
		{
			data_frm_size = data_frm_size + (4*annmr);
		}

		/* Calculate 2/4 bytes for both (FREQ + DFREQ) & store globally */
		if (fdf == 0)
		{
			data_frm_size = data_frm_size + 4;
		}
		else
		{
			data_frm_size = data_frm_size + 8;
		}

		/* Calculate 2 bytes for each DGNMR & store globally */
		data_frm_size = data_frm_size + (2*dgnmr);

		/* Insert the fields in CFG Frame: sync word */
		memset(cfg2_frm,'\0',sizeof(cfg2_frm));
		cfg2_frm[indx++] = 0xAA;
		cfg2_frm[indx++] = 0x31;

		/* Insert frame size in CFG Frame */
		i2c(cfg2_frm_size, temp);
		B_copy(cfg2_frm, temp, indx, 2);
		indx = indx + 2;

		/* Insert user given PMU ID in CFG Frame  */
		pmu_id = cfg_info->cfg_pmuID;
		i2c(pmu_id, temp);
		B_copy(cfg2_frm, temp, indx, 2);
		indx = indx + 2;

		/* Insert SOC value in CFG Frame */
		soc = 0;
		li2c(soc, temp_1);
		B_copy(cfg2_frm, temp_1, indx, 4);
		indx = indx + 4;

		/* Insert the time quality fields & fraction of second value in CFG Frame, right now it is '00000000' */
		li2c(fracsec, temp_1);
		B_copy(cfg2_frm, temp_1, indx, 4);
		indx = indx + 4;

		/* Insert Time Base for calculating fraction of second in CFG Frame, here it is static 1000000μs */
		li2c(TB, temp_1);
		B_copy(cfg2_frm, temp_1, indx, 4);
		indx = indx + 4;

		/* Insert number of PMU in CFG Frame: Static "0001" */
		i2c(num_pmu, temp);
		B_copy(cfg2_frm, temp, indx, 2);
		indx = indx + 2;

		/* Insert the station name or PMU name in CFG Frame */
		strncpy(temp_5, (const char *)cfg_info->cfg_STNname, 16);
		B_copy(cfg2_frm, (unsigned char *)temp_5, indx, 16);
		indx = indx + 16;

		/* Again insert pmu id in CFG Frame */
		i2c(pmu_id, temp);
		B_copy(cfg2_frm, temp, indx, 2);
		indx = indx + 2;

		/* Insert format word in CFG Frame */
		i2c(frmt, temp);
		B_copy(cfg2_frm, temp, indx, 2);
		indx = indx + 2;

		/* Insert number of phasor or PHNMR in CFG Frame */
		i2c(phnmr, temp);
		B_copy(cfg2_frm, temp, indx, 2);
		indx = indx + 2;

		/* Insert number of analog or ANNMR in CFG Frame */
		i2c(annmr, temp);
		B_copy(cfg2_frm, temp, indx, 2);
		indx = indx + 2;

		/* Insert number of Digital status word or DGNMR in CFG Frame */
		i2c(dgnmr, temp);
		B_copy(cfg2_frm, temp, indx, 2);
		indx = indx + 2;

		/* All channel names or CHNAM fields in CFG frame. */
		/* Insert phasor channel names in CFG frame */
		B_copy(cfg2_frm, (unsigned char *)cfg_info->cfg_phasor_channels, indx, 16*phnmr);
		indx = indx + 16*phnmr;

		/* Insert Digital channel names in CFG frame */
		B_copy(cfg2_frm, (unsigned char *)cfg_info->cfg_analog_channels, indx, 16*annmr);
		indx = indx + 16*annmr;

		/* Insert Digital channel names in CFG frame */
		B_copy(cfg2_frm, (unsigned char *)cfg_info->cfg_digital_channels, indx, 16*16*dgnmr);
		indx = indx + 16*16*dgnmr;

		/* Insert FACTOR VALUES for Phasor, Analog and Digital in CFG frame */
		for(j=0, tmp_k=0; j<phnmr; j++)			/* FACTOR VALUES for Phasor */
		{
			cfg2_frm[indx++] = phunit_final_val[tmp_k++];
			cfg2_frm[indx++] = phunit_final_val[tmp_k++];
			cfg2_frm[indx++] = phunit_final_val[tmp_k++];
			cfg2_frm[indx++] = phunit_final_val[tmp_k++];
		}

		for(j=0, tmp_k=0; j<annmr; j++)			/* FACTOR VALUES for Analog */
		{
			cfg2_frm[indx++] = anunit_final_val[tmp_k++];
			cfg2_frm[indx++] = anunit_final_val[tmp_k++];
			cfg2_frm[indx++] = anunit_final_val[tmp_k++];
			cfg2_frm[indx++] = anunit_final_val[tmp_k++];
		}

		for(j=0; j< dgnmr; j++)			/* FACTOR VALUES for Digital */
		{
			li2c(digunit, temp_1);
			B_copy(cfg2_frm, temp_1, indx, 4);
			indx = indx + 4;
		}

		/* Insert normal frequency or fnom VALUE in CFG frame */
		i2c(cfg_info->cfg_fnom, temp);
		B_copy(cfg2_frm, temp, indx, 2);
		indx = indx + 2;

		/* Insert configuration count or cfgcnt VALUE in CFG frame */
		i2c(cfgcnt, temp);
		B_copy(cfg2_frm, temp, indx, 2);
		indx = indx + 2;

		/* Insert Data Rate in CFG frame */
		data_rate = cfg_info->cfg_dataRate;
		i2c(data_rate, temp);
		B_copy(cfg2_frm, temp, indx, 2);
		indx = indx + 2;

        /* Insert current SOC value in CFG Frame */
        gettimeofday(&tim, NULL);
        soc = (long) tim.tv_sec;
        li2c(soc, temp_1);
        B_copy(cfg2_frm, temp_1, 6, 4);

        /* Insert the time quality fields & fraction of second value in CFG Frame */
        //	gettimeofday(&tim, NULL);
        //	t1=tim.tv_sec+(tim.tv_usec/1000000.0);
        //	fracsec = (t1-soc)*TB;
        fracsec = (long) (tim.tv_usec*TB/1e6);
        li2c(fracsec, temp_1);
        B_copy(cfg2_frm, temp_1, 10, 4);

		/* Calculation & insert the checksum VALUE of new CFG frame (up to now) */
		chk = compute_CRC(cfg2_frm,indx);

		cfg2_frm[indx++] = (chk >> 8) & ~(~0<<8);  	/* CHKSUM high byte */
		cfg2_frm[indx++] = (chk ) & ~(~0<<8);     	/* CHKSUM low byte  */

		char buff[50];

          /* Create a new PMU Setup File and save the info with 'pmuPMU-ID.txt' name */
		memset(pmuFilePath, '\0', 200);
		strcpy(pmuFilePath, pmuFolderPath);
		strcat(pmuFilePath, "/");
		strcat(pmuFilePath, "pmu");
		sprintf(buff, "%d", pmu_id);
		strcat(pmuFilePath, buff);
		strcat(pmuFilePath, ".bin");
		pmuFilePath[strlen(pmuFilePath)] = '\0';

		fp1 = fopen (pmuFilePath,"wb");

		sprintf(buff, "%d\n%d\n%d\n%s\n", PMU_uport, PMU_tport, PMU_mulport,PMU_mulip);
		fputs(buff, fp1);

		fputs("CFG 1 ", fp1);
		sprintf(buff, "%d", indx);
		fputs(buff, fp1);
		fputc('\n', fp1);

		fwrite(cfg2_frm, sizeof(unsigned char),indx,fp1);
		fputc('\n', fp1);

		fclose (fp1);

		printf("\nCFG Frame size = %d-Bytes, ", cfg2_frm_size);
		printf("& Data Frame size = %d-Bytes.\n", data_frm_size);
	}

     /* Now send a signal for CFG file path to PMU Server process*/
	strcpy(ShmPTR->cfgPath, pmuFilePath);
	ShmPTR->dataFileVar = 2;

	p1.pid = ShmPTR->pid;
	kill(p1.pid, SIGUSR1);
	printf("\nConfiguration file path has been sent to PMU Server through signal.\n");

	return 0;
} /* end of function() */


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  show_pmu_details (GtkWidget *widget, gpointer udata)               */
/* It display the details of PMU Server and Configuartion details.              */
/* ----------------------------------------------------------------------------	*/

void show_pmu_details (GtkWidget *widget, gpointer udata)
{
	/* local variables */
	GtkWidget *main_box, *view, *scrolled_window, *new_window;
    gint event;
    FILE *fp1;

	/* Open the saved PMU Setup File and read the informations */
	fp1 = fopen (fptr,"rb");
    new_window = gtk_dialog_new_with_buttons ("PMU Properties",
            GTK_WINDOW (pmu_data->Pmu_Simulator),
            GTK_DIALOG_MODAL,
            "_Close", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response (GTK_DIALOG(new_window), GTK_RESPONSE_CANCEL);
    g_signal_connect (new_window, "destroy", G_CALLBACK (gtk_widget_destroy), new_window);
    gtk_window_set_default_size (GTK_WINDOW (new_window), -1, 400);
    gtk_window_set_resizable (GTK_WINDOW (new_window), FALSE);
    /* Create a new scrolled window */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_size_request (scrolled_window, -1, 400);

    /* The policy is one of GTK_POLICY AUTOMATIC, or GTK_POLICY_ALWAYS. */
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

    main_box = gtk_dialog_get_content_area (GTK_DIALOG (new_window));
    gtk_box_pack_start (GTK_BOX (main_box), scrolled_window, TRUE, FALSE, 0);
    view = create_pmu_view (fp1);
    fclose(fp1);
    gtk_container_add (GTK_CONTAINER (scrolled_window), view);
    /* Finally show the setup_display_window. */
    gtk_widget_show_all (new_window);
    event = gtk_dialog_run(GTK_DIALOG(new_window));
    if (event == GTK_RESPONSE_CANCEL)
    {
        gtk_widget_destroy(GTK_WIDGET (new_window));
    }
};

/**************************************** End of File *******************************************************/



//Main program ####################################################    Main program
int create_fifo_buffer() 
{
    char temp_val[1024];
    lines=50000;
    fp=fopen("../DataDir/data.csv", "r");  
    if (!fp) 
    {
        printf("Can't open file\n");
        return(0);
    }

    if(fgets(temp_val, 1024, fp)!=NULL) //To get approx valueof length for each line and account for -ive signs
    {
        temp_val[strlen(temp_val)-1]='\0';
        size_buffer=strlen(temp_val)+30;//+12 to account for sign
        fseek(fp, 0, SEEK_SET);
    }

    for(int local_count=1;local_count<lines+1;local_count++)
    {
        char* temp_string = malloc(size_buffer*sizeof(unsigned char));	
        if(fgets(temp_string,size_buffer, fp)==NULL)
        {
            fseek(fp, 0, SEEK_SET);
            fgets(temp_string,size_buffer, fp);
        }
        insert_element(temp_string);
        free(temp_string);
    }
		/* local Variables*/
	int  j, a, x1, tmp_k;
	int tempi, indx = 0;
	char stn[17],*rline = NULL, *d1;
	unsigned char cline[MAX_STRING_SIZE];

	size_t len = 0;
	ssize_t read;
	FILE *fp1;

	/* Open the saved PMU Setup File and read the CFG frame if any? */
	fp1 = fopen (pmuFilePath,"rb");

	if (fp1 != NULL)
	{
		tempi = 1;
		while(tempi < 6)
		{
			read = getline(&rline, &len, fp1);

			if(read == 0)
				break;

			tempi++;
		}

		if(read > 0)
		{
			d1 = strtok (rline," ");
			d1 = strtok (NULL," ");
			tempi = atoi(d1);

			if (tempi == 1)
			{
				d1 = strtok (NULL,"\n");
				tempi = atoi(d1);

				/* Copy the full Configuration Frame into 'cline' array */
				memset(cline,'\0',sizeof(cline));
				fread(cline, sizeof(unsigned char), tempi, fp1);
				fclose(fp1);

				/* Allocate the memory for ConfigurationFrame object */
				cfg_info = malloc(sizeof(struct ConfigurationFrame));
				cfg_info->cfg_STNname = malloc(17);

				/* Get the CFG size & store globaly */
				temp[0] = cline[2];
				temp[1] = cline[3];
				cfg2_frm_size = c2i(temp);

				/* Get the PMU ID from CFG FRM & store globally */
				temp[0] = cline[4];
				temp[1] = cline[5];
				pmu_id = c2i(temp);
				cfg_info->cfg_pmuID = pmu_id;

				/* Get the PMU ID from CFG FRM & store globally */
				temp_1[0] = cline[14];
				temp_1[1] = cline[15];
				temp_1[2] = cline[16];
				temp_1[3] = cline[17];
				//TB = c2li(temp_1);

				/* Get the Station name from CFG FRM & store globally */
				for(a=0; a<16; a++)
				{
					stn[a] = cline[20+a];
				}
				stn[16] = '\0';
				strcpy(cfg_info->cfg_STNname, stn);

				/* Get the FORMAT word from CFG FRM */
				temp[0] = cline[38];
				temp[1] = cline[39];
				frmt = c2i(temp);

				/* Initialize the frmt bits as in his appropriate global variable */
				if(frmt == 15)
				{
					fdf=1, af=1, pf=1, pn=1;
				}
				else if(frmt == 14)
				{
					fdf=1, af=1, pf=1, pn=0;
				}
				else if(frmt == 13)
				{
					fdf=1, af=1, pf=0, pn=1;
				}
				else if(frmt == 12)
				{
					fdf=1, af=1, pf=0, pn=0;
				}
				else if(frmt == 11)
				{
					fdf=1, af=0, pf=1, pn=1;
				}
				else if(frmt == 10)
				{
					fdf=1, af=0, pf=1, pn=0;
				}
				else if(frmt == 9)
				{
					fdf=1, af=0, pf=0, pn=1;
				}
				else if(frmt == 8)
				{
					fdf=1, af=0, pf=0, pn=0;
				}
				else if(frmt == 7)
				{
					fdf=0, af=1, pf=1, pn=1;
				}
				else if(frmt == 6)
				{
					fdf=0, af=1, pf=1, pn=0;
				}
				else if(frmt == 5)
				{
					fdf=0, af=1, pf=0, pn=1;
				}
				else if(frmt == 4)
				{
					fdf=0, af=1, pf=0, pn=0;
				}
				else if(frmt == 3)
				{
					fdf=0, af=0, pf=1, pn=1;
				}
				else if(frmt == 2)
				{
					fdf=0, af=0, pf=1, pn=0;
				}
				else if(frmt == 1)
				{
					fdf=0, af=0, pf=0, pn=1;
				}
				else
				{
					fdf=0, af=0, pf=0, pn=0;
				}

				/* Store FORMAT in global data structure */
				cfg_info->cfg_fdf = fdf;
				cfg_info->cfg_af = af;
				cfg_info->cfg_pf = pf;
				cfg_info->cfg_pn = pn;

				/* Get the PHNMR from CFG FRM & store globally */
				temp[0] = cline[40];
				temp[1] = cline[41];
				phnmr = c2i(temp);
				cfg_info->cfg_phnmr_val = phnmr;

				/* Get the ANNMR from CFG FRM & store in globally */
				temp[0] = cline[42];
				temp[1] = cline[43];
				annmr = c2i(temp);
				cfg_info->cfg_annmr_val = annmr;

				/* Get the DGNMR from CFG FRM & store globally */
				temp[0] = cline[44];
				temp[1] = cline[45];
				dgnmr = c2i(temp);
				cfg_info->cfg_dgnmr_val = dgnmr;

				/* To escape the some of fields in cfg frame */
				indx = 46 + (16*phnmr) + (16*annmr) + (256*dgnmr) + (4*phnmr) + (4*annmr) + (4*dgnmr) + 2;
				temp[0] = cline[indx++];
				temp[1] = cline[indx++];
				cfgcnt = c2i(temp);

				/* Get the Data Rate from CFG FRM & store globally */
				temp[0] = cline[indx++];
				temp[1] = cline[indx++];
				data_rate = c2i(temp);
				cfg_info->cfg_dataRate = data_rate;

				/* Seprate the Phasor Channels from CFG FRM & store globally */
				cfg_info->cfg_phasor_channels = (char *)malloc((phnmr*16) * sizeof(char));

				for(x1=0, indx=46; x1< (phnmr*16); x1++,indx++)
				{
					cfg_info->cfg_phasor_channels[x1] = cline[indx];
				}

				/* Seprate the Analog Channels from CFG FRM & store globally */
				cfg_info->cfg_analog_channels = (char *)malloc((annmr*16) * sizeof(char));

				for(x1=0, indx; x1< (annmr*16); x1++,indx++)
				{
					cfg_info->cfg_analog_channels[x1] = cline[indx];
				}

				/* Seprate the Digital Channels from CFG FRM & store globally */
				cfg_info->cfg_digital_channels = (char *)malloc((dgnmr*16*16) * sizeof(char));

				for(x1=0, indx; x1< (dgnmr*16*16); x1++,indx++)
				{
					cfg_info->cfg_digital_channels[x1] = cline[indx];
				}

				/* Insert FACTOR VALUES for Phasor, Analog and Digital in new CFG frame */
				for(j=0, tmp_k=0; j<cfg_info->cfg_phnmr_val; j++)/* FACTOR VALUES for Phasor */
				{
					phunit_final_val[tmp_k++] = cline[indx++];
					phunit_final_val[tmp_k++] = cline[indx++];
					phunit_final_val[tmp_k++] = cline[indx++];
					phunit_final_val[tmp_k++] = cline[indx++];
				}
				for(j=0, tmp_k=0; j<cfg_info->cfg_annmr_val; j++)/* FACTOR VALUES for Analog */
				{
					anunit_final_val[tmp_k++] = cline[indx++];
					anunit_final_val[tmp_k++] = cline[indx++];
					anunit_final_val[tmp_k++] = cline[indx++];
					anunit_final_val[tmp_k++] = cline[indx++];
				}
			}
		}
	}
    write_data();
    make_config();    
    printf("\n Files created \n");		
    fclose(fp);
    delete_all();
    return(0);
}
//check if buffer is empty
bool is_empty() 
{
   return head == NULL;
}
//delete first item ####################################################    DELETE ONE
void delete_one() 
{
buffercount--;
//save reference to first link
struct node *tempLink =tail;
if(tail->next == tail) 
      {
      tail = NULL;
      head=NULL;
      }     
else
   tail = tail->next;
free(tempLink->data);
free(tempLink);
}
//delete all items ####################################################    DELETE ALL
void delete_all()
{
while(!is_empty()) 
	{            
	delete_one();
	}   
	buffercount=0;
}
//insert link at the first location  ####################################################    Insert Element
void insert_element(char dat[]) 
{
    //printf("\n++%s++",dat);
    buffercount++;
    struct node *link = (struct node*) malloc(sizeof(struct node));
    link->data = malloc(size_buffer*sizeof(unsigned char));
    dat[strlen(dat)-1]=0;
    strcpy(link->data,dat);
    if (is_empty()) 
    {
        head = link;
        head->next = head;
        tail=link;
        buffercount=1;
    }
    else   
    {
        if(buffercount==(bufferlength+1))
        {
            delete_one();
        }
        head->next = link;

        head = link;
        head->next=head;
    }  
}
//display the elements in the buffer  ####################################################    Display Buffer
void print_buffer() 
{
struct node *ptr = tail;
printf("\n");
if(tail != NULL) 
	{
	while(ptr->next != ptr) 
		{     
         	printf("(%s)\n",ptr->data);
         	ptr = ptr->next;
      		}
     	printf("(%s)\n",ptr->data); 
   	}
}

//Make config data
void make_config()
{ 
    int flag_space=0;
    FILE *cfg_file;

		char pmuFilePath1[200];
        char buff1[50];
		memset(pmuFilePath1, '\0', 200);
		strcpy(pmuFilePath1,"..");
		strcat(pmuFilePath1, "/share/");
		strcat(pmuFilePath1, "pmu_");
		sprintf(buff1, "%d", pmu_id);
		strcat(pmuFilePath1, buff1);
		strcat(pmuFilePath1, ".cfg");
		pmuFilePath1[strlen(pmuFilePath1)] = '\0';


    cfg_file = fopen(pmuFilePath1,"w");
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    //printf("\n length %d\n",strlen(cfg_info->cfg_STNname));
    for(int local_count=0;local_count<strlen(cfg_info->cfg_STNname);local_count++)
    { 
        flag_space=0;
        for(int local_count_1=local_count;local_count_1<strlen(cfg_info->cfg_STNname);local_count_1++)  //Checking for space
        {if(cfg_info->cfg_STNname[local_count_1]!=' ') flag_space=1;}
        if(flag_space==0) break;

        fprintf(cfg_file,"%c",cfg_info->cfg_STNname[local_count]);
        //printf("%c",cfg_info->cfg_STNname[local_count]);
    }
    fprintf(cfg_file,",%d,2013\n",pmu_id);
    //printf(",%d,2013\n",pmu_id);
    fprintf(cfg_file,"%d,%dA,%dD\n",phnmr+dgnmr*16,phnmr,dgnmr*16);
    //printf("%d,%dA,%dD\n",phnmr+dgnmr*16,phnmr,dgnmr*16);
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    //printf("\nStation Name:%s",cfg_info->cfg_STNname);
    //printf("PMU ID:%d",pmu_id);
    //printf("\trev_id:2013");


    //printf("\nNo of phasors: %d",phnmr);
    //printf("\nNo of analog values: %d",annmr);
    //printf("\nNo of digital values: %d",dgnmr);
    //printf("\nPhasor Channels\n");
    int count=0,analog=1;
    for(int local_count=0; local_count< (phnmr); local_count++)
    {				
        fprintf(cfg_file,"%d,",analog++);	
        //printf("%d,",analog-1);
        for(int local_count_1=0; local_count_1< 16; local_count_1++) 
        {
            flag_space=0;
            for(int local_count_2=local_count_1;local_count_2<16;local_count_2++)  //Checking for non-space chara ahead
            {if(cfg_info->cfg_phasor_channels[local_count_2+count*16]!=' ') flag_space=1;}
            if(flag_space==0) break;
            //printf("%c",cfg_info->cfg_phasor_channels[local_count_1+count*16]);
            fprintf(cfg_file,"%c",cfg_info->cfg_phasor_channels[local_count_1+count*16]);
        }
        if(phunit_final_val[count*4]==0)
        {
            fprintf(cfg_file,", , ,kV,1,0,0,-3.40282e+38,3.40282e+38,1,1,P\n");
            //printf(", , ,kV,1,0,0,-3.40282e+38,3.40282e+38,1,1,P\n");//channel_multiplier=1 channel ocfg_fileset=0
        }
        else if(phunit_final_val[count*4]==1)
        {
            fprintf(cfg_file,", , ,kA,1,0,0,-3.40282e+38,3.40282e+38,1,1,P\n");
            //printf(", , ,kA,1,0,0,-3.40282e+38,3.40282e+38,1,1,P\n");
        }
        count++;	    
    }

    /*
       printf("\n\n");
       printf("\t%d\t\n",phunit_final_val[0]);
       printf("\t%d\t\n",phunit_final_val[4]);
       printf("\n\n");
       printf("\t%d\t\n",anunit_final_val[0]);
       printf("\t%d\t\n",anunit_final_val[4]);
       printf("\t%d\t\n",anunit_final_val[8]);
       printf("\t%d\t\n",anunit_final_val[12]);
       printf("\t%d\t\n",anunit_final_val[16]);
       printf("\t%d\t\n",anunit_final_val[20]);

       printf("\n\n"); */

    //printf("Digital Channels\n");
    count=0;
    analog=1;
    for(int local_count=0; local_count< (dgnmr*16); local_count++)
    {
        fprintf(cfg_file,"%d,",analog++);
        //printf("%d,",analog-1);	
        for(int local_count_1=0; local_count_1< 16; local_count_1++) 
        { //if(cfg_info->cfg_digital_channels[local_count_1+count*16]!=' ')
            flag_space=0;
            for(int local_count_2=local_count_1;local_count_2<16;local_count_2++)  //Checking for space
            {if(cfg_info->cfg_digital_channels[local_count_2+count*16]!=' ') flag_space=1;}
            if(flag_space==0) break;
            //printf("%c",cfg_info->cfg_digital_channels[local_count_1+count*16]);
            fprintf(cfg_file,"%c",cfg_info->cfg_digital_channels[local_count_1+count*16]);
        }
        count++;
        fprintf(cfg_file,", , ,0\n");
        //printf(", , ,0\n");
    }
    fprintf(cfg_file,"%d\n",f);
    //printf("%d\n",f);

    fprintf(cfg_file,"%s\n","1");
    //printf("%s\n","1");


    //fprintf(cfg_file,"%d,",cfg_info->cfg_dataRate);
    //printf("%d,",cfg_info->cfg_dataRate);

    int datarate_1=1000;
    fprintf(cfg_file,"%d,",1000);
    //printf("%d,",1000);


    fprintf(cfg_file,"%.0f\n",end_sample);
    //printf("%.0f\n",end_sample);

    fprintf(cfg_file,"%s\n",start_time);
    //printf("%s\n",start_time);

    //fprintf(cfg_file,"%s\n","Time_trigger");
    fprintf(cfg_file,"%s\n",start_time);
    //printf("%s\n","Time_trigger");

    fprintf(cfg_file,"%s\n","float32");
    //printf("%s\n","float32");

    fprintf(cfg_file,"%s\n","1000");//time_stamp_multiplication_factor
    //printf("%s\n","1000");

    fprintf(cfg_file,"%s\n","+5h30,+5h30");
    //printf("%s\n","+5h30,+5h30");

    fprintf(cfg_file,"%s\n","0000,0");
    //printf("%s\n","0000,0");	
    fclose(cfg_file);	        
    /*cfg_file = fopen("../share/pmu.cfg","r");	        
      fseek(cfg_file, 0, SEEK_END);
      long fsize = ftell(cfg_file );
      fseek(cfg_file, 0, SEEK_SET);  // same as rewind(f); 

      char *string_cfg_file = malloc(fsize + 1);
      fread(string_cfg_file,sizeof(char),fsize,cfg_file);
      string_cfg_file[fsize] = 0;
      printf("\n Size :  %ld \t Text :  %s  END\n",fsize,string_cfg_file);



      fclose(cfg_file); */
}


void write_data()
{
    FILE *dat_file;
    long int sampleno=0;

		char pmuFilePath2[200];
        char buff2[50];
		memset(pmuFilePath2, '\0', 200);
		strcpy(pmuFilePath2,"..");
		strcat(pmuFilePath2, "/share/");
		strcat(pmuFilePath2, "pmu_");
		sprintf(buff2, "%d", pmu_id);
		strcat(pmuFilePath2, buff2);
		strcat(pmuFilePath2, ".dat");
		pmuFilePath2[strlen(pmuFilePath2)] = '\0';

    dat_file = fopen(pmuFilePath2,"wb");
    struct node *ptr = tail;
    if(tail != NULL) 
    {
        for(int local_count=0;local_count<0.01*bufferlength;local_count++)
        {
            ptr = ptr->next;    //Skipping first few values to prevent overlap with running code
        }
        while(ptr->next != ptr) 
        {
            sampleno++;     
            write_data_helper(ptr->data,sampleno,dat_file);
            ptr = ptr->next;
        }
        sampleno++;
        write_data_helper(ptr->data,sampleno,dat_file);
        end_sample=sampleno;
    } 	
    fclose(dat_file);
}
void write_data_helper(char* temp,long int sampleno,FILE* dat_file)
{ 
    int field_count = 0;
    float a;
    //float zero=0;
    int digitalcount=0;
    int dig[100];                
    char *field = strtok(temp, ",");
    while (field) 
    {
        if(field_count == 0)  
        { 
            fwrite(&sampleno,sizeof(char),4,dat_file);
            fwrite(&sampleno,sizeof(char),4,dat_file);

            if(sampleno==1) strcpy(start_time,field);
            // else if(sampleno==bufferlength) end_time=field;
        } 
        else if(field_count == 1)  
        {   if(sampleno==1) 
            {
                strcat(start_time,",");
                strcat(start_time,field);
            }
        }      	               
        else if(field_count<=phnmr+1)
        {
            a=strtod(field , NULL);
            fwrite(&a,sizeof(char),4,dat_file);
        }
        else if(field_count>phnmr+1)
        {
            a=strtod(field , NULL);
            dig[digitalcount++]=(int)a;
        }
        field = strtok(NULL, ",");	
        field_count++;
    }
    for(int local_count_1=0;local_count_1<dgnmr;local_count_1++)
    {
        int b=0;
        for(int local_count=0;local_count<16;local_count++)
        {
            b=b+dig[local_count_1+local_count]*(1<<local_count);
        }
        fwrite(&b,sizeof(char),2,dat_file);
    }
}


