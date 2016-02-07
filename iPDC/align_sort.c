/* ----------------------------------------------------------------------------- 
 * align_sort.c
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


#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <pthread.h>
#include "parser.h"
#include "global.h" 
#include "align_sort.h" 
#include "connections.h"
#include "dallocate.h"


/* ------------------------------------------------------------------------------------	*/
/*				Functions in align_sort.c				*/
/* ------------------------------------------------------------------------------------	*/

/*	1.  void time_align(struct data_frame *df)          	    	*/
/*	2.  void assign_df_to_TSB(struct data_frame *df,int index) 	*/
/*	3.  void dispatch(int index)					*/
/*	4.  void sort_data_inside_TSB(int index)			*/
/*	5.  void clear_TSB(int index)					*/
/*	6.  void create_dataframe(int index)				*/
/*	7.  void create_cfgframe()					*/

/* -----------------------------------------------------------------------------*/



//int i; // Commented by Kedar
//front = -1;
//rear = -1;
//pthread_mutex_t mutex_on_TSB = PTHREAD_MUTEX_INITIALIZER;


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  time_align():                                	     		*/
/* It searches for the correct TSB[index] where data frame df is to be 		*/
/* assigned. If the df has soc and fracsec which is older then soc and fracsec  */
/* of TSB[first] then we discard the data frame					*/
/* ----------------------------------------------------------------------------	*/

int front = -1;
int rear=-1;
int Tsb_Count = 0;
int old_df_front = -1;
int old_df_rear = -1;

