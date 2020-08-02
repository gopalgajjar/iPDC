/* ----------------------------------------------------------------------------- 
 * parser.c
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


/* ---------------------------------------------------------------------------- */
/*                       Functions defined in parser.c          	    			*/
/* -----------------------------------------------------------------------------*/

/*                 1. void cfgparser(char [])           	    	      		*/
/*                 2. int  remove_old_cfg(char[])              	    	      	*/
/*                 3. void cfginsert(struct cfg_frame *)       	    	      	*/
/*                 4. void dataparser(unsigned char data[])					*/
/*                 5. int  check_statword(char stat[])               	      	*/
/*                 6. unsigned int to_intconvertor(unsigned char array[])  */
/*                 7. unsigned int to_intconvertor1(unsigned char array[]) */ 
/*                 8. unsigned long int to_long_int_convertor(unsigned char array[]) */
/*                 9. unsigned long int to_long_int_convertor1(unsigned char array[]) */
/*                10. float decode_ieee_single(const void *v) */
/*                11. void copy_cbyc(unsigned char dst[],unsigned char *s,int size) */
/*                12. int ncmp_cbyc(unsigned char dst[],unsigned char src[],int size) */
/*                13. void dataCollectInBuffer(char *cmd, struct BUFFER *buffer)  */
/*                14. void* dataWriteInFile() */
/*                15. void initfilestreams()  */
/*                16. void closefilestreams() */
/*                17. void trimSpaces(char *TO, char *FROM) */

/* ---------------------------------------------------------------------------- */


#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h> 
#include  <pthread.h>
#include  <math.h>
#include <float.h>
#include <assert.h>
// #include  <mysql.h>
#include  <errno.h>
#include  "parser.h"
#include  "global.h" 
#include  "dallocate.h" 
#include  "connections.h"

// const int PI = 3.14;

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  cfgparser():                                	     		*/
/* It creates configuration objects for the received configuration frames.	*/
/* Configuration frame is also written in the file `cfg.bin`.			*/
/* If the object is already present, it will replace in cfg_frame LL and	*/
/* also in the file `cfg.bin` by calling remove_old_cfg().			*/
/* ----------------------------------------------------------------------------	*/

