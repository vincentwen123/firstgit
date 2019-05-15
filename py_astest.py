import os,sys,shutil,filecmp,time,gevent,ctypes
from ctypes import *



def ssd_shell_1():
	i=0
	fmt="%Y-%m-%d %X"
	tpath="./ssd_test2"
	spath="./ssd_test1"
	sfile=spath+"/tmp.file1"
	tfile=tpath+"/tmp.file_"
	logpath=spath+"/ssd_log"
	try:
		os.mkdir(spath)
		os.mkdir(tpath)
	except FileExistsError as e:
		print(e)
	with open(sfile,"w") as f:
		pass
	os.system("dd if=/dev/urandom of=./ssd_test1/tmp.file1 bs=1M count=10 >/dev/null 2>&1")

	while True:
		time.sleep(0.5)#change to gevent.sleep(0.5)
		ttfile=tfile+str(i)
		shutil.copy(sfile,ttfile)
		i+=1
		if filecmp.cmp(sfile,ttfile,False):
			print(time.strftime(fmt)+" ssd success")
			with open(logpath,"a") as f:
				f.write(time.strftime(fmt)+" ssd normal\n")
				f.flush()
		else:
			print(time.strftime(fmt)+"ssd wrong")
			with open(logpath,"a") as f:
				f.write(time.strftime(fmt)+"ssd wrong\n")
				f.flush()




