import os
import time
os.mkdir("testlog")
cmd="sh ssdtest.sh"
fmt="%Y-%m-%d %X"
ret=os.system(cmd)
if ret==0:
	pass
else:
	print(time.strftime(fmt)+" ssd test wrong")
	with open("./testlog/log","a") as f:
		f.write(time.strftime(fmt)+" ssd test wrong\n")
		f.flush()

