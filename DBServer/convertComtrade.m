% ## Copyright (C) 2016 Gopal Gajjar
% ## 
% ## This program is free software; you can redistribute it and/or modify it
% ## under the terms of the GNU General Public License as published by
% ## the Free Software Foundation; either version 3 of the License, or
% ## (at your option) any later version.
% ## 
% ## This program is distributed in the hope that it will be useful,
% ## but WITHOUT ANY WARRANTY; without even the implied warranty of
% ## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% ## GNU General Public License for more details.
% ## 
% ## You should have received a copy of the GNU General Public License
% ## along with this program.  If not, see <http://www.gnu.org/licenses/>.
%
% ## -*- texinfo -*- 
% ## @deftypefn {Function File} {@var{retval} =} convertComtrade (@var{input1}, @var{input2})
% ##
% ## @seealso{}
% ## @end deftypefn
% 
% ## Author: Gopal Gajjar gopal@ee.iitb.ac.in
% ## Created: 2016-02-09
% ## Converts comtrage files produced by DBServer from float32 to binary format
% ## of comtrade. Required for compatibilty with old comtrade readers

function [] = convertComtrade (PathAndCfgName)

    if nargin == 0
        [CfgFileName,Path] = uigetfile('*.cfg');
        PathAndCfgName =[Path CfgFileName];
        DatFileName = strcat(CfgFileName(1:length(CfgFileName)-4), '.dat');
        PathAndDatName =[Path DatFileName];
    else
        PathAndDatName = strcat(PathAndCfgName(1:length(PathAndCfgName)-4), '.dat');
    end
    [Path, File, Ext] = fileparts(PathAndCfgName); 
    if isempty(Path)
        Path = '.';
    end
    [status, PathAndCfgNameNew] = system(['mktemp --tmpdir --suffix=X ', ...
        File,Ext,'XXX']);
    PathAndCfgNameNew(end) = [];
    [status, PathAndDatNameNew] = system(['mktemp --tmpdir --suffix=X ', ...
        File,'dat','XXX']);
    PathAndDatNameNew(end) = [];

    cfg_id = fopen(PathAndCfgName,'r');
    dat_id = fopen(PathAndDatName,'r');
    cfg_string = cell([1,50]);
    k=1;
    while 1
        temp_string = fgetl(cfg_id);
        if ~ischar(temp_string), break, end
        cfg_string{k} = flinescan(temp_string,',');
        k=k+1;
    end
    fclose(cfg_id);
    Ana_Ch = sscanf(char(cfg_string{1,2}(2)),'%d');
    Dig_Ch = sscanf(char(cfg_string{1,2}(3)),'%d');
    NR1 = Ana_Ch;
    NR2 = ceil(Dig_Ch/16);
    fseek(dat_id,0,-1); % place at start
    dat(1:2,:) = fread(dat_id,[2,inf],'2*uint32',4*NR1+2*NR2,'ieee-le');
    l = size(dat,2);
    fseek(dat_id,8,-1); % place at first analog data
    dat(3:2+NR1,1:l) = fread(dat_id,[NR1,l],[num2str(NR1) '*float32'], 8+2*NR2,'ieee-le');
    if NR2 ~= 0
        fseek(dat_id, 8+4*NR1,-1); % place at first digital data
        dat(3+NR1:2+NR1+NR2,1:l) = fread(dat_id,[NR2,l],[num2str(NR2)...
        '*uint16'], 8+4*NR1,'ieee-le');
    end
    fclose(dat_id);
    dat = dat';

    cfg_string{k-4} = {'BINARY'};
    for k = 3:Ana_Ch+2
        b = mean(dat(:,k));
        a = max(abs(dat(:,k)-b))/2^15;
        dat(:,k) = round((dat(:,k) -b)/a);
        cfg_string{1,k}{6} = num2str(a);
        cfg_string{1,k}{7} = num2str(b);
        cfg_string{1,k}{9} = num2str(-2^15);
        cfg_string{1,k}{10} = num2str(2^15 -1);
    end
    dat = dat';

    [dat_id,msg] = fopen(PathAndDatNameNew,'w');
    fseek(dat_id,0,-1);
    fwrite(dat_id,dat([1:2],1),'2*uint32',0,'ieee-le');
    fwrite(dat_id,dat([3:NR1+2],1),[num2str(NR1),'*int16'],0,'ieee-le');
    fwrite(dat_id,dat([2+NR1+1:end],1),[num2str(NR2),'*uint16'],0,'ieee-le');

    fseek(dat_id,8,-1);
    fwrite(dat_id,dat([1:2],2:end),'2*uint32',2*(NR1+NR2),'ieee-le');
    fseek(dat_id,8+2*NR1,-1);
    fwrite(dat_id,dat([3:NR1+2],2:end),[num2str(NR1),'*int16'],(2*NR2+8),'ieee-le');
    fseek(dat_id,8+2*(NR1+NR2),-1);
    fwrite(dat_id,dat([2+NR1+1:end],2:end),[num2str(NR2),'*uint16'],(2*NR1+8),'ieee-le');
    fclose(dat_id);

    cfg_id = fopen(PathAndCfgNameNew,'w');
    for k = 1:length(cfg_string);
        flineprint(cfg_id,cfg_string(1,k));
    end
    fclose(cfg_id);

    [status, output] = system(['mv ', PathAndDatNameNew, ' ', ...
    Path,'/',File,'_new.dat']);
    [status, output] = system(['mv ', PathAndCfgNameNew, ' ', ...
    Path, '/',File,'_new.cfg']);

function flineprint(FID,in)
    kk = length(in{1});
    for k = 1:(kk-1)
        fprintf(FID,'%s,', in{1}{k});
    end
    fprintf(FID,'%s\n',in{1}{kk});

function out = flinescan(rem,dlm)
    k = 1;
    while ~isempty(rem)
        [out{k},rem] = strtok_singleDlm(rem,dlm);
        k=k+1;
    end

%%%% Subfunction strtok_singleDlm
%%%% A specelization of the default strtok
function [token, remainder] = strtok_singleDlm(string,delimiters);
    len = length(string);
    i = 1;
    if(string(i) == delimiters)
        i = i+1;
    end
    start = i;
    while (~any(string(i) == delimiters))
        i = i + 1;
        if (i > len), break, end
    end
    finish = i - 1;
    token = string(start:finish);
    remainder = string(finish +1:len);
