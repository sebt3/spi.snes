#!/bin/sh

grep SubTexture *xml|awk '{
	for(i=3;i<=7;i++){
		sub(/.*="/,"",$i);sub(/"/,"",$i)
	}
	sub(/\..*/,"",$3)
	if($3~/^grey/){$4+=512}
	if($3~/^yellow/){$5+=768}
	if($3~/^red/){$5+=512}
	if($3~/^green/){$5+=256}
	print $3" "$4":"$5" "($4+$6)":"($5+$7)}'
