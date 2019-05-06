import os
cmd="sh ssdtest.sh"
ret=os.system(cmd)
if ret==0:
	pass
else:
	print("ssd test wrong")