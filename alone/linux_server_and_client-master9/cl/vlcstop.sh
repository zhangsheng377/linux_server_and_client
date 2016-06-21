#!/bin/bash
proc_name="vlc"
proc_id=`ps ax|grep ${proc_name}|awk '{print $1}'`
if [[-z $proc_id]];then
	echo "The task is not running!"
else
	echo ${proc_name}" pid:"
	echo ${proc_id[@]}
	for id in ${proc_id[*]}
	do
		echo ${id}
		kill -9 ${id}
		if [$? -eq 0];then
			echo "task is killed"
		else
			echo "kill task failed"
		fi
	done
fi