void cfgparser(unsigned char st[]){ 

    FILE *fp;
    unsigned char *s;
    int match = 0,dgchannels,i,j, phtype, antype;
    struct cfg_frame *cfg;
    struct channel_names *cn;
    unsigned long int l_phunit,l_anunit;

    unsigned char *sync,*frame_size,*idcode_h,*soc,*fracsec,*time_base,*num_pmu,*stn,*idcode_l,*format,*phnmr,*annmr,*dgnmr;
    unsigned char *fnom,*cfg_cnt,*data_rate,*buf;

    sync = malloc(3*sizeof(unsigned char));
    frame_size = malloc(3*sizeof(unsigned char));
    idcode_h= malloc(3*sizeof(unsigned char));
    soc = malloc(5*sizeof(unsigned char));
    fracsec = malloc(5*sizeof(unsigned char));
    time_base = malloc(5*sizeof(unsigned char));
    num_pmu = malloc(3*sizeof(unsigned char));
    stn = malloc(17*sizeof(unsigned char));
    idcode_l = malloc(3*sizeof(unsigned char));
    format = malloc(5*sizeof(unsigned char));
    phnmr = malloc(3*sizeof(unsigned char));
    annmr = malloc(3*sizeof(unsigned char));
    dgnmr = malloc(3*sizeof(unsigned char));

    fnom = malloc(3*sizeof(unsigned char));
    cfg_cnt = malloc(3*sizeof(unsigned char));
    data_rate = malloc(3*sizeof(unsigned char));
    buf = malloc(9*sizeof(unsigned char));

    memset(sync,'\0',3);
    memset(frame_size,'\0',3);
    memset(idcode_h,'\0',3);
    memset(soc,'\0',3);
    memset(fracsec,'\0',5);
    memset(time_base,'\0',5);
    memset(num_pmu,'\0',3);
    memset(stn,'\0',17);
    memset(idcode_l,'\0',3);
    memset(format,'\0',3);
    memset(phnmr,'\0',3);
    memset(annmr,'\0',3);
    memset(dgnmr,'\0',3);

    memset(fnom,'\0',3);
    memset(cfg_cnt ,'\0',3);
    memset(data_rate,'\0',3);
    memset(buf,'\0',9);

    /******************** PARSING BEGINGS *******************/

    pthread_mutex_lock(&mutex_file);

    fp = fopen("cfg.bin","ab"); //Store configuration in a file     

    if (fp == NULL)
        printf("File doesn't exist\n");

    cfg = malloc(sizeof(struct cfg_frame));
    if(!cfg) {
        printf("No enough memory for cfg\n");
    }

    printf("Inside cfgparser()\n");
    s = st;

    //Copy sync word to file
    copy_cbyc (sync,(unsigned char *)s,2);
    sync[2] = '\0';
    s = s + 2;

    // Separate the FRAME SIZE
    copy_cbyc (frame_size,(unsigned char *)s,2);
    frame_size[2]='\0';
    cfg->framesize = to_intconvertor(frame_size);
//    printf("FRAME SIZE %d\n",cfg->framesize);
    s = s + 2;

    size_t result;

    result = fwrite(st, sizeof(unsigned char),cfg->framesize, fp);
//    printf("No of bytes written %ld, into the cfg file.\n",(long int)result);

    unsigned long fileLen;
    fseek(fp, 0, SEEK_END);
    fileLen = ftell(fp);
    fseek(fp, 0, SEEK_SET);
//    printf("FILE LENGTH %ld\n",fileLen);

    //SEPARATE IDCODE
    copy_cbyc (idcode_h,(unsigned char *)s,2);
    idcode_h[2] = '\0';
    cfg->idcode = to_intconvertor(idcode_h);
    //printf("ID Code %d\n",cfg->idcode);
    s = s + 2;

    //SEPARATE SOC	
    copy_cbyc (soc,(unsigned char *)s,4);
    soc[4] = '\0';
    cfg->soc = to_long_int_convertor(soc);	 			
//    printf("SOC %ld\n",cfg->soc);
    s =s + 4;

    //SEPARATE FRACSEC	
    copy_cbyc (fracsec,(unsigned char *)s,4);
    fracsec[4] = '\0';

    unsigned char fsec[3];
    fsec[0] = fracsec[1];
    fsec[1] = fracsec[2];
    fsec[2] = fracsec[3];
    cfg->fracsec = to_long_int_convertor1(fsec);
//    printf("FracSec %ld\n",cfg->fracsec);
    s = s + 4;

    //SEPARATE TIMEBASE
    copy_cbyc (time_base,(unsigned char *)s,4);
    time_base[4]='\0';
    cfg->time_base = to_long_int_convertor(time_base);			
//    printf("Time Base %ld\n",cfg->time_base);
    s = s + 4;

    //SEPARATE PMU NUM
    copy_cbyc (num_pmu,(unsigned char *)s,2);
    num_pmu[2] = '\0';	
    cfg->num_pmu = to_intconvertor(num_pmu);		
    printf("Number of PMU's %d\n",cfg->num_pmu);
    s = s + 2;

    // Allocate Memeory For Each PMU
    cfg->pmu = malloc(cfg->num_pmu* sizeof(struct for_each_pmu *));
    if(!cfg->pmu) {
        printf("Not enough memory pmu[][]\n");
        exit(1);
    }

    for (i = 0; i < cfg->num_pmu; i++) {
        cfg->pmu[i] = malloc(sizeof(struct for_each_pmu));
    }

    j = 0;

    ///WHILE EACH PMU IS HANDLED
    while(j<cfg->num_pmu) {

        //SEPARATE STATION NAME
        memset(cfg->pmu[j]->stn,'\0',17);
        copy_cbyc (cfg->pmu[j]->stn,(unsigned char *)s,16);
        cfg->pmu[j]->stn[16] = '\0';

//        printf("STATION NAME %s\n",cfg->pmu[j]->stn);				
        s = s + 16;

        //SEPARATE IDCODE		
        copy_cbyc (idcode_l,(unsigned char *)s,2);
        idcode_l[2]='\0';
        cfg->pmu[j]->idcode = to_intconvertor(idcode_l);
        //printf("ID Code %d\n",cfg->pmu[j]->idcode);
        s = s + 2;

        //SEPARATE DATA FORMAT		
        copy_cbyc ((unsigned char *)cfg->pmu[j]->data_format,(unsigned char *)s,2);
        cfg->pmu[j]->data_format[2]='\0';
        //printf("PMU format %s\n",cfg->pmu[j]->data_format);
        s = s + 2;

        unsigned char hex = cfg->pmu[j]->data_format[1];
        hex <<= 4;

        // Extra field has been added to identify polar,rectangular,floating/fixed point	
        cfg->pmu[j]->fmt = malloc(sizeof(struct format));
        if((hex & 0x80) == 0x80) cfg->pmu[j]->fmt->freq = 1; else cfg->pmu[j]->fmt->freq = 0;
        if((hex & 0x40) == 0x40 ) cfg->pmu[j]->fmt->analog = 1; else cfg->pmu[j]->fmt->analog = 0;
        if((hex & 0x20) == 0x20) cfg->pmu[j]->fmt->phasor = 1; else cfg->pmu[j]->fmt->phasor = 0;
        if((hex & 0x10) == 0x10) cfg->pmu[j]->fmt->polar =  1; else cfg->pmu[j]->fmt->polar = 0;

        //SEPARATE PHASORS	
        copy_cbyc (phnmr,(unsigned char *)s,2);
        phnmr[2]='\0';
        cfg->pmu[j]->phnmr = to_intconvertor(phnmr);
//        printf("Phasors %d\n",cfg->pmu[j]->phnmr);
        s = s + 2;

        //SEPARATE ANALOGS			
        copy_cbyc (annmr,(unsigned char *)s,2);
        annmr[2]='\0';
        cfg->pmu[j]->annmr = to_intconvertor(annmr);
//        printf("Analogs %d\n",cfg->pmu[j]->annmr);
        s = s + 2;

        //SEPARATE DIGITALS			
        copy_cbyc (dgnmr,(unsigned char *)s,2);
        dgnmr[2]='\0';
        cfg->pmu[j]->dgnmr = to_intconvertor(dgnmr);
//        printf("Digitals %d\n",cfg->pmu[j]->dgnmr);
        s = s + 2; 

        cn = malloc(sizeof(struct channel_names));
        cn->first = NULL;

        ////SEPARATE PHASOR NAMES 
        if(cfg->pmu[j]->phnmr != 0){
            cn->phtypes = malloc((cfg->pmu[j]->phnmr) * sizeof(unsigned char*));
            cn->phnames = malloc((cfg->pmu[j]->phnmr) * sizeof(unsigned char*));
            if(!cn->phnames) {
                printf("Not enough memory cfg->pmu[j]->cn->phnames[][]\n");
                exit(1);
            }

            for (i = 0; i < cfg->pmu[j]->phnmr; i++) {

                cn->phnames[i] = malloc(17*sizeof(unsigned char));
                memset(cn->phnames[i],'\0',17);
                cn->phtypes[i] = malloc(2*sizeof(unsigned char));
                memset(cn->phtypes[i],'\0',2);

            }

            cfg->pmu[j]->phunit = malloc(cfg->pmu[j]->phnmr*sizeof(float*));
            if(!cfg->pmu[j]->phunit) {
                printf("Not enough memory cfg.pmu[j]->phunit[][]\n");
                exit(1);
            }

            for (i = 0; i < cfg->pmu[j]->phnmr; i++) {
                cfg->pmu[j]->phunit[i] = malloc(sizeof(float));
            }


            i = 0;//Index for PHNAMES
            while(i<cfg->pmu[j]->phnmr){

                copy_cbyc (cn->phnames[i],(unsigned char *)s,16);
                cn->phnames[i][16] = '\0';
//                printf("Phnames %s\n",cn->phnames[i]);
                s = s + 16;
                i++;
            }	
        } 		

        //SEPARATE ANALOG NAMES
        if(cfg->pmu[j]->annmr != 0){
            cn->antypes =  malloc((cfg->pmu[j]->annmr)*sizeof(unsigned char*));
            cn->angnames = malloc((cfg->pmu[j]->annmr)*sizeof(unsigned char*));
            if(!cn->angnames) {

                printf("Not enough memory cfg->pmu[j]->cn->phnames[][]\n");
                exit(1);
            }

            for (i = 0; i < cfg->pmu[j]->annmr; i++) {

                cn->angnames[i] = malloc(17*sizeof(unsigned char));
                memset(cn->angnames[i],'\0',17);
                cn->antypes[i] = malloc(5*sizeof(unsigned char));
                memset(cn->antypes[i],'\0',5);
            }

            cfg->pmu[j]->anunit = malloc(cfg->pmu[j]->annmr*sizeof(float*));
            if(!cfg->pmu[j]->anunit) {
                printf("Not enough memory cfg.pmu[j]->anunit[][]\n");
                exit(1);
            }

            for (i = 0; i < cfg->pmu[j]->annmr; i++) {
                cfg->pmu[j]->anunit[i] = malloc(sizeof(float));
            }

            i = 0;//Index for ANGNAMES

            while(i<cfg->pmu[j]->annmr){
                copy_cbyc (cn->angnames[i],(unsigned char *)s,16);
                cn->angnames[i][16]='\0';
//                printf("ANGNAMES %s\n",cn->angnames[i]);
                s = s + 16; 
                i++;
            }
        }


        if(cfg->pmu[j]->dgnmr != 0){

            cfg->pmu[j]->dgunit = malloc(cfg->pmu[j]->dgnmr*sizeof(unsigned char*));
            if(!cfg->pmu[j]->dgunit) {

                printf("Not enough memory cfg->pmu[j]->dgunit[][]\n");
                exit(1);
            }

            for (i = 0; i < cfg->pmu[j]->dgnmr; i++) {

                cfg->pmu[j]->dgunit[i] = malloc(5);
            }
        }

        i = 0; 
        int di;//Index for number of dgwords

        struct dgnames *q;

        while(i < cfg->pmu[j]->dgnmr) {

            struct dgnames *temp1 = malloc(sizeof(struct dgnames));
            temp1->dgn = malloc(16*sizeof(unsigned char *));	
            if(!temp1->dgn) {

                printf("Not enough memory temp1->dgn\n");
                exit(1);
            }

            for (di = 0; di < 16; di++) {

                temp1->dgn[di] = malloc(17*sizeof(unsigned char));

            }

            temp1->dg_next = NULL;

            for(dgchannels = 0;dgchannels < 16;dgchannels++){

                memset(temp1->dgn[dgchannels],'\0',16);
                copy_cbyc (temp1->dgn[dgchannels],(unsigned char *)s,16);
                temp1->dgn[dgchannels][16] = '\0';
                s += 16;
//                printf("%s\n",temp1->dgn[dgchannels]);
            }

            if(cn->first == NULL){
                cn->first = q = temp1;

            } else {

                while(q->dg_next!=NULL){
                    q = q->dg_next;
                }
                q->dg_next = temp1;			       
            }  

            i++;  
        } //DGWORD WHILE ENDS

        cfg->pmu[j]->cnext = cn;//Assign to pointers

        ///PHASOR FACTORS
        if(cfg->pmu[j]->phnmr != 0){

            i = 0;
            while(i < cfg->pmu[j]->phnmr){ //Separate the Phasor conversion factors

                memset(buf,'\0',9);
                copy_cbyc (buf,(unsigned char *)s,1);
                buf[1] = '\0';
                s = s + 1;
                phtype = to_intconvertor1(buf);
                copy_cbyc (buf,(unsigned char *)s,3);
                buf[3] = '\0';
                s = s + 3;
                l_phunit = to_long_int_convertor1(buf);

                if (phtype == 0)
                {
                    cfg->pmu[j]->cnext->phtypes[i] = "V";
                    if (cfg->pmu[j]->fmt->phasor == 0)
                    {
                        *cfg->pmu[j]->phunit[i] = l_phunit * 1e-5;
                    }
                    else
                    {
                        *cfg->pmu[j]->phunit[i] = 1;
                    }
                }
                else if (phtype == 1)
                {
                    cfg->pmu[j]->cnext->phtypes[i] = "I";
                    if (cfg->pmu[j]->fmt->phasor == 0)
                    {
                        *cfg->pmu[j]->phunit[i] = l_phunit * 1e-5;
                    }
                    else
                    {
                        *cfg->pmu[j]->phunit[i] = 1;
                    }
                }
                else
                {
                    perror("Invalid first byte in PHUNIT - probably error in PMU");
                }

//                printf("Phasor Factor %d = %f\n",i,*cfg->pmu[j]->phunit[i]);
                i++;
            }
        }//if for PHASOR Factors ends

        //ANALOG FACTORS
        if(cfg->pmu[j]->annmr != 0){

            i=0;
            while(i<cfg->pmu[j]->annmr){ //Separate the Phasor conversion factors

                memset(buf,'\0',9);
                copy_cbyc (buf,(unsigned char *)s,1);
                buf[1] = '\0';
                s = s + 1;

                antype = to_intconvertor1(buf);
                switch(antype)
                {
                    case 0 : cfg->pmu[j]->cnext->antypes[i] = "POW";
                             break;
                    case 1 : cfg->pmu[j]->cnext->antypes[i] = "RMS";
                             break;
                    case 2 : cfg->pmu[j]->cnext->antypes[i] = "PEAK";
                             break;
                    default: 
                             {
                                 if (antype >= 65 && antype <= 255)
                                 {
                                     cfg->pmu[j]->cnext->antypes[i] = "USERDEF";
                                 }
                                 else
                                     perror("Invalid first byte in ANUNIT?");
                                 break;
                             }
                }

                copy_cbyc (buf,(unsigned char *)s,3);
                buf[3] = '\0';
                s = s + 3;
                l_anunit = to_long_int_convertor1(buf);

                if (cfg->pmu[j]->fmt->analog == 0)
                {
                    *cfg->pmu[j]->anunit[i] = l_anunit * 1e-5; // Assumed factor of 1e5 in iPDC
                }
                else
                {
                    *cfg->pmu[j]->anunit[i] = 1;
                }

//                printf("Analog Factor %d = %f\n",i,*cfg->pmu[j]->anunit[i]);
                i++;
            }

        } // if for ANALOG FActtors ends

        ///DIGITAL FACTORS
        if(cfg->pmu[j]->dgnmr != 0){

            i = 0;
            while(i < cfg->pmu[j]->dgnmr ){ //Separate the Digital conversion factors

                copy_cbyc(cfg->pmu[j]->dgunit[i],(unsigned char *)s,4);
                cfg->pmu[j]->dgunit[i][4] = '\0';
//                printf("DGWORD %s\n",cfg->pmu[j]->dgunit[i]); 
                s += 4;
                i++;
            }
        } //if for Digital Words FActtors ends

        copy_cbyc (fnom,(unsigned char *)s,2);
        fnom[2]='\0';
        cfg->pmu[j]->fnom = to_intconvertor(fnom);
//        printf("FREQUENCY %d\n",cfg->pmu[j]->fnom);
        s = s + 2;

        copy_cbyc (cfg_cnt,(unsigned char *)s,2);
        cfg_cnt[2] = '\0';
        cfg->pmu[j]->cfg_cnt = to_intconvertor(cfg_cnt);
//        printf("CFG CHANGE COUNT %d\n",cfg->pmu[j]->cfg_cnt);
        s = s + 2;
        j++; 
    }//While for PMU number ends

    copy_cbyc (data_rate,(unsigned char *)s,2);
    data_rate[2] = '\0';
    cfg->data_rate = to_intconvertor(data_rate);
//    printf("Data Rate %d\n",cfg->data_rate);
    s += 2;
    cfg->cfgnext = NULL;

    // Adjust the configuration object pointers

    // Lock the mutex_cfg    
    pthread_mutex_lock(&mutex_cfg);

    // Index is kept to replace the cfgfirst if it matches
    int index = 0; 

    if (cfgfirst == NULL)  { // Main if
        printf("1\n");
        cfgfirst = cfg;	
        fclose(fp);    

    } else {

        struct cfg_frame *temp_cfg = cfgfirst,*tprev_cfg;
        tprev_cfg = temp_cfg;

        //Check if the configuration frame already exists
        while(temp_cfg!=NULL){

            if(cfg->idcode == temp_cfg->idcode) {

                printf("CFG PRESENT NEED TO REPLACE\n");
                match = 1;
                break;	

            } else {

                index++;
                tprev_cfg = temp_cfg;
                temp_cfg = temp_cfg->cfgnext;
            }
        }// While ends

        if(match) {

            if(!index) {

                // Replace the cfgfirst
                cfg->cfgnext = cfgfirst->cfgnext;
                free_cfgframe_object(cfgfirst);				
                cfgfirst = cfg;

                // Get the new value of the CFG frame
                fclose(fp);
                remove_old_cfg(cfg->idcode,st);      

            } else {

                // Replace in between cfg
                tprev_cfg->cfgnext = cfg;
                cfg->cfgnext = temp_cfg->cfgnext;
                free_cfgframe_object(temp_cfg);
                fclose(fp);
                remove_old_cfg(cfg->idcode,st);      
            }

        } else { // No match and not first cfg 

            tprev_cfg->cfgnext = cfg;
            fclose(fp);
        }   

    } //Main if

    // cfginsert(cfg); // DATABASE INSERTION

    pthread_mutex_unlock(&mutex_cfg);
    pthread_mutex_unlock(&mutex_file);

    free(sync);
    free(frame_size);
    free(idcode_h);
    free(soc);
    free(fracsec);
    free(time_base);
    free(num_pmu);
    free(stn);
    free(idcode_l);
    free(format);
    free(phnmr);
    free(annmr);
    free(dgnmr);
    free(fnom);
    free(cfg_cnt);
    free(data_rate);
    free(buf);
} 


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  remove_old_cfg():              	     				*/
/* It removes the old configuration frame from the file `cfg.bin` and replaces 	*/
/* it with new configuration frame at same position in the file. 	  	*/
/* ----------------------------------------------------------------------------	*/

