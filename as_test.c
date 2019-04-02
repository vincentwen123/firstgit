#include "arinc429_api.h"
#include "cpld_api.h"
#include "fsl_types.h"
#include "fsl_types.h"
#include "rs485_api.h"
#include "rs422_api.h"
#include "arinc717_api.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <pthread.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <stddef.h>
#include <sys/prctl.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <mqueue.h>


#define VERSION 3
#define SEC 1000000
#define ERROR_429 0x0fffff86
#define NONE 0x10000086
#define SUCCESS 0x00000086
#define N 1
#define PIDLOG "./pidlog/test.pid"
#define PIDIR "./pidlog"
#define LOGFILE "./log"
#define FMT_DATE "%Y-%m-%d %H:%M:%S"
#define SPEED 115200
#define DATA 0xfff

#define PRINT(fmt, args...) \
    do { \
        fprintf(stdout, fmt, ##args); \
        if (log_fp) { \
            fprintf(log_fp, fmt, ##args); \
            fflush(log_fp);\
        } \
    }while(0)

static int start_all_test=0;
static int stop_test=0;
static FILE* log_fp;
static time_t now;
static struct tm tm;
static char tmbuf[32]; 
static int semid;
static union semun{
	int val;
	struct semid_ds* array;
	struct seminfo* buf;
}semun;
static struct sembuf sembuf;

void usage(){
    fprintf(stdout,
        "Usage: as_test [-a47SsUdcV]\n"
        "   [-a] - Enable all test\n"
        "   [-4] - Enable ARINC 429 test\n"
        "   [-7] - Enable ARINC 717 test\n"
        "   [-s] - Enable ssd test\n"
        "   [-U] - Enable usb3.0 test\n"
        "   [-u] - Enable usb2.0 test\n"
        "   [-d] - Enable dist test\n"
        "   [-S] - Stop test\n"
        "   [-c] - Clean test files\n"
        "   [-V] - Show version\n"
        "   [none or wrong] - Print usage()\n");
}

void time_log(){
	now=time(NULL);
	gmtime_r(&now,&tm);
	strftime(tmbuf,sizeof(tmbuf)-1,FMT_DATE,&tm);
	fprintf(log_fp, "%s\n",tmbuf );
}

void* bao429(void* arg){
	cpld_init();
    cpld_write_reg(0x10,0x0);
	arinc429_init();
	uint32_t regval=0;      
    //regval=cpld_read_reg(0x10);
    //printf("regval=%d\n",regval);
	cpld_write_reg(0x10,0x80);
	//regval=cpld_read_reg(0x10);
	//printf("regval=%d\n",regval);

	int n1;
	int n2;
	int n3;
	int n0;
	time_log();
	PRINT("start 429 test\n");
	//create mqueue
	/*
	mqd_t mqid;
	struct mq_attr attr;
	uint32_t *bufin0;
	uint32_t *bufin1;
	uint32_t *bufin2;
	uint32_t *bufin3;
	uint32_t *bufin4;
	uint32_t *bufin5;
	uint32_t *bufin6;
	uint32_t *bufin7;
	memset(&attr,0,sizeof(attr));
	attr.mq_msgsize= sizeof(uint32_t);
	attr.mq_maxmsg=10;
	mqid=mq_open("/mq_429_01",O_CREAT|O_RDWR,0666,&attr);
	if(mqid<0) PRINT("error in mq");
	*/

	while(1)
	{
		//rx_channel 01 = tx_channel 0		
		uint32_t txCh01_data;
		uint32_t* rxp_ch0=(uint32_t*)malloc(N*sizeof(txCh01_data));
		uint32_t* rxp_ch1=(uint32_t*)malloc(N*sizeof(txCh01_data));
		if (rxp_ch0==NULL||rxp_ch1==NULL)
		{
			perror("malloc wrong");
			exit(0);
		}

		*rxp_ch0=arinc429_rx_chn_data(0);
		*rxp_ch1=arinc429_rx_chn_data(1);
		//printf("rxp_ch0:%x\n",rxp_ch0 );
		//printf("rxp_ch1:%x\n",rxp_ch1 );
		/*bufin0=arinc429_rx_chn_data(0);
		bufin1=arinc429_rx_chn_data(1);
		mq_receive(mqid,(char*)bufin0,sizeof(uint32_t),NULL);
		mq_send(mqid,(char*)rxp_ch0,sizeof(uint32_t),0);
		mq_receive(mqid,(char*)bufin1,sizeof(uint32_t),NULL);
		mq_send(mqid,(char*)rxp_ch1,sizeof(uint32_t),0);*/

		if(arinc429_rx_chn_empty(0)) PRINT("none of chn0\n");
		if(arinc429_rx_chn_empty(1)) PRINT("none of chn1\n");
		if(*rxp_ch0==*rxp_ch1 && *rxp_ch1!=NONE)
		{
			txCh01_data=arinc429_tx_chn_data(0,*rxp_ch0);
			n0=0;
		}
		else 
		{
			n0++;
			if(n0>=2)
			{
				n0=0;
				arinc429_tx_chn_data(0,ERROR_429);
				PRINT("ch01_429 wrong\n");
			}
		}
		free(rxp_ch0);
		rxp_ch0=NULL;
		free(rxp_ch1);
		rxp_ch1=NULL;
		//rx_channel 23 = tx_channel 1
		uint32_t txCh23_data;
		uint32_t* rxp_ch2=(uint32_t*)malloc(N*sizeof(txCh01_data));
		uint32_t* rxp_ch3=(uint32_t*)malloc(N*sizeof(txCh01_data));
		if (rxp_ch2==NULL||rxp_ch3==NULL)
		{
			perror("malloc wrong");
			exit(0);
		}
		*rxp_ch2=arinc429_rx_chn_data(2);
		*rxp_ch3=arinc429_rx_chn_data(3);
		if(arinc429_rx_chn_empty(2)) PRINT("none of chn2\n");
		if(arinc429_rx_chn_empty(3)) PRINT("none of chn3\n");
		if(*rxp_ch2==*rxp_ch3 && *rxp_ch2!=NONE)
		{
			txCh23_data=arinc429_tx_chn_data(1,*rxp_ch2);
			n1=0;
		}
		else 
		{
			n1++;
			if(n1==2)
			{
				n1=0;
				arinc429_tx_chn_data(1,ERROR_429);
				//PRINT("ch23_429 wrong\n");
			}
		}
		free(rxp_ch2);
		rxp_ch2=NULL;
		free(rxp_ch3);
		rxp_ch3=NULL;
		//rx_channel 45 = tx_channel 2
		uint32_t txCh45_data;
		uint32_t* rxp_ch4=(uint32_t*)malloc(N*sizeof(txCh01_data));
		uint32_t* rxp_ch5=(uint32_t*)malloc(N*sizeof(txCh01_data));
		if (rxp_ch4==NULL||rxp_ch5==NULL)
		{
			perror("malloc wrong");
			exit(0);
		}
		*rxp_ch4=arinc429_rx_chn_data(4);
		*rxp_ch5=arinc429_rx_chn_data(5);
		if(arinc429_rx_chn_empty(4)) PRINT("none of chn4\n");
		if(arinc429_rx_chn_empty(5)) PRINT("none of chn5\n");
		if(*rxp_ch4==*rxp_ch5 && *rxp_ch4!=NONE)
		{
			txCh45_data=arinc429_tx_chn_data(2,*rxp_ch4);
			n2=0;
		}
		else 
		{
			n2++;
			if(n2==2)
			{
				n2=0;
				arinc429_tx_chn_data(2,ERROR_429);
				//PRINT("ch45_429 wrong\n");
			}
		}
		free(rxp_ch4);
		rxp_ch4=NULL;
		free(rxp_ch5);
		rxp_ch5=NULL;
		//rx_channel 67 = tx_channel 3
		uint32_t txCh67_data;
		uint32_t* rxp_ch6=(uint32_t*)malloc(N*sizeof(txCh01_data));
		uint32_t* rxp_ch7=(uint32_t*)malloc(N*sizeof(txCh01_data));
		if (rxp_ch6==NULL||rxp_ch7==NULL)
		{
			perror("malloc wrong");
			exit(0);
		}
		*rxp_ch6=arinc429_rx_chn_data(6);

		*rxp_ch7=arinc429_rx_chn_data(7);
		if(arinc429_rx_chn_empty(6)) PRINT("none of chn6\n");
		if(arinc429_rx_chn_empty(7)) PRINT("none of chn7\n");
		if(*rxp_ch6==*rxp_ch7 && *rxp_ch6!=NONE)
		{
			txCh67_data=arinc429_tx_chn_data(3,*rxp_ch6);
			n3=0;
		}
		else 
		{
			n3++;
			if(n3==2)
			{
				n3=0;
				arinc429_tx_chn_data(3,ERROR_429);
				//PRINT("ch67_429 wrong\n");
			}
		}
		free(rxp_ch6);
		rxp_ch6=NULL;
		free(rxp_ch7);
		rxp_ch7=NULL;
		usleep(SEC);
	}
}

void* t717(void* arg)
{

	uint16_t rdata;
	uint16_t data;
	static int errn=0;
	static int errw=0;
	static uint16_t msgid=0;
	arinc717_chip_init();
	while(1)
	{
		//rx
		//after 5s fifoempty=fail
		//printf("emp:%d\n",arinc717_rx_fifo_empty());		
		if (arinc717_rx_fifo_empty())
		{
			errn++;
			if(errn==5) 
			{
			PRINT("none of 717 data\n");
			errn=0;
			}
			sleep(1);
		}
		else
		{
			rdata=arinc717_rx_fifo_data();
			//printf("rdata:%x",rdata);
			if (rdata!=DATA && rdata!=0x0247 && rdata!=0x05b8 && rdata!=0x0db8 && rdata!=0x0a47)
			{
				errw++;
				if(errw==20) PRINT("717-rx-wrong\n");
			}
			else
			{
				errw=0;
				errn=0;
			}
		}
		//tx
		switch(msgid)
		{
		  case 0:
		    data=0x0247;
		    break;
		  case 64:
		    data=0x05b8;
		    break;
		  case 128:
		    data=0x0a47;
		    break;
		  case 192:
		    data=0x0db8;
		    break;
		  default:
		    data=0xffff;
		    break;
		}
       	 	
		if(0!=arinc717_tx_fifo_data(data))
		{
			PRINT("717-tx-wrong\n");
		}
		msgid=(msgid+1)%256;
	}
}

void* storagetest(void* arg){
	FILE* fp;
	char buf_r[128];
	char buf_w[128];
	time_log();
	fprintf(log_fp, "start ssd test\n");
	memset(buf_r,0,sizeof(buf_r));
	memset(buf_w,0,sizeof(buf_w));
	fp=popen("sh ssd_test.sh","r");
	if (fp==NULL)
	{
		/* code */
		printf("fail to popen\n");
		return;
	}

	while(1){
		fgets(buf_r,sizeof(buf_r),fp);
		printf("%s",buf_r );
	}

	pclose(fp);
}

void* storagetest_ssd1(void* arg){
	FILE* fp;
	char buf_r[256];
	char buf_422[256];
	time_log();
	PRINT("start ssd1 test\n");
	memset(buf_r,0,sizeof(buf_r));
	memset(buf_422,0,sizeof(buf_422));
	rs422_initialize(SPEED, 1);
	fp=popen("sh /run/media/public/a9fd3382-d33c-4517-bc89-f50d4f9449e6/ssd_test1.sh","r");
	if (fp==NULL)
	{
		/* code */
		printf("fail to popen\n");
		return;
	}
	for(;;)
	{
		if(semop_p()==0) printf("semop_p wrong\n");
		fgets(buf_r,sizeof(buf_r),fp);
		printf("%s",buf_r );
		int len=sprintf(buf_422,"%s\r\n",buf_r);
		//printf("422:%s",buf_422 );
		rs422_send_data(buf_422,len);
		fflush(stdout);
		if(semop_v()==0) printf("semop_v wrong\n");
	}
	pclose(fp);
}

void* storagetest_ssd2(void* arg){
	FILE* fp;
	char buf_r[256];
	char buf_422[256];
	time_log();
	PRINT("start ssd2 test\n");
	memset(buf_r,0,sizeof(buf_r));
	memset(buf_422,0,sizeof(buf_422));
	rs422_initialize(SPEED, 1);
	fp=popen("sh /run/media/public/91de1017-34e8-4ed7-a1fd-caabb9a8fa22/ssd_test2.sh","r");
	if (fp==NULL)
	{
		/* code */
		printf("fail to popen\n");
		return;
	}
	for(;;)
	{
		if(semop_p()==0) printf("semop_p wrong\n");
		fgets(buf_r,sizeof(buf_r),fp);
		printf("%s",buf_r );
		int len=sprintf(buf_422,"%s\r\n",buf_r);
		//printf("422:%s",buf_422 );
		rs422_send_data(buf_422,len);
		fflush(stdout);
		if(semop_v()==0) printf("semop_v wrong\n");
	}
	pclose(fp);
}

void* distest(void* arg){
	uint32_t bufin[21];
	static int i;
	static int j = 0;
	static int p = 0;
	static int err=0;
	static uint32_t state0=0;
	static uint32_t state1=1;
	static char bufin_485[128];
	time_log();
	PRINT("start dist test\n");
	cpld_init();
	rs485_initialize(SPEED, 1);
	while(1)
	{
		sleep(1);
		for (i = 0; i < 21; ++i)
		{
			/* code */
			if(0!=ReadDI((uint32_t)i,bufin+i))
			{
				PRINT("error read chn :%d\n", i);
			}	
		}
		if(semop_p()==0) printf("semop_p wrong\n");
		printf("bufin=%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d\n", bufin[20],bufin[19],\
			bufin[18],bufin[17],bufin[16],bufin[15],bufin[14],bufin[13],bufin[12],bufin[11],\
			bufin[10],bufin[9],bufin[8],bufin[7],bufin[6],bufin[5],bufin[4],bufin[3],\
			bufin[2],bufin[1],bufin[0]);
		if(semop_v()==0) printf("semop_v wrong\n");
		int len=sprintf(bufin_485,"bufin=%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d\r\n", bufin[20],bufin[19],\
			bufin[18],bufin[17],bufin[16],bufin[15],bufin[14],bufin[13],bufin[12],bufin[11],\
			bufin[10],bufin[9],bufin[8],bufin[7],bufin[6],bufin[5],bufin[4],bufin[3],\
			bufin[2],bufin[1],bufin[0]);
		rs485_send_data(bufin_485,len);
		//sprintf(buf,"")
		/*for ( p = 0; p < 21; ++p)
		{
			if(bufin[p]!=bufin[p+1]) 
			{
				err=1;
				//printf("error in : %d\n",p);
			}
		}
		if(err==0) PRINT("dist_read success\n");*/
		for (j = 0; j < 11; ++j)
		{
			if ((rand()%2)==1)
			{
				if(0!=WriteDO((uint32_t)j,state1)) PRINT("error write chn :%d\n", j);
			}
			else
			{
				if(0!=WriteDO((uint32_t)j,state0)) PRINT("error write chn :%d\n", j);
			}
		}		
	}
}

int create_pid(){
	struct flock lock;
	pid_t pid;
	char buf[32];
	int len;

	mkdir(PIDIR,0766);
	int fd=open(PIDLOG,O_CREAT|O_RDWR,0766);
	if (fd<0)
	{
		perror("open wrong");
		return -1;
	}	
	memset(&lock,0,sizeof(lock));
	lock.l_type=F_WRLCK;
	lock.l_whence=SEEK_SET;
	int ret=fcntl(fd,F_SETLK,&lock);
	if (ret<0)
	{
		perror("do not open test again or create:fcntl wrong");
		close(fd);
		return -1;
	}
	ftruncate(fd,0);
	lseek(fd,0,SEEK_SET);
	pid=getpid();
	len=snprintf(buf,32,"%d",(int)pid);
	write(fd,buf,len);
	memset(&lock,0,sizeof(lock));
	lock.l_type=F_UNLCK;
	lock.l_whence=SEEK_SET;
	if(fcntl(fd,F_SETLK,&lock)<0){
		perror("unlock fail");
		close(fd);
		return -1;
	}
	PRINT("created pidlog.pid\n");
	close(fd);
	return 0;
}

void* lock_pid(){
	struct flock lock;
	int fd=open(PIDLOG,O_RDWR);
	if (fd<0)
	{
		printf("lock_pid->open wrong\n");
		pthread_exit(0);
	}
	memset(&lock,0,sizeof(lock));
	lock.l_type=F_RDLCK;
	lock.l_whence=SEEK_SET;
	if (fcntl(fd,F_SETLK,&lock)<0)
	{
		perror("lock_pid->fcntl wrong");
		pthread_exit(0);
	}
	while (1);
}

void* syshell(char* cmd){
	pid_t status;
	status = system(cmd);
	//printf("%s\n",cmd );
	if (-1 == status) printf("system error!");
	else
	{
		printf("exit status value = [0x%x]\n", status);
		if (WIFEXITED(status))
		{
			if (0 == WEXITSTATUS(status)) printf("run shell script successfully.\n");
			else printf("run shell script fail, script exit code: %d\n", WEXITSTATUS(status));
		}
		else printf("exit status = [%d]\n", WEXITSTATUS(status));
	}
}

void kill_test(){
	//char buf[64];
	char cmd[128];
	FILE* fp;
	int pid;
	fp=fopen(PIDLOG,"r");
	if(fp==NULL){
		printf("kill_test->fopen wrong\n");
		exit(0);
	}
	fscanf(fp,"%d",&pid);
	fclose(fp);
	printf("pid=%d\n",pid );
	sprintf(cmd,"kill -9 %d",pid);
	syshell(cmd);
}

void signal_set(){
	stop_test=1;
}

int semop_p(){
	memset(&sembuf,0,sizeof(sembuf));
	sembuf.sem_op=-1;
	sembuf.sem_flg=SEM_UNDO;
	//sembuf.semnum=0;
	if (semop(semid,&sembuf,1)==-1)
	{
		perror("semop_p wrong");
		return 0;
	}
	return 1;	
}

int semop_v(){
	memset(&sembuf,0,sizeof(sembuf));
	sembuf.sem_op=1;
	sembuf.sem_flg=SEM_UNDO;
	//sembuf.sem_num=0;
	if (semop(semid,&sembuf,1)==-1)
	{
		error("semop_v wrong");
		return 0;
	}
	return 1;
}

int main(int argc, char *argv[])
{
	
	int p,i;
	int td[]={0,0,0,0,0,0};
	int version=VERSION;
	pthread_t tid[10];
	void * arg;

	if (argc == 1)
    {
        usage();
        exit(0);
    }
    log_fp=fopen(LOGFILE,"a+");
	if (log_fp==NULL) printf("can not create log\n");
	if(log_fp!=NULL) PRINT("create log successfully\n");
	//sem control
	semid=semget((key_t)1111,1,0766|IPC_CREAT);
	semun.val=1;
	if (semctl(semid,0,SETVAL,semun)<0) perror("semctl-setval wrong");

	while ((p = getopt(argc, argv, "a47SUusdcV")) != -1) 
    {
		switch(p)
        {
        case 'a':
            start_all_test = 1;
            break;
        case '4':
            bao429(arg);
            break;
        case '7':
            t717(arg);
            break;
        case 's':
            storagetest(arg);
            break;
        case 'U':
            storagetest(arg);
            break;
        case 'u':
           storagetest(arg);
            break;
        case 'd':
            distest(arg);
            break;
        case 'V':
            printf("AS_TEST Version : %d\n",version );
            break;
        case 'S':
            stop_test = 1;
            break;   
        case '?':
            fprintf(stdout, "AS_TEST Version : %d\n", version);
            usage();
            exit(0);
		default:
            break;
        }
	}

	
	//if (daemon(1,1)<0) PRINT("daemon wrong\n");
	if(create_pid()==0)
	{
		printf("create_pid wrong\n");
		pthread_create(&tid[4],NULL,lock_pid,NULL);
	}
	if (start_all_test)
	{
		if((pthread_create(&tid[0],NULL,bao429,NULL))!=0) perror("start 429-pthread wrong");
		if((pthread_create(&tid[1],NULL,distest,NULL))!=0) perror("start dist-pthread wrong");
		if((pthread_create(&tid[2],NULL,storagetest_ssd1,NULL))!=0) perror("start storage-pthread wrong");
		if((pthread_create(&tid[3],NULL,storagetest_ssd2,NULL))!=0) perror("start storage-pthread wrong");
		if((pthread_create(&tid[5],NULL,t717,NULL))!=0) perror("start 717-pthread wrong");
	}



	if (stop_test)
	{
		if ((tid[0]!=0))
		{
			if(!pthread_cancel(tid[0])) printf("cancel pthread wrong\n");
		}
		if ((tid[1]!=0))
		{
			if(!pthread_cancel(tid[1])) printf("cancel pthread wrong\n");
		}
		if ((tid[2]!=0))
		{
			if(!pthread_cancel(tid[2])) printf("cancel pthread wrong\n");
		}
		if ((tid[3]!=0))
		{
			if(!pthread_cancel(tid[3])) printf("cancel pthread wrong\n");
		}
		if ((tid[4]!=0))
		{
			if(!pthread_cancel(tid[4])) printf("cancel pthread wrong\n");
		}
		if ((tid[5]!=0))
		{
			if(!pthread_cancel(tid[5])) printf("cancel pthread wrong\n");
		}
		if(semctl(semid,0,IPC_RMID)<0) perror("ipc_rmid wrong");

		sleep(10);
		kill_test();
		return 0;
	}
	
	pthread_join(tid[4],NULL);
	pthread_join(tid[0],NULL);
	pthread_join(tid[1],NULL);
	pthread_join(tid[2],NULL);
	pthread_join(tid[3],NULL);	
	return 0;	
}
