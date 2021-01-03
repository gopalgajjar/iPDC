## Copyright (C) 2017 Gopal Gajjar
## 
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
## 
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

## -*- texinfo -*- 
## @deftypefn {Function File} {@var{retval} =} updateFile (@var{input1}, @var{input2})
##
## @seealso{}
## @end deftypefn

## Author: Gopal Gajjar gopal@ee.iitb.ac.in
## Created: 2017-10-26

function [] = updateFile (input1)
    data = csvread(input1);
%     data(:,3:2:end-2) = data(:,3:2:end-2)*pi/180;
%     csvwrite (input1 ,data, "precision", "%16.4f");
    data1 = data(:,2:7);
    feeders = round((size(data,2)-9)/6);
    data2 = [data(:,1:end-2), repmat(data1,1,feeders), data(:,end-1:end)];
    outfile = [input1(1:end-4), "_withLineVoltage", input1(end-3:end)];
    csvwrite (outfile ,data2, "precision", "%12.4f");
endfunction