void remove_old_cfg(int idcode,unsigned char frame[]) { // Begins

    FILE *file,*newfile;
    int result;
    unsigned int idCODE,framesize;
    unsigned char *s,id[3],*line,frame_len[2];
    unsigned long fileLen;

    file = fopen("cfg.bin","rb");
    newfile = fopen("ncfg.bin","wb");

    if (file != NULL) {

        //Get file length
        fseek(file, 0, SEEK_END);
        fileLen = ftell(file);
        fseek(file, 0, SEEK_SET);
        printf("BEFORE REMOVAL OF OLDFRAME FILE LEN %ld\n",fileLen);

        while (fileLen != 0) /* Till the EOF */{

            fseek (file,2 ,SEEK_CUR);
            fread(frame_len, sizeof(unsigned char),2, file);
            fseek (file,-4 ,SEEK_CUR);

            framesize = to_intconvertor(frame_len);
            line = malloc(framesize*sizeof(unsigned char));
            memset(line,'\0',sizeof(line));
            fread(line, sizeof(unsigned char),framesize,file);				
            s = line;
            s += 4;
            //match IDCODE in cfg.bin file
            copy_cbyc (id,s,2);
            id[2] = '\0';	
            idCODE = to_intconvertor(id);

            if(idCODE == idcode) {

                printf("IDcode of new cfg MATCHED with old cfg in file cfg.bin!\n");
                break;

            } else {

                //Place rest of lines in the new file			
                fwrite(line, sizeof(unsigned char),framesize,newfile);
                free(line);
                fileLen -= framesize;
            }        
        }//While ends

        // The new cfg is copied in the ncfg.bin file 			
        unsigned int len;
        unsigned char *p = frame;			
        p += 2;
        copy_cbyc (frame_len,p,2);
        len = to_intconvertor(frame_len);
        fwrite(frame, sizeof(unsigned char),len,newfile);

        // If cfg.bin file still contains data copy it to ncfg.bin
        while (fileLen != 0) /* Till the EOF */{

            fseek (file,2 ,SEEK_CUR);
            fread(frame_len, sizeof(unsigned char),2, file);
            fseek (file,-4 ,SEEK_CUR);

            framesize = to_intconvertor(frame_len);
            line = malloc(framesize*sizeof(unsigned char));
            memset(line,'\0',sizeof(line));
            fread(line, sizeof(unsigned char),framesize,file);				

            if(!ncmp_cbyc(line,frame,framesize)) {                  		   

                //This skips the last line of the file that contains already added cfg
                //hence we dont copy this line to ncfg.bin
                break;

            } else {	

                fwrite(frame, sizeof(unsigned char),framesize,newfile);
                free(line);						
                fileLen -= framesize;	  				
            }	
        }	

        //File renaming 
        fclose (file);
        fclose(newfile);

        if( remove( "cfg.bin" ) != 0 )
            perror( "Error deleting file" );
        result= rename("ncfg.bin","cfg.bin");
        if ( result == 0 )
            printf ("File successfully renamed\n");
        else
            perror( "Error renaming file\n" );

    } else {

        perror ("cfg.bin"); /* why didn't the file open? */
    }
} 


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  cfginsert():              	     					*/
/* It inserts/updates the configuration frames in the configuration tables.	*/
/* ----------------------------------------------------------------------------	*/
/* This function deleted by Gopal on 20151211 remove mysql */

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  dataparser():                                	     		*/
/* Parses the data frames. It searches for configuration objects that matches   */ 
/* with the IDCODE and then inserts into data tables. 				*/
/* ----------------------------------------------------------------------------	*/