void time_align(struct data_frame *df) {

	int flag = 0;
	int i;
	unsigned int IDcode;
	unsigned int df_soc,df_fracsec,tsb_soc,tsb_fracsec;

	if (front == -1) { // TSB is used for the first time

		front = rear = 0;
		assign_df_to_TSB(df,front);		
		
/*		struct  cfg_frame *temp_cfg = cfgfirst;		
		while(temp_cfg != NULL) {
			
			printf("%d--",to_intconvertor(temp_cfg->idcode));
			temp_cfg = temp_cfg->cfgnext;
		}
		printf("\n");				
*/		return;	

	} else {

		df_soc = to_long_int_convertor(df->soc);
		unsigned char *fsec;
		fsec = malloc(3*sizeof(unsigned char));
		fsec[0] = df->fracsec[1];
		fsec[1] = df->fracsec[2];
		fsec[2] = df->fracsec[3];
		df_fracsec = to_long_int_convertor1(fsec);

		unsigned char *tsb_fsec;
		tsb_fsec = malloc(3*sizeof(unsigned char));
		tsb_fsec[0] = TSB[front].fracsec[1];
		tsb_fsec[1] = TSB[front].fracsec[2];
		tsb_fsec[2] = TSB[front].fracsec[3];
				
		tsb_soc = to_long_int_convertor((unsigned char *)TSB[front].soc);

		tsb_fracsec = to_long_int_convertor1(tsb_fsec);		
		IDcode = to_intconvertor(df->idcode);	

//		printf("Id = %u df soc %u fsec =%u front tsb_soc %u tsb_fsec %u\n",IDcode,df_soc,df_fracsec,tsb_soc,tsb_fracsec);	
	
		if((old_df_front != -1) &&  ((df_soc < ODFT[old_df_front].soc) || ((df_soc == ODFT[old_df_front].soc) && (df_fracsec < ODFT[old_df_front].fracsec)))) {
//		if((df_soc < tsb_soc) || ((df_soc == tsb_soc) && (df_fracsec < tsb_fracsec))) {

			free_dataframe_object(df);			
			myfree(tsb_fsec);
			myfree(fsec);
			unsigned char *tsb_fsec1;	
			tsb_fsec1 = malloc(3*sizeof(unsigned char));
			tsb_fsec1[0] = TSB[rear].fracsec[1];
			tsb_fsec1[1] = TSB[rear].fracsec[2];
			tsb_fsec1[2] = TSB[rear].fracsec[3];
			unsigned int tsb_fracsec1 = to_long_int_convertor1(tsb_fsec1);	
			unsigned int tsb_soc1 = to_long_int_convertor((unsigned char *)TSB[rear].soc);						
			printf("LOssId = %u df soc %u fsec =%u front tsb_soc %u tsb_fsec %u rear soc %u fsec %u \n",IDcode,df_soc,df_fracsec,tsb_soc,tsb_fracsec,tsb_soc1,tsb_fracsec1);	

			return;	
		}

		for(i = 0; i<MAXTSB; i++) {
				//	memset(dLog,'\0',2500);
			if((TSB[i].soc != NULL ) &&(TSB[i].first_data_frame != NULL)) {

				tsb_soc = to_long_int_convertor((unsigned char *)TSB[i].soc);
				tsb_fsec[0] = TSB[i].fracsec[1];
				tsb_fsec[1] = TSB[i].fracsec[2];
				tsb_fsec[2] = TSB[i].fracsec[3];
				tsb_fracsec = to_long_int_convertor1(tsb_fsec);				

				if(df_soc == tsb_soc) 
				{
					if(tsb_fracsec == df_fracsec) {	
						
						flag = 1;						
						break;
					} 
				}
			}
		}
		myfree(tsb_fsec);
		myfree(fsec);
	} // if other than the front = -1

		
	if(flag) {

		//Assign to existing TSB
		assign_df_to_TSB(df,i);	
//		printf("*aa front %d,rear %d Tsb  Count %d,%u,%u,%u\n",front,rear,Tsb_Count,IDcode,df_soc,df_fracsec);
			
	} else {

		if(Tsb_Count == MAXTSB) {	

			//intermediate_dispatch(front);
			dispatch(front); 
		//	writeTimeToLog(44,IDcode,df_soc,df_fracsec); 

			front = (front + 1) % MAXTSB;

			rear = (rear + 1) % MAXTSB;
//			printf("*aa\n");
			assign_df_to_TSB(df,rear);
			tsb_soc = to_long_int_convertor(TSB[front].soc);
//			printf("**aa front %d,rear %d Tsb  Count %d,%u,%u,%u\n",front,rear,Tsb_Count,IDcode,df_soc,df_fracsec);
	
		} else {
	
			if (front == rear)
			{
				//memset(dLog,'\0',2500);

				if (Tsb_Count == 0)
				{
//					printf("aa\n");	
					assign_df_to_TSB(df,rear);					
					tsb_soc = to_long_int_convertor(TSB[front].soc);
//					printf("aa front %d,rear %d Tsb  Count %d %u,%u,%u\n",front,rear,Tsb_Count,IDcode,df_soc,df_fracsec);

				} else {

					rear=(rear + 1) % MAXTSB;					
//					printf("bb\n");
					assign_df_to_TSB(df,rear);
					tsb_soc = to_long_int_convertor(TSB[front].soc);
//					printf("bb front %d,rear %d Tsb  Count %d %u,%u,%u\n",front,rear,Tsb_Count,IDcode,df_soc,df_fracsec);				
				}
			} else {

					rear=(rear + 1) % MAXTSB;
//					printf("cc\n");	
					assign_df_to_TSB(df,rear);					
//					printf("cc front %d,rear %d Tsb  Count %d,%u,%u,%u\n",front,rear,Tsb_Count,IDcode,df_soc,df_fracsec);
			}
		} 	
	}
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  assign_df_to_TSB():                               	     		*/
/* It assigns the arrived data frame df to TSB[index]				*/
/* ----------------------------------------------------------------------------	*/

void assign_df_to_TSB(struct data_frame *df,int index) { 
	
	/* Check if the TSB is used for the first time. If so we need to 
	   allocate memory to its member variables */

//	printf("Inside assign_df id = %d index %d front = %d,rear = %d ,soc = %d, fsec = %d\n",IDcode,index,front,rear,df_soc,df_fracsec);

	if(TSB[index].soc == NULL) { // 1 if
		
		struct  cfg_frame *temp_cfg = cfgfirst;

		TSB[index].soc = malloc(5);
		TSB[index].fracsec = malloc(5);

		memset(TSB[index].soc,'\0',5);
		memset(TSB[index].fracsec,'\0',5);

		copy_cbyc((unsigned char *)TSB[index].soc,df->soc,4);
		copy_cbyc((unsigned char *)TSB[index].fracsec,df->fracsec,4);
		TSB[index].num = 0;

		// Kedar 06-05-2013 v1.4 initialize the old soc and fracsec 
		unsigned char *fsec;
		fsec = malloc(3*sizeof(unsigned char));
		fsec[0] = TSB[index].fracsec[1];
		fsec[1] = TSB[index].fracsec[2];
		fsec[2] = TSB[index].fracsec[3];			
			
		if(old_df_front == -1) {

			old_df_front = 0;
			old_df_rear = 0;

			ODFT[old_df_rear].soc = to_long_int_convertor(TSB[index].soc);
			ODFT[old_df_rear].fracsec = to_long_int_convertor1(fsec);
			
		} else {
		
			old_df_rear = (old_df_rear + 1)%OLDFRAMECOUNT;
			ODFT[old_df_rear].soc = to_long_int_convertor(TSB[index].soc);
			ODFT[old_df_rear].fracsec = to_long_int_convertor1(fsec);
		} 
//		free(fsec);

		Tsb_Count++;
		TSB[index].first_data_frame = df; /* Assign df to the 'first_data_frame' in the data frame linked list of TSB[index] */
		TSB[index].count = 1;
		/* Now we need to store the pmu/pdc id in the pmupdc_id_list that would be required while sorting */
		struct pmupdc_id_list *temp_pmuid;
		while(temp_cfg != NULL) {

			/* Create a node of the type 'pmupdc_id_list' and copy the pmu/pde id from the cfg to it */
			struct pmupdc_id_list *pmuid = malloc(sizeof(struct pmupdc_id_list));
			pmuid->idcode = malloc(3);
			memset(pmuid->idcode,'\0',3);
			copy_cbyc((unsigned char *)pmuid->idcode,temp_cfg->idcode,2);
			pmuid->num_pmu = to_intconvertor(temp_cfg->num_pmu);
			pmuid->nextid = NULL;
			TSB[index].num++;

			if(TSB[index].idlist == NULL) { /* Assign the pmuid to the idlist as it is the first id in the list */

				TSB[index].idlist = temp_pmuid = pmuid;

			} else {

				temp_pmuid->nextid = pmuid;
				temp_pmuid = pmuid;

			}			

			temp_cfg = temp_cfg->cfgnext;				
		} // while ends . A pmu/pdc id list is created for the TSB[index]
	
		TSB[index].counter = 1;
		
		unsigned int IDcode = to_intconvertor(df->idcode);		
		unsigned int df_soc = to_long_int_convertor(df->soc);

		fsec[0] = df->fracsec[1];
		fsec[1] = df->fracsec[2];
		fsec[2] = df->fracsec[3];
		unsigned int df_fracsec = to_long_int_convertor1(fsec);
//		writeTimeToLog(2,IDcode,df_soc,df_fracsec); // Checkpoint time before sorting
		free(fsec);
		if(TSB[index].count == TSB[index].num) {

				int xx = index;			
				//writeTimeToLog(2,IDcode,'d',df_soc,df_fracsec); // Checkpoint time before sorting
				intermediate_dispatch(xx);
		} else {

			//writeTimeToLog(2,IDcode,'d',df_soc,df_fracsec); // Checkpoint time before sorting
		}


//		printf("*dd Index %d front %d  rear %d, Tcount %d, %u,%u,%u\n",index,front,rear,Tsb_Count,IDcode,df_soc,df_fracsec);
		int k = 0;
		for (k=0;k<MAXTSB;k++) {

			if((k != index) && (TSB[k].first_data_frame != NULL) && (TSB[k].soc != NULL )) {

				TSB[k].counter++;
		/*		df_soc = to_long_int_convertor(TSB[k].first_data_frame->soc);
				IDcode = to_intconvertor(TSB[k].first_data_frame->idcode);						
					
				fsec[0] = TSB[k].first_data_frame->fracsec[1];
				fsec[1] = TSB[k].first_data_frame->fracsec[2];
				fsec[2] = TSB[k].first_data_frame->fracsec[3];			
				unsigned int df_fracsec = to_long_int_convertor1(fsec); 
*/
//				printf("dd Counter %d Index %d front %d  rear %d, Tcount %d ,%u,%u,%u\n",TSB[k].counter,k,front,rear,Tsb_Count,IDcode,df_soc,df_fracsec);
				if(TSB[k].counter == MAXTSB) {
	
				//	writeTimeToLog(46,IDcode,df_soc,df_fracsec);
//					printf("dd Index %d front %d  rear %d, Tcount %d ,%u,%u,%u\n",k,front,rear,Tsb_Count,IDcode,df_soc,df_fracsec);
					intermediate_dispatch(k);
				}
			}
		}
			
	} else { // 1 if else 


		//printf("Inside assign_df id = %d index %d front = %d,rear = %d\n",IDcode,index,front,rear);
		struct  cfg_frame *temp_cfg = cfgfirst;
		if(TSB[index].first_data_frame == NULL) { // 2 if 

			/* After TSB[index] is cleared this is the first data frame for it. 
			The memory for the member variables of TSB[index] has already 
			been allocated. Hence after dispatch() and clear_TSB() operation
			this TSB is to be assigned the data_frame for the first time. */

			copy_cbyc((unsigned char *)TSB[index].soc,df->soc,4);
			copy_cbyc((unsigned char *)TSB[index].fracsec,df->fracsec,4);			

			// Kedar 06-05-2013 v1.4 Reset the old frame times
			old_df_rear = (old_df_rear + 1) % OLDFRAMECOUNT;

			ODFT[old_df_rear].soc = to_long_int_convertor(TSB[index].soc);
			
			unsigned char *fsec;
			fsec = malloc(3*sizeof(unsigned char));
			fsec[0] = TSB[index].fracsec[1];
			fsec[1] = TSB[index].fracsec[2];
			fsec[2] = TSB[index].fracsec[3];			
			ODFT[old_df_rear].fracsec = to_long_int_convertor1(fsec);
			free(fsec);

			TSB[index].first_data_frame = df;     /* Assign df to the 'first_data_frame' in the data frame linked list of TSB[index] */			
			Tsb_Count++;
			TSB[index].count = 1;
			TSB[index].num = 0;
			/* Now we need to store the pmu/pdc id in the pmupdc_id_list 
			that would be required while sorting */
			struct pmupdc_id_list *temp_pmuid;
			while(temp_cfg != NULL) {

				/* Create a node of the type 'pmupdc_id_list' and 
				copy the pmu/pde id from the cfg to it */

				struct pmupdc_id_list *pmuid = malloc(sizeof(struct pmupdc_id_list));
				pmuid->idcode = malloc(3);
				memset(pmuid->idcode,'\0',3);
				copy_cbyc((unsigned char *)pmuid->idcode,temp_cfg->idcode,2);
				pmuid->num_pmu = to_intconvertor(temp_cfg->num_pmu);	
				pmuid->nextid = NULL;
				TSB[index].num++;

				if(TSB[index].idlist == NULL) { /* Assign the pmuid to the idlist as it is the first id in the list */

					TSB[index].idlist = temp_pmuid = pmuid;

				} else {

					temp_pmuid->nextid = pmuid;
					temp_pmuid = pmuid;

				}			

				temp_cfg = temp_cfg->cfgnext;				
			} // while ends

			TSB[index].counter = 1;

			unsigned int tsb_soc = to_long_int_convertor(df->soc);
			unsigned int IDcode = to_intconvertor(df->idcode);
			unsigned char *tsb_fsec;
			tsb_fsec = malloc(3*sizeof(unsigned char));
			tsb_fsec[0] = df->fracsec[1];
			tsb_fsec[1] = df->fracsec[2];
			tsb_fsec[2] = df->fracsec[3];			
			unsigned int tsb_fracsec = to_long_int_convertor1(tsb_fsec); 

			//writeTimeToLog(2,IDcode,'d',tsb_soc,tsb_fracsec); // Checkpoint time before sorting
			free(tsb_fsec);
			unsigned int df_soc = to_long_int_convertor(df->soc);
//			unsigned char *fsec;
			fsec = malloc(3*sizeof(unsigned char));
			fsec[0] = df->fracsec[1];
			fsec[1] = df->fracsec[2];
			fsec[2] = df->fracsec[3];
			unsigned int df_fracsec = to_long_int_convertor1(fsec);
            free(fsec); // Added by gopal 20140830
			if(TSB[index].count == TSB[index].num) {

					int xx = index;			
			//		writeTimeToLog(2,IDcode,'d',df_soc,df_fracsec); // Checkpoint time before sorting
					intermediate_dispatch(xx);
			} else {

			//	writeTimeToLog(2,IDcode,'d',df_soc,df_fracsec); // Checkpoint time before sorting
			}	

			//printf("*ee Index %d front %d  rear %d, Tcount %d,%u,%u,%u\n",index,front,rear,Tsb_Count,IDcode,df_soc,df_fracsec);
			int k = 0;
			for (k=0;k<MAXTSB;k++) {

				if((k != index) && (TSB[k].first_data_frame != NULL) && (TSB[k].soc != NULL )) {

					TSB[k].counter++;
/*					df_soc = to_long_int_convertor(TSB[k].first_data_frame->soc);
					IDcode = to_intconvertor(TSB[k].first_data_frame->idcode);						
						
					fsec[0] = TSB[k].first_data_frame->fracsec[1];
					fsec[1] = TSB[k].first_data_frame->fracsec[2];
					fsec[2] = TSB[k].first_data_frame->fracsec[3];			
					unsigned int df_fracsec = to_long_int_convertor1(fsec); 

*/					//printf("ee Counter %d Index %d front %d  rear %d, Tcount %d ,%u,%u,%u\n",TSB[k].counter,k,front,rear,Tsb_Count,IDcode,df_soc,df_fracsec);					
					if(TSB[k].counter == MAXTSB) {

				//		writeTimeToLog(46,IDcode,tsb_soc,tsb_fracsec);
						//printf("ee Index %d front %d  rear %d, Tcount %d,%u,%u,%u\n",k,front,rear,Tsb_Count,IDcode,df_soc,df_fracsec );
						intermediate_dispatch(k);
						
					}
				}
			}

		} else { // 2 if else

		//	printf("Inside assign_df id = %d index %d front = %d,rear = %d\n",IDcode,index,front,rear);

			/* Traverse the data frames of TSB[index] and assign the df to 'dnext' of 
			the last element in the data frame LL.*/
			struct data_frame *temp_df,*check_df;

			/* Need to check if df with same idcode and soc is already assigned to 
			   the TSB[index] */
			check_df = TSB[index].first_data_frame;
			while(check_df != NULL) {

				if(!ncmp_cbyc(check_df->idcode,df->idcode,2)) {

					free_dataframe_object(df);
					printf("already assigned frame\n");
					return;					

				} else {

					check_df = check_df->dnext;

				}							
			}

			temp_df = TSB[index].first_data_frame;
			while(temp_df->dnext != NULL) {

				temp_df = temp_df->dnext;			

			}

			temp_df->dnext = df;
			
/*			unsigned int df_soc,df_fracsec;
			df_soc = to_long_int_convertor(df->soc);
			unsigned char *fsec;
			fsec = malloc(3*sizeof(unsigned char));
			fsec[0] = df->fracsec[1];
			fsec[1] = df->fracsec[2];
			fsec[2] = df->fracsec[3];
			df_fracsec = to_long_int_convertor1(fsec);
			unsigned int IDcode = to_intconvertor(df->idcode);		
*///			printf("*ff Index %d front %d  rear %d, Tcount %d,%u,%u,%u\n",index,front,rear,Tsb_Count,IDcode,df_soc,df_fracsec);
//			writeTimeToLog(4,IDcode,df_soc,df_fracsec);
			
			TSB[index].count++;

			// New Code by Kedar			
			if(TSB[index].count == TSB[index].num) {

				int xx = index;
				intermediate_dispatch(xx);
//				printf("ff Index %d front %d  rear %d, Tcount %d,%u,%u,%u\n",index,front,rear,Tsb_Count,IDcode,df_soc,df_fracsec );
//				writeTimeToLog(47,IDcode,df_soc,df_fracsec);
			}

		} // 2 if ends	

	} // 1 if ends
} 

void intermediate_dispatch(int xx) {

/*	unsigned int tsb_soc = to_long_int_convertor(TSB[xx].soc);
	unsigned char *tsb_fsec;
	tsb_fsec = malloc(3*sizeof(unsigned char));
	tsb_fsec[0] = TSB[xx].fracsec[1];
	tsb_fsec[1] = TSB[xx].fracsec[2];
	tsb_fsec[2] = TSB[xx].fracsec[3];			
	unsigned int tsb_fracsec = to_long_int_convertor1(tsb_fsec); 
	unsigned int IDcode = to_intconvertor(TSB[xx].first_data_frame->idcode);
*/	unsigned int yy = xx;

//	printf("*xx = %d front %d, rear %d TCount %d, %u,%u,%u\n",xx,front,rear,Tsb_Count,IDcode,tsb_soc,tsb_fracsec);	

	if(front < rear) {
			
		if(xx == rear) {

			dispatch(xx);
			rear= (rear - 1	)<0 ? rear-1+MAXTSB:rear -1; // one step backward
//			printf("gg Index %d front %d  rear %d, Tcount %d \n",xx,front,rear,Tsb_Count );			
		} else if(xx == front) {

			dispatch(front);
			front = (front + 1) % MAXTSB;
			
			//printf("hh Index %d front %d  rear %d, Tcount %d \n",yy,front,rear,Tsb_Count );

		} else if(xx <rear)  {	

			dispatch(xx);					
			unsigned int tsb_soc1;
			while (xx < rear ) {
			
				copy_cbyc (TSB[xx].soc,(unsigned char *)TSB[xx + 1].soc,4);
				copy_cbyc (TSB[xx].fracsec,(unsigned char *)TSB[xx + 1].fracsec,4);
	
				TSB[xx].count = TSB[xx + 1].count;
				TSB[xx].num = TSB[xx + 1].num;
				TSB[xx].counter = TSB[xx + 1].counter;				
				TSB[xx].idlist = TSB[xx + 1].idlist;
				TSB[xx].first_data_frame = TSB[xx + 1].first_data_frame;
				xx++;
			}

			memset(TSB[rear].soc,'\0',5);
			memset(TSB[rear].fracsec,'\0',5);
			TSB[rear].count = 0;		
			TSB[rear].num = 0;
			TSB[rear].counter = 0;				
			TSB[rear].first_data_frame = NULL;
			TSB[rear].idlist = NULL;

			rear= (rear - 1	)<0 ? rear-1+MAXTSB:rear -1; // one step backward			
//			printf("ii Index %d front %d  rear %d, Tcount %d \n",yy,front,rear,Tsb_Count );
		} 
	} else if (front > rear) {

		if(xx == rear) {

			dispatch(rear);
//			unsigned int tsb_soc = to_long_int_convertor(TSB[xx].soc);
			
			if(front - rear == MAXTSB)
			{
				rear = front;
				//printf("jj Index %d front %d  rear %d, Tcount %d \n",yy,front,rear,Tsb_Count );
			 } else {							

				rear = rear - 1;
			 	if(rear <0)
			 		rear = rear + MAXTSB;
				}
				//printf("kk Index %d front %d  rear %d, Tcount %d \n",yy,front,rear,Tsb_Count );
				
			} else if(xx == front) {

				dispatch(front);
				front = (front + 1) % MAXTSB;
			//	printf("ll Index %d front %d  rear %d, Tcount %d \n",yy,front,rear,Tsb_Count );

			} else if(xx > front) { // xx>front

				dispatch(xx);
				int rr = xx;

				while (rr != rear) {

					int h = (rr + 1) % MAXTSB;
			
					copy_cbyc (TSB[rr].soc,(unsigned char *)TSB[h].soc,4);
					copy_cbyc (TSB[rr].fracsec,(unsigned char *)TSB[h].fracsec,4);
			
					TSB[rr].count = TSB[h].count;
					TSB[rr].num = TSB[h].num;
					TSB[rr].counter = TSB[h].counter;				
					TSB[rr].idlist = TSB[h].idlist;
					TSB[rr].first_data_frame = TSB[h].first_data_frame;
					rr = (rr + 1) % MAXTSB;
				}
														
				memset(TSB[rear].soc,'\0',5);
				memset(TSB[rear].fracsec,'\0',5);
				TSB[rear].count = 0;		
				TSB[rear].num = 0;
				TSB[rear].counter = 0;
				TSB[rear].first_data_frame = NULL;
				TSB[rear].idlist = NULL;
				rear= (rear - 1	)<0 ? rear-1+MAXTSB:rear -1; // one step backward			
				
			//	printf("mm Index %d front %d  rear %d, Tcount %d \n",yy,front,rear,Tsb_Count );

		} else if(xx < front) {

			dispatch(xx);
			int rr = xx;
			while (rr != rear) {
					
					int h = (rr + 1) % MAXTSB;
					
					copy_cbyc (TSB[rr].soc,(unsigned char *)TSB[h].soc,4);
					copy_cbyc (TSB[rr].fracsec,(unsigned char *)TSB[h].fracsec,4);
					
					TSB[rr].count = TSB[h].count;
					TSB[rr].num = TSB[h].num;
					TSB[rr].counter = TSB[h].counter;				
					TSB[rr].idlist = TSB[h].idlist;
					TSB[rr].first_data_frame = TSB[h].first_data_frame;
					rr = (rr + 1) % MAXTSB;
				}
														
				memset(TSB[rear].soc,'\0',5);
				memset(TSB[rear].fracsec,'\0',5);
				TSB[rear].count = 0;		
				TSB[rear].num = 0;
				TSB[rear].counter = 0;
				TSB[rear].first_data_frame = NULL;
				TSB[rear].idlist = NULL;
				rear= (rear - 1	)<0 ? rear-1+MAXTSB:rear -1; // one step backward							
				
			//	printf("nn Index %d front %d  rear %d, Tcount %d \n",yy,front,rear,Tsb_Count );
		}
	} else {
			
		dispatch(front);
		//printf("oo Index %d front %d  rear %d, Tcount %d \n",yy,front,rear,Tsb_Count );
	}
}

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  dispatch():                                	     		*/
/* It dispatches the combined data frame to all the destination devices		*/
/* ----------------------------------------------------------------------------	*/

void dispatch(int index) {

	int size,flag = 0;
	Tsb_Count--;
	unsigned int tsb_soc = to_long_int_convertor(TSB[index].soc);
	struct data_frame *df = TSB[index].first_data_frame;
	unsigned int IDcode = to_intconvertor(df->idcode);
	adjustOldFrameCount(index);

	unsigned char *tsb_fsec;
	tsb_fsec = malloc(3*sizeof(unsigned char));
	tsb_fsec[0] = TSB[index].fracsec[1];
	tsb_fsec[1] = TSB[index].fracsec[2];
	tsb_fsec[2] = TSB[index].fracsec[3];			
	unsigned int tsb_fracsec = to_long_int_convertor1(tsb_fsec); 
	
//		writeTimeToLog(3,IDcode,tsb_soc,tsb_fracsec); // Checkpoint time before sorting
			
	sort_data_inside_TSB(index);

	dataframe = NULL;	 
	pthread_mutex_lock(&mutex_Upper_Layer_Details);
	struct Upper_Layer_Details *temp_pdc = ULfirst;

    if(flag == 0) {
        size = create_dataframe(index);	
	//writeTimeToLog(2,IDcode,'d',tsb_soc,tsb_fracsec); 	//Checkpoint time after creation of combined data frame				
        flag = 1;
    }
    // First send data to the DB server
    if((DB_pdc->UL_upper_pdc_cfgsent == 1) && (DB_pdc->UL_data_transmission_off == 0)) {			

        if(DB_pdc->config_change == 1) {
            dataframe[14] = 0x04;
            dataframe[15] = 0x00;
            //DB_pdc->config_change = 0; // Added by Gopal on 2014-08-31 to just send it once
        } else {
            dataframe[14] = 0x00;
            dataframe[15] = 0x00;
        }
        if (sendto(DB_pdc->sockfd,dataframe, size, 0,
                    (struct sockaddr *)&DB_pdc->pdc_addr,sizeof(DB_pdc->pdc_addr)) == -1)
            perror("sendto");
    }
    // Now handle the upper level PDCs
	while(temp_pdc != NULL ) {

		if((temp_pdc->UL_upper_pdc_cfgsent == 1) && (temp_pdc->UL_data_transmission_off == 0)) {			

			if(temp_pdc->config_change == 1) {

				dataframe[14] = 0x04;
				dataframe[15] = 0x00;

			} else {

				dataframe[14] = 0x00;
				dataframe[15] = 0x00;

			}

			if(temp_pdc->port == UDPPORT) {

				if (sendto(temp_pdc->sockfd,dataframe, size, 0,
						(struct sockaddr *)&temp_pdc->pdc_addr,sizeof(temp_pdc->pdc_addr)) == -1)
					perror("sendto");	

			} else if((temp_pdc->port == TCPPORT) && (temp_pdc->tcpup == 1)) {

				if(send(temp_pdc->sockfd,dataframe,size, 0)== -1) {
					perror("send");	
					printf("TCP connection closed\n");	
					temp_pdc->tcpup = 0;
					pthread_cancel(temp_pdc->thread_id);		
				}			
			}		
		}
		temp_pdc = temp_pdc->next;
	}
	pthread_mutex_unlock(&mutex_Upper_Layer_Details);
	free(tsb_fsec);
	if(dataframe != NULL) 	free(dataframe);

	clear_TSB(index);
//	printf("After\n");
}


void adjustOldFrameCount(int index) {

	int flag = 0;
	int i;

	unsigned char *tsb_fsec;
	unsigned int tsb_soc,tsb_fracsec;

	tsb_fsec = malloc(3*sizeof(unsigned char));
	tsb_fsec[0] = TSB[index].fracsec[1];
	tsb_fsec[1] = TSB[index].fracsec[2];
	tsb_fsec[2] = TSB[index].fracsec[3];
				
	tsb_soc = to_long_int_convertor((unsigned char *)TSB[index].soc);
	tsb_fracsec = to_long_int_convertor1(tsb_fsec);		
    
    free(tsb_fsec); // added by gopal on 20140829

/*	if(old_df_rear  >= old_df_front) {
		for(i = old_df_rear; i>= old_df_front; i--) {
			if((ODFT[i].soc == tsb_soc) && (ODFT[i].fracsec == tsb_fracsec)) {

				flag = 1;
				break;
					 
			} else {

				continue;
			} //if for soc ends
		}// for ends					
	} else {

		for(i = old_df_rear; i< old_df_front; i++) {

			if((ODFT[i].soc == tsb_soc) && (ODFT[i].fracsec == tsb_fracsec)) {

				flag = 1;
				break;
					 
			} else {

				continue;
			
			} //if for soc ends
		}// for ends								
	}
*/	

	for(i = 0; i< OLDFRAMECOUNT; i++) {
			
		if((ODFT[i].soc == tsb_soc) && (ODFT[i].fracsec == tsb_fracsec)) {

			flag = 1;
			break;				
		} 
	}

	if(flag)
	{
		int xx = i;
		ODFT[xx].soc = 0;
		ODFT[xx].fracsec = 0;			
		if(old_df_front < old_df_rear) {
				
			if(xx == old_df_rear) {
				
				old_df_rear= (old_df_rear - 1)%OLDFRAMECOUNT;
				if(old_df_rear <0)

					old_df_rear = old_df_rear + OLDFRAMECOUNT;//		

			} else if(xx == old_df_front) {
							
				old_df_front = (old_df_front + 1) % OLDFRAMECOUNT;

			} else if(xx < old_df_rear)  {	

				while (xx < old_df_rear) {

					ODFT[xx].soc = ODFT[xx + 1].soc;
					ODFT[xx].fracsec = ODFT[xx + 1].fracsec;
					xx++;
				}

				ODFT[old_df_rear].soc = 0;
				ODFT[old_df_rear].fracsec = 0;			

				old_df_rear = (old_df_rear - 1) % OLDFRAMECOUNT;		
				if(old_df_rear < 0)
					old_df_rear = old_df_rear + OLDFRAMECOUNT;
			}
		} else if (old_df_front > old_df_rear) {

			if(xx == old_df_rear) {

				if(old_df_front - old_df_rear == OLDFRAMECOUNT)
				{
								//front = rear;
					old_df_rear = old_df_front;
								
				 } else {							
					
					old_df_rear = old_df_rear - 1;
						if(old_df_rear <0)
							old_df_rear = old_df_rear + OLDFRAMECOUNT;							
				}

			} else if(xx == old_df_front) {
					
				old_df_front = (old_df_front + 1) % OLDFRAMECOUNT;

			} else  { // xx>front
							
				int rr = xx;
				while (rr != old_df_rear) {

					int h = (rr + 1) % OLDFRAMECOUNT;
					ODFT[rr].soc = ODFT[h].soc;
					ODFT[rr].fracsec = ODFT[h].fracsec;
					rr = (rr + 1) % OLDFRAMECOUNT;
				}
															
				old_df_rear = (old_df_rear - 1) % OLDFRAMECOUNT;
				if(old_df_rear < 0)
					old_df_rear = old_df_rear + OLDFRAMECOUNT;				
			} 

		} else {

			old_df_front = (old_df_front + 1)%OLDFRAMECOUNT;
			old_df_rear = old_df_front;
		}
	}else {

		printf("No match with ODFT\n");
	} 
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  sort_data_inside_TSB():                          	     		*/
/* This function sorts the data frames in the TSB[index] in the order of the 	*/
/* Idcodes present in the 'struct pmupdc_id_list list' of the TSB[index]	*/
/* ----------------------------------------------------------------------------	*/

void sort_data_inside_TSB(int index) { 

	struct pmupdc_id_list *temp_list;
	struct data_frame *prev_df,*curr_df,*sorted_df,*r_df,*s_df,*last_df,*p_df;
	int match = 0;
	unsigned int id_check;

	/* Pointer track_df will hold the address of the last sorted data_frame object. 
	Thus we assign to the 'track_df->dnext ' the next sorted data_frame  object and so on */
	temp_list = TSB[index].idlist; /* Starting ID required for sorting */
	last_df = TSB[index].first_data_frame;
	p_df = TSB[index].first_data_frame;

	curr_df = last_df;
	sorted_df = prev_df = NULL;

	while(temp_list != NULL) { // 1 while

		match = 0;
		while(curr_df != NULL) { // 2. Traverse the pmu id in TSB and sort 

			if(!ncmp_cbyc(curr_df->idcode,(unsigned char *)temp_list->idcode,2)){

				match = 1;
				break;

			} else {

				prev_df = curr_df; 
				curr_df = curr_df->dnext;						

			}

		} // 2 while ends	

		if (match == 0) {	// id whose data frame did not arrive or No match!

			curr_df = generate_dummy_dataframe(temp_list->idcode);			
			
			//Copy SOC						
			copy_cbyc (curr_df->soc,(unsigned char *)TSB[index].soc,4);
			curr_df->soc[4] = '\0';

			//Copy FRACSEC						
			copy_cbyc (curr_df->fracsec,(unsigned char *)TSB[index].fracsec,4);
			curr_df->fracsec[4] = '\0';

			unsigned int tsb_soc = to_long_int_convertor(TSB[index].soc);
			unsigned int IDcode = to_intconvertor(curr_df->idcode);
			unsigned char *tsb_fsec;
			tsb_fsec = malloc(3*sizeof(unsigned char));
			tsb_fsec[0] = TSB[index].fracsec[1];
			tsb_fsec[1] = TSB[index].fracsec[2];
			tsb_fsec[2] = TSB[index].fracsec[3];			
			unsigned int tsb_fracsec = to_long_int_convertor1(tsb_fsec); 

			free(tsb_fsec);
			printf("generate_dummy_dataframe for %u soc %u fsec %u\n",IDcode,tsb_soc,tsb_fracsec);
		
		}

		if(prev_df == NULL) {

			r_df = curr_df;
			s_df = curr_df->dnext;
				
			if(sorted_df == NULL) {

				sorted_df = r_df;
				TSB[index].first_data_frame = sorted_df;
					
			} else {

				sorted_df->dnext = r_df;
				sorted_df = r_df;
			}
			sorted_df->dnext = s_df ;
			curr_df = last_df = s_df;

		} else {

			if(sorted_df == NULL) {

				r_df = curr_df;
				s_df = r_df->dnext;
				prev_df->dnext = s_df;
				sorted_df = r_df;
				TSB[index].first_data_frame = sorted_df;
				sorted_df->dnext = last_df ;
				curr_df = last_df;
				prev_df = NULL;

			} else {//if(sorted_df != NULL)

				r_df = curr_df;
				s_df = r_df->dnext;
				prev_df->dnext = s_df;
				sorted_df->dnext = r_df;
				sorted_df = r_df;
				sorted_df->dnext = last_df ;
				curr_df = last_df;
				prev_df = NULL;
			}					
		}                                

		temp_list = temp_list->nextid;  //go for next ID

	} // 1. while ends
} 


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  clear_TSB():                                	     		*/
/* It clears TSB[index] and frees all data frame objects after the data frames  */
/* in TSB[index] have been dispatched to destination device			*/
/* ----------------------------------------------------------------------------	*/

void clear_TSB(int index) { // 

	memset(TSB[index].soc,'\0',5);
	memset(TSB[index].fracsec,'\0',5);

	struct pmupdc_id_list *t_list,*r_list;
	t_list = TSB[index].idlist;
	TSB[index].count = 0;

	while(t_list != NULL) {

		r_list = t_list->nextid;
		free(t_list->idcode);
		free(t_list);
		t_list = r_list;			
	}

	struct data_frame *t,*r;
	t = TSB[index].first_data_frame;

	while(t != NULL) {

		r = t->dnext;
		free_dataframe_object(t);
		t = r;			
	}

	TSB[index].first_data_frame = NULL;
	TSB[index].idlist = NULL;
	TSB[index].num = 0;
	TSB[index].counter = 0;
}

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  create_dataframe():                              	     		*/
/* It creates the IEEEC37.118 Standard based combined data frame from the data	*/
/* frames received from all the source devices to be sent to the destination	*/
/* devices.														*/
/* ----------------------------------------------------------------------------	*/

int create_dataframe(int index) {

	int total_frame_size = 0;
	unsigned char temp[3];
	struct data_frame *temp_df;
	unsigned int fsize;
	uint16_t chk;

	temp_df = TSB[index].first_data_frame;

	while(temp_df != NULL) {

		fsize = to_intconvertor(temp_df->framesize);
		total_frame_size = total_frame_size + fsize;
		total_frame_size -= 16; // To skip SYNC + FRAMESIZE + idcode + soc + fracsec + checksum
		temp_df = temp_df->dnext;
	}

	// Calculate the single combined data frame size
	total_frame_size = total_frame_size + 16;	/* 16 is for comman fields SYNC + FRAMESIZE + idcode + soc + fracsec + checksum */

	// Allocate memory for complete single data frame
	dataframe = malloc((total_frame_size + 1)*sizeof(char)); 

	if(!dataframe) {

		printf("No enough memory for data frame!\n");
	}	
	dataframe[total_frame_size] = '\0';	

	// Start the data frame creation 
	int z = 0;
	byte_by_byte_copy(dataframe,DATASYNC,z,2); // SYNC Word for data frame
	z += 2;

	memset(temp,'\0',3);
	int_to_ascii_convertor(total_frame_size,temp);
	byte_by_byte_copy(dataframe,temp,z,2); // FRAME SIZE
	z += 2;

	memset(temp,'\0',3);
	int_to_ascii_convertor(PDC_IDCODE,temp);
	byte_by_byte_copy(dataframe,temp,z,2); // PDC ID
	z += 2;

	byte_by_byte_copy(dataframe,(unsigned char *)TSB[index].soc,z,4); //SOC
	z += 4;
	byte_by_byte_copy(dataframe,(unsigned char *)TSB[index].fracsec,z,4); //FRACSEC + Time Quality?
	z += 4;

	temp_df = TSB[index].first_data_frame;
	while(temp_df != NULL) { // 1

		int j = 0;
		while(j < temp_df->num_pmu) { // 2

			//Copy STAT Word
			byte_by_byte_copy(dataframe,temp_df->dpmu[j]->stat,z,2);
			z += 2;

			int i = 0;

			//Copy Phasors
			if(temp_df->dpmu[j]->phnmr != 0) {

				if(temp_df->dpmu[j]->fmt->phasor == '1') {

					while(i < temp_df->dpmu[j]->phnmr) {

						byte_by_byte_copy(dataframe,temp_df->dpmu[j]->phasors[i],z,8); // Phasors
						z += 8;
						i++;
					}

				} else {

					while(i < temp_df->dpmu[j]->phnmr) {

						byte_by_byte_copy(dataframe,temp_df->dpmu[j]->phasors[i],z,4); // Phasors
						z += 4;
						i++;
					}
				}			
			}

			//Copy FREQ
			if(temp_df->dpmu[j]->fmt->freq == '1') {

				byte_by_byte_copy(dataframe,temp_df->dpmu[j]->freq,z,4); // FREQ
				z += 4;
				byte_by_byte_copy(dataframe,temp_df->dpmu[j]->dfreq,z,4); // FREQ
				z += 4;

			} else {

				byte_by_byte_copy(dataframe,temp_df->dpmu[j]->freq,z,2); // FREQ
				z += 2;
				byte_by_byte_copy(dataframe,temp_df->dpmu[j]->dfreq,z,2); // FREQ
				z += 2;

			}						

			// Copy Analogs
			if(temp_df->dpmu[j]->annmr != 0) {

				if(temp_df->dpmu[j]->fmt->analog == '1') {

					for(i = 0; i<temp_df->dpmu[j]->annmr; i++){					

						byte_by_byte_copy(dataframe,temp_df->dpmu[j]->analog[i],z,4); // ANALOGS
						z += 4;
					}

				} else {

					for(i = 0; i<temp_df->dpmu[j]->annmr; i++){					

						byte_by_byte_copy(dataframe,temp_df->dpmu[j]->analog[i],z,2); // ANALOGS
						z += 2;
					}
				}						
			}

			i = 0;

			//Copy DIGITAL
			if(temp_df->dpmu[j]->dgnmr != 0) {

				while(i < temp_df->dpmu[j]->dgnmr) {

					byte_by_byte_copy(dataframe,temp_df->dpmu[j]->digital[i],z,2); // DIGITAL
					z += 2;
					i++;
				}			
			}
			j++;			
		} // 2 while

		temp_df = temp_df->dnext;	
	} // 1 while
	
	// Attach a checksum
	chk = compute_CRC(dataframe,z);
	dataframe[z++] = (chk >> 8) & ~(~0<<8);  /* CHKSUM high byte; */
	dataframe[z++] = (chk ) & ~(~0<<8);   	/* CHKSUM low byte;  */

	// For iPDC configuration changed
	old_fsize = z;
	
	return z;	
} 


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  create_cfgframe():                                	     		*/
/* It creates the IEEEC37.118 Standard based combined configuration frame from  */
/* the configuration frames received from all the source devices to be sent to  */
/* destination devices												*/
/* ----------------------------------------------------------------------------	*/

int create_cfgframe() { 

	struct cfg_frame *temp_cfg;
	int total_frame_size = 0,count = 0;
	unsigned char datarate[2],soc[4],fracsec[4]; // hard coded
	int total_num_pmu = 0;
	unsigned char time_base[4];
	unsigned int fsize,num_pmu,phnmr,dgnmr,annmr;
	unsigned int data_rate,temp_data_rate;
	unsigned long int sec,frac = 0,temp_tb,tb;
	uint16_t chk;

	sec = (long int)time (NULL);
	long_int_to_ascii_convertor(sec,soc);
	long_int_to_ascii_convertor(frac,fracsec);

	temp_cfg = cfgfirst;

	while(temp_cfg != NULL) {

		if(count == 0) { // Copy the soc,fracsec,timebase from the first CFG to the combined CFG

			//SEPARATE TIMBASE	
			tb = to_long_int_convertor(temp_cfg->time_base);
			copy_cbyc (time_base,temp_cfg->time_base,4);

			data_rate = to_intconvertor(temp_cfg->data_rate);
			copy_cbyc (datarate,temp_cfg->data_rate,2);

			fsize = to_intconvertor(temp_cfg->framesize);
			total_frame_size += fsize;
			count++;	// count used to count num of cfg
			
			num_pmu = to_intconvertor(temp_cfg->num_pmu);	
			total_num_pmu += num_pmu;	
			temp_cfg = temp_cfg->cfgnext;

		} else {

			fsize = to_intconvertor(temp_cfg->framesize);
			total_frame_size += fsize;
			total_frame_size -= 24;

			// take the Lowest Timebase
			temp_tb = to_long_int_convertor(temp_cfg->time_base);
			if(temp_tb < tb) {

				copy_cbyc (time_base,temp_cfg->time_base,4);
				tb = temp_tb;

			}

			// take the highest data rate
			temp_data_rate = to_intconvertor(temp_cfg->data_rate);
			if(temp_data_rate > data_rate) {

				copy_cbyc (datarate,temp_cfg->data_rate,2);
				data_rate = temp_data_rate;				
			}	
			count++;	// count used to count num of cfg
			
			num_pmu = to_intconvertor(temp_cfg->num_pmu);	
			total_num_pmu += num_pmu;	
			temp_cfg = temp_cfg->cfgnext;
		}

	} // While ENDS	

	cfgframe = malloc((total_frame_size + 1)*sizeof(unsigned char)); // Allocate memory for data frame
	cfgframe[total_frame_size] = '\0';

	// Start the Combined CFG frame creation 
	int z = 0;
	byte_by_byte_copy(cfgframe,CFGSYNC,z,2); // SYNC
	z += 2;

	unsigned char temp[3];
	memset(temp,'\0',3);
	int_to_ascii_convertor(total_frame_size,temp);
	byte_by_byte_copy(cfgframe,temp,z,2); // FRAME SIZE
	z += 2;

	unsigned char tmp[2];
	tmp[0]= cfgframe[2];
	tmp[1]= cfgframe[3];
	int newl;
	newl = to_intconvertor(tmp);
	if (newl == 0) {
		return 0;
	}
	printf("CFG Frame Len %d.\n",newl);

	memset(temp,'\0',3);
	int_to_ascii_convertor(PDC_IDCODE,temp);
	byte_by_byte_copy(cfgframe,temp,z,2); // PDC ID
	z += 2;

	byte_by_byte_copy(cfgframe,soc,z,4); //SOC
	z += 4;
	byte_by_byte_copy(cfgframe,fracsec,z,4); //FRACSEC
	z += 4;
	byte_by_byte_copy(cfgframe,time_base,z,4); //TIMEBASE
	z += 4;

	memset(temp,'\0',3);
	int_to_ascii_convertor(total_num_pmu,temp);
	byte_by_byte_copy(cfgframe,temp,z,2); // No of PMU
	z += 2;

	int i,j; 
	temp_cfg = cfgfirst;

	while(temp_cfg != NULL) { // 1

		num_pmu = to_intconvertor(temp_cfg->num_pmu);	
		j = 0;

		while (j < num_pmu) { //2

			byte_by_byte_copy(cfgframe,temp_cfg->pmu[j]->stn,z,16); // STN
			z += 16;

			byte_by_byte_copy(cfgframe,temp_cfg->pmu[j]->idcode,z,2); // IDCODE
			z += 2;

			byte_by_byte_copy(cfgframe,temp_cfg->pmu[j]->data_format,z,2); // FORMAT
			z += 2;

			byte_by_byte_copy(cfgframe,temp_cfg->pmu[j]->phnmr,z,2); // PHNMR
			z += 2;

			byte_by_byte_copy(cfgframe,temp_cfg->pmu[j]->annmr,z,2); // ANNMR
			z += 2;

			byte_by_byte_copy(cfgframe,temp_cfg->pmu[j]->dgnmr,z,2); // DGNMR
			z += 2;	

			phnmr = to_intconvertor(temp_cfg->pmu[j]->phnmr);
			annmr = to_intconvertor(temp_cfg->pmu[j]->annmr);
			dgnmr = to_intconvertor(temp_cfg->pmu[j]->dgnmr);

			// Copy Phasor Names
			if(phnmr != 0){

				for(i = 0; i<phnmr;i++) {

					byte_by_byte_copy(cfgframe,temp_cfg->pmu[j]->cnext->phnames[i],z,16); // Phasor Names
					z += 16;						
				}
			}

			// Copy Analog Names
			if(annmr != 0){

				for(i = 0; i<annmr;i++) {

					byte_by_byte_copy(cfgframe,temp_cfg->pmu[j]->cnext->angnames[i],z,16); // Analog Names
					z += 16;											
				}
			}

			// Copy Digital Names
			if(dgnmr != 0) {

				struct dgnames *temp_dgname = temp_cfg->pmu[j]->cnext->first;
				while (temp_dgname != NULL) {

					for(i = 0;i<16;i++) {

						byte_by_byte_copy(cfgframe,temp_dgname->dgn[i],z,16); // Digital Names
						z += 16; 				

					} // Copy 16 channel names of digital word 	

					temp_dgname = temp_dgname->dg_next;
				} // Go to next Digital word

			}

			// PHUNIT
			if(phnmr != 0){

				for (i = 0; i<phnmr;i++) {

					byte_by_byte_copy(cfgframe,temp_cfg->pmu[j]->phunit[i],z,4); // PHUNIT
					z += 4; 									
				}
			}	

			// ANUNIT
			if(annmr != 0){

				for (i = 0; i<annmr;i++) {

					byte_by_byte_copy(cfgframe,temp_cfg->pmu[j]->anunit[i],z,4); // ANUNIT
					z += 4; 									

				}
			}	

			// DGUNIT
			if(dgnmr != 0){

				for (i = 0; i<dgnmr;i++) {

					byte_by_byte_copy(cfgframe,temp_cfg->pmu[j]->dgunit[i],z,4); // DGUNIT
					z += 4; 									
				}	

			}		

			byte_by_byte_copy(cfgframe,temp_cfg->pmu[j]->fnom,z,2); // FNOM
			z += 2; 									

			byte_by_byte_copy(cfgframe,temp_cfg->pmu[j]->cfg_cnt,z,2); // CFGCNT
			z += 2; 									

			j++; // index for pmu_num

		} // while 2

		temp_cfg = temp_cfg->cfgnext;	// Take next CFG	

	} // while 1


	byte_by_byte_copy(cfgframe,datarate,z,2); // DATA RATE
	z += 2; 									

	chk = compute_CRC(cfgframe,z);
	cfgframe[z++] = (chk >> 8) & ~(~0<<8);  /* CHKSUM high byte; */
	cfgframe[z++] = (chk ) & ~(~0<<8);   	/* CHKSUM low byte;  */
	return z;	
} 


/* ----------------------------------------------------------------------------	*/
/* FUNCTIO unsigned char* generate_dummy_dataframe(int id):					*/
/* ----------------------------------------------------------------------------	*/

struct data_frame* generate_dummy_dataframe(unsigned char *idcode)
{
	int match=0,i,j=0, fsize=0;
	unsigned char tempI[2];
	unsigned int num_pmu,phnmr,annmr,dgnmr; 
	struct cfg_frame *temp_cfg;
	struct data_frame *df;

	unsigned char temp2[2] = {0, 0};
	unsigned char temp4[4] = {0, 0, 0, 0};
	unsigned char temp8[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	//printf("Inside generate_dummy_dataframe\n");

	pthread_mutex_lock(&mutex_cfg);

	temp_cfg = cfgfirst;

	// Check for the data frame IDCODE in Configuration Frame?
	while(temp_cfg != NULL){

		if(!ncmp_cbyc((unsigned char *) idcode,temp_cfg->idcode,2)) {

			match = 1;
			break;	

		} else {

			temp_cfg = temp_cfg->cfgnext;
		}
	}
	pthread_mutex_unlock(&mutex_cfg);

	// If idcode matches with cfg idcode
	if(match){

		//Allocate memory for data frame 
		df = malloc(sizeof(struct data_frame));
		if(!df) {

			printf("Not enough memory for dummy df\n");
			exit(1);
		}
		df->dnext = NULL;

		// Allocate memory for df->framesize
		df->framesize = malloc(3*sizeof(unsigned char));
		if(!df->framesize) {

			printf("Not enough memory df->idcode\n");
			exit(1);
		}

		// Allocate memory for df->idcode
		df->idcode = malloc(3*sizeof(unsigned char));
		if(!df->idcode) {

			printf("Not enough memory df->idcode\n");
			exit(1);
		}

		// Allocate memory for df->soc
		df->soc = malloc(5*sizeof(unsigned char));
		if(!df->soc) {

			printf("Not enough memory df->soc\n");
			exit(1);
		}

		// Allocate memory for df->fracsec
		df->fracsec = malloc(5*sizeof(unsigned char));
		if(!df->fracsec) {

			printf("Not enough memory df->fracsec\n");
			exit(1);
		}

		// Allocate Memeory For Each PMU
		num_pmu = to_intconvertor(temp_cfg->num_pmu);

		/* For each PMU data */
		df->dpmu = malloc(num_pmu* sizeof(struct data_for_each_pmu *));

		if(!df->dpmu) {

			printf("Not enough memory df->dpmu[][]\n");
			exit(1);
		}

		for (i = 0; i < num_pmu; i++) {

			df->dpmu[i] = malloc(sizeof(struct data_for_each_pmu));
		}

		/* Now start separating the data from data frame */
		/* Copy Framesize */
		copy_cbyc (df->framesize,temp2,2);
		df->framesize[2] = '\0';

		//Copy IDCODE
		/*int id = atoi(idcode);
		tempI[0] = id>>8;
		tempI[1] = id;*/
		copy_cbyc (df->idcode,idcode,2);	
		df->idcode[2] = '\0';

		//Copy SOC
		copy_cbyc (df->soc,temp4,4);
		df->soc[4] = '\0';

		//Copy FRACSEC
		copy_cbyc (df->fracsec,temp4,4);
		df->fracsec[4] = '\0';
		fsize += 16;	// For SYNC + FRAMESIZE + idcode + soc + fracsec + checksum

		//Copy NUM PMU
		df->num_pmu = num_pmu;

		// Separate the data for each PMU	    	
		while(j<num_pmu) {		  					

			// Extract PHNMR, DGNMR, ANNMR values
			phnmr = to_intconvertor(temp_cfg->pmu[j]->phnmr);	
			annmr = to_intconvertor(temp_cfg->pmu[j]->annmr);
			dgnmr = to_intconvertor(temp_cfg->pmu[j]->dgnmr);

			//Allocate memory for stat, Phasors, Analogs,Digitals and Phasors and Frequencies
			/* Memory Allocation Begins */

			// Allocate memory for stat
			df->dpmu[j]->stat = malloc(3*sizeof(unsigned char));

			if(!df->dpmu[j]->stat) {

				printf("Not enough memory for df->dpmu[j]->stat\n");
			}

			// Allocate memory for Phasor
			df->dpmu[j]->phasors = malloc(phnmr*sizeof(unsigned char *));

			if(!df->dpmu[j]->phasors) {

				printf("Not enough memory df->dpmu[j]->phasors[][]\n");
				exit(1);
			}

			if(temp_cfg->pmu[j]->fmt->phasor == '1') {

				for (i = 0; i < phnmr; i++)

					df->dpmu[j]->phasors[i] = malloc(9*sizeof(unsigned char));
			} else {

				for (i = 0; i < phnmr; i++) 

					df->dpmu[j]->phasors[i] = malloc(5*sizeof(unsigned char));
			}

			/* Allocate memory for Analogs */
			df->dpmu[j]->analog = malloc(annmr*sizeof(unsigned char *));

			if(!df->dpmu[j]->analog) {

				printf("Not enough memory df->dpmu[j]->analog[][]\n");
				exit(1);
			}

			if(temp_cfg->pmu[j]->fmt->analog == '1') {

				for (i = 0; i < annmr; i++) 

					df->dpmu[j]->analog[i] = malloc(9*sizeof(unsigned char));
			} else {

				for (i = 0; i < annmr; i++) 

					df->dpmu[j]->analog[i] = malloc(5*sizeof(unsigned char));
			}

			/* Allocate memory for Frequency & DFREQ */
			if(temp_cfg->pmu[j]->fmt->freq == '1') {

				df->dpmu[j]->freq = malloc(5*sizeof(unsigned char));
				df->dpmu[j]->dfreq = malloc(5*sizeof(unsigned char));

			} else {

				df->dpmu[j]->freq = malloc(3*sizeof(unsigned char));
				df->dpmu[j]->dfreq = malloc(3*sizeof(unsigned char));
			}

			/* Allocate memory for Digital */
			df->dpmu[j]->digital = malloc(dgnmr* sizeof(unsigned char*));

			if(!df->dpmu[j]->digital) {

				printf("Not enough memory df->dpmu[j]->digital[][]\n");
				exit(1);
			}

			for (i = 0; i < dgnmr; i++) {

				df->dpmu[j]->digital[i] = malloc(3*sizeof(unsigned char));
			}
			/* Memory Allocation Ends here */		

			// Copy the PMU data fields
			tempI[0] = 0x82; //changed by Kedar on 4-7-2013
			tempI[1] = 0x00;
			copy_cbyc (df->dpmu[j]->stat,tempI,2);
			df->dpmu[j]->stat[2] = '\0';
			fsize += 2;

			// Copy Format Word
			df->dpmu[j]->fmt = malloc(sizeof(struct format));
			df->dpmu[j]->fmt->freq = temp_cfg->pmu[j]->fmt->freq;
			df->dpmu[j]->fmt->analog = temp_cfg->pmu[j]->fmt->analog;
			df->dpmu[j]->fmt->phasor = temp_cfg->pmu[j]->fmt->phasor;
			df->dpmu[j]->fmt->polar =  temp_cfg->pmu[j]->fmt->polar;

			// Copy number of phasors, analogs, and digitals	
			df->dpmu[j]->phnmr = phnmr;
			df->dpmu[j]->annmr = annmr;			
			df->dpmu[j]->dgnmr = dgnmr;

			//Phasors 	
			if(temp_cfg->pmu[j]->fmt->phasor == '1') {

				for(i=0;i<phnmr;i++){	

					copy_cbyc (df->dpmu[j]->phasors[i],temp8,8);
					df->dpmu[j]->phasors[i][8] = '\0';
					fsize += 8;
				}
			} else {

				for(i=0;i<phnmr;i++){					

					copy_cbyc (df->dpmu[j]->phasors[i],temp4,4);
					df->dpmu[j]->phasors[i][4] = '\0';
					fsize += 4;
				}
			}						

			/* For Freq */ 	
			if(temp_cfg->pmu[j]->fmt->freq == '1') {

				copy_cbyc (df->dpmu[j]->freq,temp4,4);
				df->dpmu[j]->freq[4] = '\0';
				fsize += 4;

				copy_cbyc (df->dpmu[j]->dfreq,temp4,4);
				df->dpmu[j]->dfreq[4] = '\0';
				fsize += 4;

			} else {

				copy_cbyc (df->dpmu[j]->freq,temp2,2);
				df->dpmu[j]->freq[2] = '\0';
				fsize += 2;

				copy_cbyc (df->dpmu[j]->dfreq,temp2,2);
				df->dpmu[j]->dfreq[2] = '\0';
				fsize += 2;
			}						

			/* For Analogs */
			if(temp_cfg->pmu[j]->fmt->analog == '1') {

				for(i = 0; i<annmr; i++){					

					copy_cbyc (df->dpmu[j]->analog[i],temp4,4);
					df->dpmu[j]->analog[i][4] = '\0';
					fsize += 4;
				}
			} else {
				for(i = 0; i<annmr; i++){					

					copy_cbyc (df->dpmu[j]->analog[i],temp2,2);
					df->dpmu[j]->analog[i][2] = '\0';
					fsize += 2;
				}
			}						

			/* For Digital */
			for(i = 0; i<dgnmr; i++) {

				copy_cbyc (df->dpmu[j]->digital[i],temp2,2);
				df->dpmu[j]->digital[i][2] = '\0';
				fsize += 2;
			}

			j++;
		} //While ends for sub PMUs

		tempI[0] = fsize>>8;
		tempI[1] = fsize;
		copy_cbyc (df->framesize,tempI,2);
		df->framesize[2] = '\0';

		return df;
	}
}
/**************************************** End of File *******************************************************/
