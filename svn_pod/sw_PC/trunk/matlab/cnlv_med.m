function Z = cnlv_sht(z,h)

%  This function computes a truncated convolution of z with h.  The
%  truncation is such that the output sequence is shortned to effect zero
%  delay beween the original input function and the filtered function. 
%  The convention is that the shorter vector is the filter and the longer
%  is the data vector to be filtered.  The function implicitly assumes the
%  length of the filter is odd.  It is also implicitly assumed that the 
%  filter is linear phase. 

filter_len = min([length(h) length(z)]);

if(floor(filter_len / 2) * 2 == filter_len)
  input('WARNING: This function only valid for odd filter lengths');
end

Z = conv(z,h);

Z = Z((1 + (filter_len - 1)/2) : (length(Z) - (filter_len -1)/2));