int dataparser(unsigned char data[]) { 


    struct cfg_frame *temp_cfg;
    int match = 0,i,j = 0;
    int stat_status,config_change = 0;
    unsigned int t_id,num_pmu,phnmr,annmr,dgnmr,comtrade_tstamp ;
    float fp_r,fp_i,fp_real,fp_imaginary,fp_analogs;
    long int f_r,f_i,f_analogs,f_freq,f_dfreq,l_soc,l_fracsec;
    short int s_analogs, s_freq, s_dfreq;
    float fp_freq,fp_dfreq; 
    
    unsigned char *sync,*framesize,*idcode,*soc,*fracsec,*timequality,*stat,*phasors,*analogs,*digital,*freq,*dfreq,*d;
    unsigned char *fp_left,*fp_right;
    unsigned char *f_left,*f_right;
    //char *cmd;

    //cmd = malloc(500);
    sync = malloc(3*sizeof(unsigned char));
    framesize = malloc(3*sizeof(unsigned char));
    idcode = malloc(3*sizeof(unsigned char));
    soc = malloc(5*sizeof(unsigned char));
    fracsec  = malloc(5*sizeof(unsigned char));
    timequality  = malloc(2*sizeof(unsigned char));
    stat = malloc(3*sizeof(unsigned char));
    phasors = malloc(9*sizeof(unsigned char));
    analogs = malloc(5*sizeof(unsigned char));
    digital = malloc(3*sizeof(unsigned char));
    freq = malloc(5*sizeof(unsigned char));
    dfreq = malloc(5*sizeof(unsigned char));

    //memset(cmd,'\0',500);
    memset(sync,'\0',3);
    memset(framesize,'\0',3);
    memset(idcode,'\0',3);
    memset(soc,'\0',5);
    memset(fracsec,'\0',5);
    memset(timequality,'\0',2);
    memset(stat,'\0',3);
    memset(phasors,'\0',9);
    memset(analogs,'\0',5);
    memset(digital,'\0',3);
    memset(freq,'\0',5);
    memset(dfreq,'\0',5);

    fp_left = malloc(5);
    fp_right = malloc(5);
    f_left = malloc(3);
    f_right = malloc(3);

    memset(fp_left,'\0',5);
    memset(fp_right,'\0',5);
    memset(f_left,'\0',3);
    memset(f_right,'\0',3);

    d = data;

    //Skip SYN 
    d += 2;

    //SEPARATE FRAMESIZE
    copy_cbyc (framesize,d,2);
    framesize[2] = '\0';
    d += 2;

    //SEPARATE IDCODE
    copy_cbyc (idcode,d,2);
    idcode[2] ='\0';
    d += 2;
    t_id = to_intconvertor(idcode);
    //printf("ID Code %d\n",t_id);

    // Check for the IDCODE in Configuration Frame
    pthread_mutex_lock(&mutex_cfg);
    temp_cfg = cfgfirst;

    while(temp_cfg != NULL){

        if(t_id == temp_cfg->idcode) {

            match = 1;
            break;	

        } else {

            temp_cfg = temp_cfg->cfgnext;
        }
    }
    pthread_mutex_unlock(&mutex_cfg);

    pthread_mutex_lock(&mutex_MYSQL_CONN_ON_DATA);

    if(match){	// idcode matches with cfg idcode

        //	printf("Inside DATAPARSER, data frame is matched with CFG.\n");

        // Allocate Memeory For Each PMU
        num_pmu = temp_cfg->num_pmu; 	

        //Copy SOC						
        copy_cbyc (soc,d,4);
        soc[4] = '\0';
        l_soc = to_long_int_convertor(soc);			
        d += 4;

        //Copy FRACSEC : First seprate the first Byte of Time Quality Flags
        copy_cbyc (timequality,d,1);
        timequality[1] = '\0';
        d += 1;

        //First seprate the next 3-Byte of Actual Fraction of Seconds
        copy_cbyc (fracsec,d,3);
        fracsec[3] = '\0';
        l_fracsec = to_long_int_convertor1(fracsec);
        l_fracsec = roundf((l_fracsec*1e6)/(temp_cfg->time_base));
        d += 3;
        // Added by Gopal and Rajeev on 25th Feb 2014.
        if (tStartArchive == 0) {
            struct tm *t1;
            t1 = localtime(&l_soc);
            t1->tm_sec = 0;
            t1->tm_min = 0;
            tStartArchive = mktime(t1); 
            binData = malloc(cfgfirst->num_pmu *sizeof(FILE*));
            srNo = malloc((cfgfirst->num_pmu)*sizeof(unsigned long int)); 
            endSampPosition = malloc((cfgfirst->num_pmu)*sizeof(unsigned int));
            cfgFileName = malloc((cfgfirst->num_pmu)*sizeof(char*));
            //initfilestreams(l_soc,l_fracsec);
            initfilestreams(tStartArchive,0);
        }
		/* Added by Gopal 2015-04-21 */ 
		if (l_fracsec == 0){
		   switch (l_soc % 4 ){
			   case 0:
				   printf("HeartBeat = %ld . ===>           \r", l_soc);
				   break;
			   case 1:
				   printf("HeartBeat = %ld o ======>       \r", l_soc);
				   break;
			   case 2:
				   printf("HeartBeat = %ld 0 =========>    \r", l_soc);
				   break;
			   case 3:
				   printf("HeartBeat = %ld O =============>\r", l_soc);
		   }
		}

        // Separate the data for each PMU	    	
        while(j < num_pmu) {		  					

            memset(stat,'\0',3);
            copy_cbyc (stat,d,2);
            stat[2] = '\0';
            d += 2;


            // Check Stat Word for each data block 
            stat_status = check_statword(stat);

            if((stat_status == 14)||(stat_status == 10)) {

                //				memset(stat,'\0',3);
                config_change = stat_status;
            }

			/* Commented by Gopal 2015-04-21  
			 * memset(cmd,'\0',500);
			 sprintf(cmd," %d,%d,%s,%ld,%ld,%x%x \n",temp_cfg->idcode,temp_cfg->pmu[j]->idcode,temp_cfg->pmu[j]->stn,l_soc,l_fracsec,stat[0],stat[1]);

			 dataCollectInBuffer(cmd,&statBuff);
			 */

            // Extract PHNMR, DGNMR, ANNMR
            phnmr = temp_cfg->pmu[j]->phnmr;	
            annmr = temp_cfg->pmu[j]->annmr;
            dgnmr = temp_cfg->pmu[j]->dgnmr;

            pthread_mutex_lock(&mutex_phasor_buffer);


            if (l_soc >= (tStartArchive + 3600)){
                tStartArchive += 3600;
                closefilestreams();
                //initfilestreams(l_soc, l_fracsec);
                initfilestreams(tStartArchive, 0);
            }

            ++srNo[j];
            fwrite(&srNo[j],sizeof(unsigned int),1,binData[j]);
            comtrade_tstamp = roundf(((l_soc - tStartArchive) + l_fracsec/1e6)*cfgfirst->data_rate);
            //fwrite(&srNo[j],sizeof(unsigned int),1,binData[j]);
            fwrite(&comtrade_tstamp,sizeof(unsigned int),1,binData[j]);
            //Phasors 
            if(phnmr != 0) {

                if(temp_cfg->pmu[j]->fmt->phasor == 1) { // Floating

                    for(i = 0;i<phnmr;i++){	

                        memset(fp_left,'\0',5);
                        memset(fp_right,'\0',5);
                        copy_cbyc (fp_left,d,4);
                        fp_left[4] = '\0';
                        d += 4;

                        copy_cbyc(fp_right,d,4);
                        fp_right[4] = '\0';   
                        d += 4;

                        fp_r = decode_ieee_single(fp_left);
                        fp_i = decode_ieee_single(fp_right);

                        if(temp_cfg->pmu[j]->fmt->polar == 1) { // POLAR

                            /*fp_real = fp_r*cos(f_i);
                              fp_imaginary = fp_r*sin(f_i);

                              Commented by Gopal on 8th Aug 2012.
                              We want to store polar values in the table */

                            fp_real = fp_r;
                            fp_imaginary = fp_i;
                        }
                        else // RECTANGULAR 
                        {
                            fp_real = hypotf(fp_r,fp_i);
                            fp_imaginary = atan2f(fp_i, fp_r);
                        }
						/* Commented by Gopal 2015-04-21  
						   memset(cmd,'\0',500);
						   sprintf(cmd," %d,%d,%ld,%ld,\"%s\",%f,%f\n",temp_cfg->idcode,temp_cfg->pmu[j]->idcode,l_soc,l_fracsec,temp_cfg->pmu[j]->cnext->phnames[i],fp_real,fp_imaginary);
						//sprintf(cmd," %d,%d,\"%s\",%ld,%ld,\"%s\",%f,%f\n",temp_cfg->idcode,temp_cfg->pmu[j]->idcode,temp_cfg->pmu[j]->stn,l_soc,l_fracsec,temp_cfg->pmu[j]->cnext->phnames[i],fp_real,fp_imaginary);
						dataCollectInBuffer(cmd, &phasorBuff);
						 */
                        // Added by Gopal on 22nd Feb 2014
                        fwrite(&fp_real,sizeof(float),1,binData[j]);
                        fwrite(&fp_imaginary,sizeof(float),1,binData[j]);

                    }
                } 
                else { // Fixed point

                    for(i = 0;i < phnmr; i++){	

                        memset(f_left,'\0',3);
                        memset(f_right,'\0',3);
                        copy_cbyc (f_left,d,2);
                        f_left[2] = '\0';
                        d += 2;

                        copy_cbyc(f_right,d,2);
                        f_right[2] = '\0';   
                        d += 2;

                        f_r = to_intconvertor(f_left);
                        f_i = to_intconvertor(f_right);

                        if(temp_cfg->pmu[j]->fmt->polar == 1) { // POLAR

                            fp_real = *temp_cfg->pmu[j]->phunit[i] *f_r;
                            // Added by Gopal on 1st Aug 2020
                            if (f_i > pow(2,15))
                            {
                                fp_imaginary = (f_i - pow(2,16))*1e-4;
                            }
                            else
                            {
                                fp_imaginary = f_i*1e-4; // Angle is in 10^4 radians
                            }
                            // Addition of 1st Aug 2020 ends
                        }
                        else // RACTANGULAR
                        {
                            fp_r = *temp_cfg->pmu[j]->phunit[i] *f_r;
                            fp_i = *temp_cfg->pmu[j]->phunit[i] *f_i;

                            fp_real = hypotf(fp_r,fp_i);
                            fp_imaginary = atan2f(fp_i, fp_r);
                        }

						/* Commented by Gopal 2015-04-21  
						   memset(cmd,'\0',500);
						   sprintf(cmd," %d,%d,%ld,%ld,\"%s\",%f,%f\n",temp_cfg->idcode,temp_cfg->pmu[j]->idcode,l_soc,l_fracsec,temp_cfg->pmu[j]->cnext->phnames[i],fp_real,fp_imaginary);
						   dataCollectInBuffer(cmd, &phasorBuff);
						 */
                        // Added by Gopal on 22nd Feb 2014
                        fwrite(&fp_real,sizeof(float),1,binData[j]);
                        fwrite(&fp_imaginary,sizeof(float),1,binData[j]);
                    }
                } 
            }// Phasors Insertion ends

            //Freq 
            if(temp_cfg->pmu[j]->fmt->freq == 1) { // FLOATING

                memset(freq,'\0',5);
                copy_cbyc (freq,d,4);
                freq[4] = '\0';
                d += 4;					 

                memset(dfreq,'\0',5);
                copy_cbyc (dfreq,d,4);
                dfreq[4] = '\0';
                d += 4;					 

                fp_freq = decode_ieee_single(freq);
                fp_dfreq = decode_ieee_single(dfreq);

            } else { // FIXED

                memset(freq,'\0',5);
                copy_cbyc (freq,d,2);
                freq[2] = '\0';
                d += 2;					 

                memset(dfreq,'\0',5);
                copy_cbyc (dfreq,d,2);
                dfreq[2] = '\0';
                d += 2;	
                s_freq = to_intconvertor(freq);
                s_dfreq = to_intconvertor(dfreq);

                fp_freq = s_freq*1e-3; // freq is in mHz deviation from nominal
                if (temp_cfg->pmu[j]->fnom == 0)
                    fp_freq = 60 + fp_freq;
                else
                    fp_freq = 50 + fp_freq;
                fp_dfreq = s_dfreq*1e-2; // dfreq is 100 times hz/sec 
            }

			/* Commented by Gopal 2015-04-21  
			   memset(cmd,'\0',500);
			   sprintf(cmd," %d,%d,%ld,%ld,%f,%f\n",temp_cfg->idcode,temp_cfg->pmu[j]->idcode,l_soc,l_fracsec,fp_freq,fp_dfreq);

			   dataCollectInBuffer(cmd, &frequencyBuff); // Freq Insert Ends
			 */

            //Analogs
            if(annmr != 0) {

                if(temp_cfg->pmu[j]->fmt->analog == 1) { // FLOATING

                    for(i = 0; i < annmr; i++){					

                        memset(analogs,'\0',5);
                        copy_cbyc(analogs,d,4);
                        d += 4;
                        analogs[4] = '\0';

                        fp_analogs = decode_ieee_single(analogs);
                        fp_analogs =  *temp_cfg->pmu[j]->anunit[i]*fp_analogs;;
						/* Commented by Gopal 2015-04-21  
						   memset(cmd,'\0',500);

						   sprintf(cmd," %d,%d,%ld,%ld,\"%s\",%f\n",temp_cfg->idcode,temp_cfg->pmu[j]->idcode,l_soc,l_fracsec,temp_cfg->pmu[j]->cnext->angnames[i],fp_analogs);

						   dataCollectInBuffer(cmd, &analogBuff);
						 */
                        // Added by Gopal on 22nd Feb 2014
                        fwrite(&fp_analogs,sizeof(float),1,binData[j]);
                    }

                } else { // FIXED

                    for(i = 0; i < annmr; i++){					

                        memset(analogs,'\0',5);
                        copy_cbyc (analogs,d,2);
                        d += 2;

                        analogs[2] = '\0';
                        s_analogs = to_intconvertor(analogs);
                        fp_analogs = *temp_cfg->pmu[j]->anunit[i]*s_analogs ;

						/* Commented by Gopal 2015-04-21  
						   memset(cmd,'\0',500);
						   sprintf(cmd," %d,%d,%ld,%ld,\"%s\",%f\n",temp_cfg->idcode,temp_cfg->pmu[j]->idcode,l_soc,l_fracsec,temp_cfg->pmu[j]->cnext->angnames[i],fp_analogs);

						   dataCollectInBuffer(cmd, &analogBuff);
						 */
                        // Added by Gopal on 22nd Feb 2014
                        fwrite(&fp_analogs,sizeof(float),1,binData[j]);
                    }
                }
            } // Insertion for Analog done here.

            // Added by Gopal on 27th Feb 2014
            // After the analog addition is done in dat file the frequency and
            // df/dt is added
            //
            fwrite(&fp_freq,sizeof(float),1,binData[j]);
            fwrite(&fp_dfreq,sizeof(float),1,binData[j]);

            // Now for the addition of digital status words in dat file
            // Initial is 16 bits for time quality and next 16 bits for status
            // word
            //

            unsigned short int timeQualityInt, statInt;
            timeQualityInt = to_shortintconvertor(timequality);							
            statInt = to_shortintconvertor(stat);							
            fwrite(&timeQualityInt,sizeof(unsigned short int),1,binData[j]);
            fwrite(&statInt,sizeof(unsigned short int),1,binData[j]);

            // Digital
            if(dgnmr != 0) {

                unsigned short int dgword;

                for(i = 0; i<dgnmr; i++) {

                    memset(digital,'\0',3);
                    copy_cbyc (digital,d,2);
                    d += 2;
                    digital[2] = '\0';
                    dgword = to_shortintconvertor(digital);							

					/* Commented by Gopal 2015-04-21  
					   memset(cmd,'\0',500);
					   sprintf(cmd," %d,%d,%ld,%ld,%u\n",temp_cfg->idcode,temp_cfg->pmu[j]->idcode,l_soc,l_fracsec,dgword);

					   dataCollectInBuffer(cmd, &digitalBuff);
					 */
                    // Added by Gopal on 27th Feb 2014
                    fwrite(&dgword,sizeof(unsigned short int),1,binData[j]);
                }
            } // Insertion for Digital done here.

            struct timeval tv;
            long local_soc, local_fsec,ms_diff,s_diff;

            /* Obtain the time of day, and convert it to a tm struct. */
            gettimeofday (&tv, NULL);

            local_soc = tv.tv_sec;
            local_fsec = tv.tv_usec;
            s_diff = (tv.tv_sec - l_soc);
            ms_diff = (tv.tv_usec - l_fracsec);

            // Formula to calculate the exact delay in micro between data frame inside-time and
            // system receive time at which that data frame received.
            ms_diff = ((s_diff == 0) ? ((ms_diff > 0) ? ms_diff : -1*ms_diff) : ((s_diff == 1) ? (1000000-l_fracsec+tv.tv_usec) : ((1000000*(s_diff-1))+(1000000-l_fracsec+tv.tv_usec))));

			/* Commented by Gopal 2015-04-21  
			   memset(cmd,'\0',500);
			   sprintf(cmd," %d,%d,%ld,%ld,%ld,%ld,%ld\n",temp_cfg->idcode,temp_cfg->pmu[j]->idcode,l_soc,l_fracsec,local_soc,local_fsec,ms_diff);
			   dataCollectInBuffer(cmd, &delayBuff);
			 */
            updatecomtradecfg(j);

            pthread_mutex_unlock(&mutex_phasor_buffer);
            j++;
        } //While ends 

    } else {

        printf("NO CFG for data frames\n");	
    }  

    pthread_mutex_unlock(&mutex_MYSQL_CONN_ON_DATA);	

    //free(cmd);
    free(sync);
    free(framesize);
    free(idcode);
    free(soc);
    free(fracsec);
    free(timequality);
    free(stat);
    free(phasors);
    free(analogs);
    free(digital);
    free(freq);
    free(dfreq);

    free(fp_left);
    free(fp_right);
    free(f_left);
    free(f_right);

    if((config_change == 14) ||(config_change == 10)) 
        return config_change;
    else return stat_status;
} 


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  check_statword():                                	     		*/
/* Check the STAT word of the data frames for any change in the data block.	*/
/* Some of the prime errors are handled.				 	*/
/* ----------------------------------------------------------------------------	*/

int check_statword(unsigned char stat[]) { 

    /*
       STAT Word Bits representation:

       Bit-09	=	0200	=	Data modified by PDC, for handling missing data frame? 
       Bit-10	=	0400	=	Configuration change, set to 1 for 1 min to advise configuration will change
       Bit-11	=	0800	=	PMU trigger detected, 0 when no trigger.
       Bit-12	=	0400	=	Data sorting, 0 by time stamp, 1 by arrival.
       Bit-13	=	2000	=	PMU sync, 0 when in sync with a UTC traceable time source
       Bit-14 & 15 =  Data error:
       01 =	4000	=	PMU error. including configuration error?.
       10 =	8000	=	PMU in test mode (do not use values) or
       absent data tags have been inserted (do not use values)
       11 =	12000=	PMU error (do not use values)

*/

    //	int ret = 0;

    /* For missing data frame? According to 2011 standard! */
    /*	if(stat[1] == 0x02) {	// Bit-09

        printf("Data Frame missing or Data modified by PDC?\n");
        ret = 9;	// earlier 16
        return ret;

        } else if ((stat[0] & 0x04) == 0x04) {	// Bit-10

        printf("Configuration Change error!\n");
        ret = 10;
        return ret;

        } else if ((stat[0] & 0x120) == 0x120) {	// Bit-14 & 15

        printf("PMU error (do not use values)!\n");
        ret = 16;
        return ret;

        } else if ((stat[0] & 0x40) == 0x40) {	// Bit-14

        printf("PMU error may include configuration error\n");
        ret = 14;
        return ret;

        } else if((stat[0] & 0x80) == 0x80) {	// Bit-15

        printf("PMU in test mode or data invalid?\n");
        ret = 15;
        return ret;

        } else if ((stat[0] & 0x20) == 0x20) {	// Bit-13

        printf("PMU Sync error\n");
        ret = 13;
        return ret;

        } else if ((stat[0] & 0x10) == 0x10) {	// Bit-12

        printf("Data sorting changed to by arrival!\n");
        ret = 12;
        return ret;

        } else if ((stat[0] & 0x08) == 0x08) {	// Bit-11

        printf("PMU Trigger detected!\n");
        ret = 11;
        return ret;
        } 

        printf("Return from STAT Word checking - %d.\n",ret);
        return ret;
        */
    int ret = 0;

    if(stat[0] == 0x82) {

        // DEVELOPERS has used these bits as an indication for PMU data that has not arrived/missing data frames
        // Changed by Kedar on 4-7-13
        ret = 16;
        return ret;

    } else if ((stat[0] & 0x04) == 0x04) {

        printf("Configuration Change error\n");
        ret = 10;
        return ret;

    } else if ((stat[0] & 0x40) == 0x40) {

        //		printf("PMU error including configuration error\n");
        ret = 14;
        return ret;

    } else if((stat[0] & 0x80) == 0x80) {

        //		printf("Data invalid\n");
        ret = 15;
        return ret;

    } else if ((stat[0] & 0x20) == 0x20) {

        //		printf("PMU Sync error\n");
        ret = 13;
        return ret;

    } else if ((stat[0] & 0x10) == 0x10) {

        //		printf("Data sorting error\n");
        ret = 12;
        return ret;

    } else if ((stat[0] & 0x08) == 0x08) {

        //		printf("PMU Trigger error\n");
        ret = 11;
        return ret;
    } 

    //printf("Return from STAT Word checking %d.\n",ret);
    return ret;
} 


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  to_intconvertor():                                	     		*/
/* ----------------------------------------------------------------------------	*/

unsigned int to_intconvertor(unsigned char array[]) {

    unsigned int n;
    n = array[0];
    n <<= 8;
    n |= array[1];
    return n;
}

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  to_shortintconvertor1():                                	     		*/
/* Written by Gopal to convert just 1 byte to int                               */
/* ----------------------------------------------------------------------------	*/

unsigned short int to_shortintconvertor(unsigned char array[]) {

    unsigned short int n;
    n = array[0];
    n <<= 8;
    n |= array[1];
    return n;
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  to_intconvertor1():                                	     		*/
/* Written by Gopal to convert just 1 byte to int                               */
/* ----------------------------------------------------------------------------	*/

unsigned int to_intconvertor1(unsigned char array[]) {

    unsigned int n;
    n = array[0];
    return n;
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  to_long_int_convertor():                                	     	*/
/* ----------------------------------------------------------------------------	*/

unsigned long int to_long_int_convertor(unsigned char array[]) {

    unsigned long int n;
    n = array[0];
    n <<= 8;
    n |= array[1];
    n <<= 8;
    n |= array[2];
    n <<= 8;
    n |= array[3];
    return n;

}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  to_long_int_convertor1():                                	     	*/
/* Written by Gopal to convert just 3 bytes to long int                         */
/* ----------------------------------------------------------------------------	*/

unsigned long int to_long_int_convertor1(unsigned char array[]) {

    unsigned int n;
    /*n = array[0];
      n <<= 8;
      n |= array[1];
      n <<= 8;
      n |= array[2];
      */


    n = array[0] <<16;
    n |= array[1] << 8;
    n |= array[2];

    return n;

}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION decode_ieee_single():                                	     	*/
/* ----------------------------------------------------------------------------	*/

float decode_ieee_single(const void *v) {

    const unsigned char *data = v;
    int s, e;
    unsigned long src;
    long f;
    float value;

    src = ((unsigned long)data[0] << 24) |
        ((unsigned long)data[1] << 16) |
        ((unsigned long)data[2] << 8) |
        ((unsigned long)data[3]);

    s = (src & 0x80000000UL) >> 31;
    e = (src & 0x7F800000UL) >> 23;
    f = (src & 0x007FFFFFUL);

    if (e == 255 && f != 0) {
        /* NaN (Not a Number) */
        value = DBL_MAX;

    } else if (e == 255 && f == 0 && s == 1) {
        /* Negative infinity */
        value = -DBL_MAX;
    } else if (e == 255 && f == 0 && s == 0) {
        /* Positive infinity */
        value = DBL_MAX;
    } else if (e > 0 && e < 255) {
        /* Normal number */
        f += 0x00800000UL;
        if (s) f = -f;
        value = ldexp(f, e - 150);
    } else if (e == 0 && f != 0) {
        /* Denormal number */
        if (s) f = -f;
        value = ldexp(f, -149);
    } else if (e == 0 && f == 0 && s == 1) {
        /* Negative zero */
        value = 0;
    } else if (e == 0 && f == 0 && s == 0) {
        /* Positive zero */
        value = 0;
    } else {
        /* Never happens */
        printf("s = %d, e = %d, f = %lu\n", s, e, f);
        assert(!"Woops, unhandled case in decode_ieee_single()");
    }

    return value;
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION copy_cbyc():                                	     		*/
/* ----------------------------------------------------------------------------	*/

void copy_cbyc(unsigned char dst[],unsigned char *s,int size) {

    int i;
    for(i = 0; i< size; i++)
        dst[i] = *(s + i);	
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION ncmp_cbyc():                                	     		*/
/* ----------------------------------------------------------------------------	*/

int ncmp_cbyc(unsigned char dst[],unsigned char src[],int size) {

    int i,flag = 0;
    for(i = 0; i< size; i++)	{

        if(dst[i] != src[i]) {

            flag = 1;
            break; 	
        }	
    }		
    return flag;
}


/*
   void dataCollectInBuffer(char *cmd, char *buffer, int type)
   {

   int err;
   if((BUFF_LEN - strlen(buffer)) < strlen(cmd)) {

   char *ttt;
   ttt = malloc((BUFF_LEN+1)*sizeof(unsigned char));
   memset(ttt,'\0',BUFF_LEN+1);
   strncpy(ttt,buffer,BUFF_LEN+1);
   memset(buffer,'\0',BUFF_LEN);
   strncpy(buffer,cmd,strlen(cmd));
   pthread_attr_t attr;
   pthread_attr_init(&attr);
   pthread_t thread1;


   if((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))) { // In  the detached state, the thread resources are
// immediately freed when it terminates, but 
perror(strerror(err));                                          // pthread_join(3) cannot be used to synchronize
exit(1);                                                        //  on the thread termination.         
}

//int er;
struct DataBuff *buf=malloc(sizeof(struct DataBuff));
buf->data = ttt;
buf->type = type;	
if((err = pthread_create(&thread1,&attr,dataWriteInFile,(void *)buf))) {

perror(strerror(err));
exit(1);
}										

} else {
strcat(buffer,cmd);
}
}
*/

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  dataWriteInFile():                                	     		*/
/* 								 		*/
/* ----------------------------------------------------------------------------	*/

/*void* dataWriteInFile(void * temp_buff) {

  FILE *p,*f,*a,*d,*e;

  struct DataBuff *buf = (struct DataBuff*)temp_buff;
  char *phasorFile = "PHASOR_MEASUREMENTS.txt";
  char *freqFile = "FREQUENCY_MEASUREMENTS.txt";
  char *analogFile = "ANALOG_MEASUREMENTS.txt";
  char *digitalFile = "DIGITAL_MEASUREMENTS.txt";
  char *frameDelay = "RECEIVED_FRAME_TIME.txt";

  if(buf->type == 1) {

  p = fopen(phasorFile,"w");
  fprintf(p,buf->data);
  fclose(p);

  f = fopen(freqFile,"w");
  fprintf(f,frequencyBuff);
  memset(frequencyBuff,'\0',BUFF_LEN);
  fclose(f);

  a = fopen(analogFile,"w");
  fprintf(a,analogBuff);
  memset(analogBuff,'\0',BUFF_LEN);
  fclose(a);

  d = fopen(digitalFile,"w");
  fprintf(d,digitalBuff);
  memset(digitalBuff,'\0',BUFF_LEN);
  fclose(d);

  e = fopen(frameDelay,"w");
  fprintf(e,delayBuff);
  memset(delayBuff,'\0',BUFF_LEN);
  fclose(e);

  } else if(buf->type == 2) {

  f = fopen(freqFile,"w");
  fprintf(f,buf->data);
  fclose(f);

  p = fopen(phasorFile,"w");
  fprintf(p,phasorBuff);
  memset(phasorBuff,'\0',BUFF_LEN);
  fclose(p);

  a = fopen(analogFile,"w");
  fprintf(a,analogBuff);
  memset(analogBuff,'\0',BUFF_LEN);
  fclose(a);

  d = fopen(digitalFile,"w");
  fprintf(d,digitalBuff);
  memset(digitalBuff,'\0',BUFF_LEN);
  fclose(d);

  e = fopen(frameDelay,"w");
  fprintf(e,delayBuff);
  memset(delayBuff,'\0',BUFF_LEN);
  fclose(e);

  } else if(buf->type == 3) {

  a = fopen(analogFile,"w");
  fprintf(a,buf->data);
  fclose(a);

  p = fopen(phasorFile,"w");
  fprintf(p,phasorBuff);
memset(phasorBuff,'\0',BUFF_LEN);
fclose(p);

f = fopen(freqFile,"w");
fprintf(f,frequencyBuff);
memset(frequencyBuff,'\0',BUFF_LEN);
fclose(f);

d = fopen(digitalFile,"w");
fprintf(d,digitalBuff);
memset(digitalBuff,'\0',BUFF_LEN);
fclose(d);

e = fopen(frameDelay,"w");
fprintf(e,delayBuff);
memset(delayBuff,'\0',BUFF_LEN);
fclose(e);

} else if(buf->type == 4) {

    d = fopen(digitalFile,"w");
    fprintf(d,buf->data);
    fclose(d);

    p = fopen(phasorFile,"w");
    fprintf(p,phasorBuff);
    memset(phasorBuff,'\0',BUFF_LEN);
    fclose(p);

    f = fopen(freqFile,"w");
    fprintf(f,frequencyBuff);
    memset(frequencyBuff,'\0',BUFF_LEN);
    fclose(f);

    a = fopen(analogFile,"w");
    fprintf(d,analogBuff);
    memset(analogBuff,'\0',BUFF_LEN);
    fclose(d);

    e = fopen(frameDelay,"w");
    fprintf(e,delayBuff);
    memset(delayBuff,'\0',BUFF_LEN);
    fclose(e);

} else if(buf->type == 5) {

    d = fopen(digitalFile,"w");
    fprintf(d,buf->data);
    fclose(d);

    p = fopen(phasorFile,"w");
    fprintf(p,phasorBuff);
    memset(phasorBuff,'\0',BUFF_LEN);
    fclose(p);

    f = fopen(freqFile,"w");
    fprintf(f,frequencyBuff);
    memset(frequencyBuff,'\0',BUFF_LEN);
    fclose(f);

    a = fopen(analogFile,"w");
    fprintf(d,analogBuff);
    memset(analogBuff,'\0',BUFF_LEN);
    fclose(d);

    e = fopen(frameDelay,"w");
    fprintf(e,delayBuff);
    memset(delayBuff,'\0',BUFF_LEN);
    fclose(e);
}

system("mysqlimport --local --fields-terminated-by=\, --fields-enclosed-by=\\\" -uroot -proot iPDC PHASOR_MEASUREMENTS.txt");
system("mysqlimport --local --fields-terminated-by=\, --fields-enclosed-by=\\\" -uroot -proot iPDC FREQUENCY_MEASUREMENTS.txt");
system("mysqlimport --local --fields-terminated-by=\, --fields-enclosed-by=\\\" -uroot -proot iPDC ANALOG_MEASUREMENTS.txt");
system("mysqlimport --local --fields-terminated-by=\, --fields-enclosed-by=\\\" -uroot -proot iPDC DIGITAL_MEASUREMENTS.txt");
system("mysqlimport --local --fields-terminated-by=\, --fields-enclosed-by=\\\" -uroot -proot iPDC RECEIVED_FRAME_TIME.txt");

printf("wrote to database\n");

free((unsigned char*)temp_buff);

pthread_exit(NULL); // Exit the thread once the task is done. 

}
*/

// New function coded by Kedar on 05-07-13 v1.4

void dataCollectInBuffer(char *cmd, struct BUFFER *buffer)
{

    int err;
    //printf("BINGO -- in dataCollectInBuffer \n");
    if((BUFF_LEN - buffer->curPos) < strlen(cmd)) {

        strncpy(phasorBuffBakupCopy,phasorBuff.buffer,BUFF_LEN);
        strncpy(frequencyBuffBakupCopy,frequencyBuff.buffer,BUFF_LEN);
        strncpy(digitalBuffBakupCopy,digitalBuff.buffer,BUFF_LEN);
        strncpy(analogBuffBakupCopy,analogBuff.buffer,BUFF_LEN);
        strncpy(delayBuffBakupCopy,delayBuff.buffer,BUFF_LEN);
        strncpy(statBuffBakupCopy,statBuff.buffer,BUFF_LEN);

        //memset(buffer.buffer,'\0',BUFF_LEN);

        memset(phasorBuff.buffer,'\0',BUFF_LEN);
        memset(frequencyBuff.buffer,'\0',BUFF_LEN);
        memset(analogBuff.buffer,'\0',BUFF_LEN);
        memset(digitalBuff.buffer,'\0',BUFF_LEN);
        memset(delayBuff.buffer,'\0',BUFF_LEN);
        memset(statBuff.buffer,'\0',BUFF_LEN);

        phasorBuff.curPos= 0;
        frequencyBuff.curPos = 0; 
        analogBuff.curPos = 0;
        digitalBuff.curPos = 0;
        delayBuff.curPos = 0;
        statBuff.curPos = 0;

        strncpy(buffer->buffer,cmd,strlen(cmd));
        buffer->curPos += strlen(cmd);

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_t thread1;


        if((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))) { // In  the detached state, the thread resources are
            // immediately freed when it terminates, but 
            perror(strerror(err));                                          // pthread_join(3) cannot be used to synchronize
            exit(1);                                                        //  on the thread termination.         
        }

        //int er;
        if((err = pthread_create(&thread1,&attr,dataWriteInFile,NULL))) {

            perror(strerror(err));
            exit(1);
        }										

    } else {
        //strcat(buffer,cmd);
        memcpy(buffer->buffer + buffer->curPos,cmd,strlen(cmd));
        buffer->curPos += strlen(cmd);
    }

}


void* dataWriteInFile() {

    FILE *p,*f,*a,*d,*e,*g;

    char *phasorFile = "PHASOR_MEASUREMENTS.txt";
    char *freqFile = "FREQUENCY_MEASUREMENTS.txt";
    char *analogFile = "ANALOG_MEASUREMENTS.txt";
    char *digitalFile = "DIGITAL_MEASUREMENTS.txt";
    char *frameDelay = "RECEIVED_FRAME_TIME.txt";
    char *statusFile = "STATUS_MEASUREMENTS.txt";

    //    printf("BINGO -- in dataWriteInFile\n");
    p = fopen(phasorFile,"w");
    fprintf(p,phasorBuffBakupCopy);
    memset(phasorBuffBakupCopy,'\0',BUFF_LEN);
    fclose(p);

    f = fopen(freqFile,"w");
    fprintf(f,frequencyBuffBakupCopy);
    memset(frequencyBuffBakupCopy,'\0',BUFF_LEN);
    fclose(f);

    a = fopen(analogFile,"w");
    fprintf(a,analogBuffBakupCopy);
    memset(analogBuffBakupCopy,'\0',BUFF_LEN);
    fclose(a);

    d = fopen(digitalFile,"w");
    fprintf(d,digitalBuffBakupCopy);
    memset(digitalBuffBakupCopy,'\0',BUFF_LEN);
    fclose(d);

    e = fopen(frameDelay,"w");
    fprintf(e,delayBuffBakupCopy);
    memset(delayBuffBakupCopy,'\0',BUFF_LEN);
    fclose(e);

    g = fopen(statusFile,"w");
    fprintf(e,statBuffBakupCopy);
    memset(statBuffBakupCopy,'\0',BUFF_LEN);
    fclose(g);
    /*
       system("mysqlimport --local --fields-terminated-by=\, --fields-enclosed-by=\\\" -uroot -proot iPDC PHASOR_MEASUREMENTS.txt");
       system("mysqlimport --local --fields-terminated-by=\, --fields-enclosed-by=\\\" -uroot -proot iPDC FREQUENCY_MEASUREMENTS.txt");
       system("mysqlimport --local --fields-terminated-by=\, --fields-enclosed-by=\\\" -uroot -proot iPDC ANALOG_MEASUREMENTS.txt");
       system("mysqlimport --local --fields-terminated-by=\, --fields-enclosed-by=\\\" -uroot -proot iPDC DIGITAL_MEASUREMENTS.txt");
       system("mysqlimport --local --fields-terminated-by=\, --fields-enclosed-by=\\\" -uroot -proot iPDC RECEIVED_FRAME_TIME.txt");
       system("mysqlimport --local --fields-terminated-by=\, --fields-enclosed-by=\\\" -uroot -proot iPDC STATUS_MEASUREMENTS.txt");
       */
    printf("wrote to database\n");
    pthread_exit(NULL); // Exit the thread once the task is done. 
}

/* Added by Gopal and Rajeev on 25th Feb 2014 */
void initfilestreams(unsigned long l_soc, unsigned long l_fracsec) 
{
    printf("Initializing file streams.\n");
    int j = 0;

    if(cfgfirst != NULL){
        struct tm *t1;
        t1 = localtime(&tStartArchive);
        char timetag[50];
        memset(timetag, '\0', 50);
        sprintf(timetag, "-%4i-%02i-%02i-%02i%s", 
                t1->tm_year+1900, t1->tm_mon+1, t1->tm_mday, t1->tm_hour, ".dat");
        

        for (j = 0;j<cfgfirst->num_pmu;j++){
            char *stn;
            stn = malloc(17);
            memset(stn,'\0',17);
            trimSpaces(stn, cfgfirst->pmu[j]->stn);
            char fileName[200];
            sprintf(fileName, "%04i_%s", cfgfirst->pmu[j]->idcode, stn);
            strcat(fileName, timetag);
            makecomtradecfg(fileName, stn, j, l_soc, l_fracsec);
            binData[j] = fopen(fileName, "w");
            srNo[j] = 0;
        }
    }
}

void closefilestreams()
{
    int j = 0;
    for (j = 0;j<cfgfirst->num_pmu;j++)
    {
        srNo[j] = 0;
        fclose(binData[j]);
    }
    system("$HOME/iPDC/bin/creatDirStructure.sh *.cfg");
    system("$HOME/iPDC/bin/creatDirStructure.sh *.dat");

    //    free(binData);
}

void makecomtradecfg(char *fileName, char *stn, int j, unsigned long l_soc, unsigned long l_fracsec)
{
    char cfgfileName[200];
    int i,k,TT,A,D;
    long int filepos;
    int An,a,b,skew,primary,secondary;
    float min,max;
    FILE *f;
//    char *chName, *ch_id, *ph, *ccbm, *uu, *PS;
    char chName[17], ch_id[129], ph[3], ccbm[2], uu[33], PS[2];
    /*
    chName = malloc(17);
    ch_id = malloc(129);
    ph = malloc(3);
    ccbm = malloc(2);
    uu = malloc(33);
    PS = malloc(2);
    */
    memset(chName, '\0', 17);
    memset(ch_id, '\0', 129);
    memset(ph, '\0',3);
    memset(ccbm, '\0',2);
    memset(uu,'\0',33);
    memset(PS,'\0',2);
    An = 0;

    k = strlen(fileName);
    memset(cfgfileName, '\0',200);
    strcat(cfgfileName,fileName);
    //strcpy(cfgfileName[k-4],".cfg");
    copy_cbyc (&cfgfileName[k-4],".cfg",4);
    
    cfgFileName[j] = malloc(200);
    memset(cfgFileName[j],'\0',200);
    strcpy(cfgFileName[j],cfgfileName);
    //printf("cfg file name = %s\n",cfgFileName[j]);

    f = fopen(cfgfileName, "w");
    fprintf(f,"iPDC_DBServer,PDC_%i,2013\n",cfgfirst->idcode);
//    filepos = ftell(f);
    /* collecting the analog channels, 2 per phasor, 1 per analog 
     * and 2 frequencies
     */
    A = cfgfirst->pmu[j]->phnmr*2 + cfgfirst->pmu[j]->annmr + 2; 
    /* collecting the digital channels, 16 channels per dgnmr
     * Plus there are 32 digital channels by default for time quality and 
     * status word */

    D = 32 + cfgfirst->pmu[j]->dgnmr*16;
    fprintf(f,"%-i,%-iA,%-iD\n",A+D,A,D);

    for (i=0; i<cfgfirst->pmu[j]->phnmr; i++)
    {
        An++;
        trimSpaces(chName, cfgfirst->pmu[j]->cnext->phnames[i]);
        sprintf(ch_id,"%s:%s",stn,chName);
        sprintf(ph,"%sm",cfgfirst->pmu[j]->cnext->phtypes[i]);
        strcpy(ccbm," ");
        if (~(strcmp(cfgfirst->pmu[j]->cnext->phtypes[i],"V"))) {
            strcpy(uu,"V");
        }
        else {
            strcpy(uu,"A");
        }
        int a = 1;
        int b = 0;
        int skew = 0;
        float min = -1*FLT_MAX;
        float max = FLT_MAX;
        int primary = 1;
        int secondary = 1;
        strcpy(PS,"P");
        // Printing for the magnitude line
        fprintf(f,"%i,%s,%s,%s,%s,%i,%i,%i,%g,%g,%i,%i,%s\n",
                An,ch_id,ph,ccbm,uu,a,b,skew,min,max,primary,secondary,PS);

        An++;
        sprintf(ph,"%sa",cfgfirst->pmu[j]->cnext->phtypes[i]);
        strcpy(uu,"rad");;
        min = -1*M_PI;
        max = M_PI;

        // Printing for the angle line
        fprintf(f,"%i,%s,%s,%s,%s,%i,%i,%i,%g,%g,%i,%i,%s\n",
                An,ch_id,ph,ccbm,uu,a,b,skew,min,max,primary,secondary,PS);

        memset(chName, '\0', 17);
        memset(ch_id, '\0', 129);
        memset(ph, '\0',3);
        memset(uu,'\0',33);
    }
    for (i=0; i<cfgfirst->pmu[j]->annmr; i++)
    {
        An++;
        trimSpaces(chName, cfgfirst->pmu[j]->cnext->angnames[i]);
        sprintf(ch_id,"%s:%s",stn,chName);
        strcpy(ph," ");
        strcpy(ccbm," ");
        sprintf(uu,"%s",cfgfirst->pmu[j]->cnext->antypes[i]);
        a = 1;
        b = 0;
        skew = 0;
        min = -1*FLT_MAX;
        max = FLT_MAX;
        primary = 1;
        secondary = 1;
        strcpy(PS,"P");
        // Printing for the analog line
        fprintf(f,"%i,%s,%s,%s,%s,%i,%i,%i,%g,%g,%i,%i,%s\n",
                An,ch_id,ph,ccbm,uu,a,b,skew,min,max,primary,secondary,PS);

        memset(chName, '\0', 17);
        memset(ch_id, '\0', 129);
        memset(ph, '\0',3);
        memset(uu,'\0',33);
    }
    // For frequency
    An++;
    sprintf(ch_id,"%s:Frequency",stn);
    strcpy(ph,"F");
    strcpy(ccbm," ");
    strcpy(uu,"Hz");
    a = 1;
    b = 0;
    skew = 0;
    min = -1*FLT_MAX;
    max = FLT_MAX;
    primary = 1;
    secondary = 1;
    strcpy(PS,"P");

    fprintf(f,"%i,%s,%s,%s,%s,%i,%i,%i,%g,%g,%i,%i,%s\n",
            An,ch_id,ph,ccbm,uu,a,b,skew,min,max,primary,secondary,PS);

    // For Df/dt
    An++;
    memset(ch_id, '\0', 129);
    sprintf(ch_id,"%s:df/dt",stn);
    strcpy(ph,"df");
    strcpy(ccbm," ");
    strcpy(uu,"Hz/s");

    fprintf(f,"%i,%s,%s,%s,%s,%i,%i,%i,%g,%g,%i,%i,%s\n",
            An,ch_id,ph,ccbm,uu,a,b,skew,min,max,primary,secondary,PS);

    // All analog channels done
    //
    // Now for digital channels
    //
    // Initial part is the constant part for time quality and status word
    int Dn = 1;
    fprintf(f,"%i,TQ_CNT0,T0, ,0\n",Dn);
    Dn++;
    fprintf(f,"%i,TQ_CNT1,T1, ,0\n",Dn);
    Dn++;
    fprintf(f,"%i,TQ_CNT2,T2, ,0\n",Dn);
    Dn++;
    fprintf(f,"%i,TQ_CNT3,T3, ,0\n",Dn);
    Dn++;
    fprintf(f,"%i,TQ_LSPND,T4, ,0\n",Dn);
    Dn++;
    fprintf(f,"%i,TQ_LSOCC,T5, ,0\n",Dn);
    Dn++;
    fprintf(f,"%i,TQ_LSDIR,T6, ,0\n",Dn);
    Dn++;
    fprintf(f,"%i,TQ_RSV,T7, ,0\n",Dn);
    Dn++;
    fprintf(f,"%i,RESV1,T8, ,0\n",Dn);
    Dn++;
    fprintf(f,"%i,RESV2,T9, ,0\n",Dn);
    Dn++;
    fprintf(f,"%i,RESV3,T10, ,0\n",Dn);
    Dn++;
    fprintf(f,"%i,RESV4,T11, ,0\n",Dn);
    Dn++;
    fprintf(f,"%i,RESV5,T12, ,0\n",Dn);
    Dn++;
    fprintf(f,"%i,RESV6,T13, ,0\n",Dn);
    Dn++;
    fprintf(f,"%i,RESV7,T14, ,0\n",Dn);
    Dn++;
    fprintf(f,"%i,RESV8,T15, ,0\n",Dn);
    Dn++;
    fprintf(f,"%i,%s_TRG1,S0, ,0\n",Dn,stn);
    Dn++;
    fprintf(f,"%i,%s_TRG2,S1, ,0\n",Dn,stn);
    Dn++;
    fprintf(f,"%i,%s_TRG3,S2, ,0\n",Dn,stn);
    Dn++;
    fprintf(f,"%i,%s_TRG4,S3, ,0\n",Dn,stn);
    Dn++;
    fprintf(f,"%i,%s_UNLK1,S4, ,0\n",Dn,stn);
    Dn++;
    fprintf(f,"%i,%s_UNLK2,S5, ,0\n",Dn,stn);
    Dn++;
    fprintf(f,"%i,%s_SEC1,S6, ,0\n",Dn,stn);
    Dn++;
    fprintf(f,"%i,%s_SEC2,S7, ,0\n",Dn,stn);
    Dn++;
    fprintf(f,"%i,%s_SEC3,S8, ,0\n",Dn,stn);
    Dn++;
    fprintf(f,"%i,%s_SEC4,S9, ,0\n",Dn,stn);
    Dn++;
    fprintf(f,"%i,%s_CFGCH,SA, ,0\n",Dn,stn);
    Dn++;
    fprintf(f,"%i,%s_PMUTR,SB, ,0\n",Dn,stn);
    Dn++;
    fprintf(f,"%i,%s_SORT,SC, ,0\n",Dn,stn);
    Dn++;
    fprintf(f,"%i,%s_SYNC,SD, ,0\n",Dn,stn);
    Dn++;
    fprintf(f,"%i,%s_PMUERR,SE, ,0\n",Dn,stn);
    Dn++;
    fprintf(f,"%i,%s_DTVLD,SF, ,0\n",Dn,stn);

    i = 0;
    unsigned long int dunit;
    int dgchannels;		
    struct dgnames* currentdgnames = cfgfirst->pmu[j]->cnext->first;
    while ( i < cfgfirst->pmu[j]->dgnmr) {
        dunit = to_long_int_convertor(cfgfirst->pmu[j]->dgunit[i]);
        // printf("Duint %u:%0x\n",dunit,dunit);
        int validChInt  = (dunit & 0xffff);
        dunit = dunit >> 16;
        int normalChInt = (dunit & 0xffff);

        //    memset(validCh[2], '\0', 1);
        for(dgchannels = 0;dgchannels < 16;dgchannels++){

            if (TAKE_N_BITS_FROM(validChInt,dgchannels,1)){ 
                memset(ch_id, '\0', 129);
                trimSpaces(chName, currentdgnames->dgn[dgchannels]);
                sprintf(ch_id,"%s:%s",stn,chName);
            }
            else {
                memset(ch_id, '\0', 129);
                sprintf(ch_id,"%s:%s",stn,"(UNUSED)");
            };
            Dn++;
            strcpy(ph, " ");
            strcpy(ccbm," ");
            int Y = TAKE_N_BITS_FROM(normalChInt,dgchannels,1);
            fprintf(f,"%i,%s,%s,%s,%i\n",Dn,ch_id,ph,ccbm,Y);
        }
        i++;  
        currentdgnames = currentdgnames->dg_next;
    } //DGWORD WHILE ENDS

    // Now the fixed parameters 
    // 
    // first LF
    fprintf(f,"%i\n",cfgfirst->pmu[j]->fnom);

    // nrates
    //fprintf(f,"1\n");
    fprintf(f,"0\n");

    // samp,endsamp
    //fprintf(f,"%i,%ld\n",cfgfirst->data_rate,90000);
    fprintf(f,"%i,",0);
    endSampPosition[j] = ftell(f);
    //printf("endSamp = %d\n",endSampPosition[j]);
    fprintf(f,"%5d\n",0);

    // timeStamps
    struct tm *t2;
    t2 = localtime(&l_soc);
    char timetag[50];
    memset(timetag, '\0', 50);
    sprintf(timetag, "%02i/%02i/%04i,%02i:%02i:%02i.%06ld", 
            t2->tm_mday,t2->tm_mon+1,t2->tm_year+1900,t2->tm_hour,t2->tm_min,
            t2->tm_sec, l_fracsec);
    fprintf(f,"%s\n",timetag);
    fprintf(f,"%s\n",timetag);

    // ft
    fprintf(f,"float32\n");

    // timemult
    long int timemult;
    timemult = 1e6/cfgfirst->data_rate;
    fprintf(f,"%ld\n",timemult);

    // time_code, local_code
    // as the time stamp is given in IST we have time_code and local_code same
    fprintf(f,"+5h30,+5h30\n");

    // tmq_code,leapsec
    fprintf(f,"0000,0\n");

    fclose(f);
}

void updatecomtradecfg(int j)
{
    FILE *f;
    f = fopen(cfgFileName[j], "r+");
    fseek(f,endSampPosition[j],SEEK_SET); 
    fprintf(f,"%5d",srNo[j]);
    fclose(f);
}

// Function added by Gopal on 26th Feb 2014
//
void trimSpaces(char *TO, char *FROM){
    int k,i;        
    for (k=0; k<=strlen(FROM); k++){
        if (~(isspace(FROM[k]))){
            break;
        }
    }
    strcpy(TO, &FROM[k]);
    for ( i=strlen(&FROM[k])-1; i>=0; --i){
        if (isspace(FROM[i])){
            TO[i] = '\0';
        }
        else{
            break;
        }
    }
    for ( i=0; i<strlen(TO); i++){
        if (isspace(TO[i])) {
            TO[i]='_';
        }
    }
}
/**************************************** End of File *******************************************************/
