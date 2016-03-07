#include <string.h>
//#include <hardware/gps.h>
//#include <hardware/hardware.h>
#include "hardware/gps.h"
#include "hardware/hardware.h"
#include "hardware_legacy/power.h"
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <utils/misc.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define NMEA_MAX_SIZE 83
#define maxlen 1024
#define gpstype 1
#define port 6789
#define WAKE_LOCK_NAME  "GPS"
#define hour 3600
#define offset 1000

static const GpsInterface* sGpsInterface = NULL;
//static GpsInterface* sGpsInterface = NULL;
static const GpsNiInterface* sGpsNiInterface = NULL;

//GpsCallbacks sGpsCallbacks;
static GpsLocation sGpsLocation;
static GpsStatus sGpsStatus;
static GpsUtcTime sGpsUtcTime;
//char snmea[NMEA_MAX_SIZE+1];
static GpsSvStatus  sGpsSvStatus;
static const char* sNmeaString;
//static char* sNmeaString;
static int sNmeaStringLength;

static void location_callback(GpsLocation* location)
{
    //printf("in location_callback(printf)\n");
    ALOGD("the location_callback(LOGD)\n");
    //D("addGpsLocation");
    //printf("In callback:Gpslocation->speed=%f\nIncallback:GpsLocation->accuracy=%f\n",location->speed,location->accuracy);
    memcpy(&sGpsLocation, location, sizeof(sGpsLocation));
}

static void status_callback(GpsStatus* status)
{
    memcpy(&sGpsStatus, status, sizeof(sGpsStatus));
}

static void sv_status_callback(GpsSvStatus* sv_status)
{
    memcpy(&sGpsSvStatus, sv_status, sizeof(sGpsSvStatus));
}

static void nmea_callback(GpsUtcTime timestamp, const char* nmea, int length)
{
    // The Java code will call back to read these values
    // We do this to avoid creating unnecessary String objects
    sNmeaString = nmea;
    sNmeaStringLength = length;
    sGpsUtcTime = timestamp;
}

static void set_capabilities_callback(uint32_t capabilities)
{
    ALOGD("set_capabilities_callback: %d\n", capabilities);
}

static void acquire_wakelock_callback()
{
    acquire_wake_lock(PARTIAL_WAKE_LOCK, WAKE_LOCK_NAME);
}

static void release_wakelock_callback()
{
    release_wake_lock(WAKE_LOCK_NAME);
}

static void request_utc_time_callback()
{
    //printf("request_utc_time_callback\n");
    ALOGD("request_utc_time_callback\n");
}

typedef void (*wgstest)(void * end);

typedef struct {
    //conversion  void (*)(void *)  to  (void*)(*)(void *)
    wgstest start;
    void * arg;
} Conversion;

//static Conversion * sConversion;

void* start_rtn(void* arg)
{
    //printf("this is the void * return function\n");
    Conversion * test = (Conversion *)malloc(sizeof(Conversion));
    test = (Conversion *)arg;
    test->start(test->arg);
    //printf("conversion function pointer\n");
    return NULL;
}


static pthread_t create_thread_callback(const char* name, void (*start)(void *), void* arg)
{
    pthread_t tid;
    Conversion * sConversion;
    sConversion = (Conversion *)malloc(sizeof(Conversion));
    sConversion->start=start;
    sConversion->arg=arg;

    int err = pthread_create(&tid, NULL, start_rtn , sConversion);
    if (err != 0)
        printf("can't create thread: %s\n", strerror(err));
    return tid;
}

GpsCallbacks sGpsCallbacks = {
    sizeof(GpsCallbacks),
    location_callback,
    status_callback,
    sv_status_callback,
    nmea_callback,
    set_capabilities_callback,
    acquire_wakelock_callback,
    release_wakelock_callback,
    create_thread_callback,
    request_utc_time_callback,
};

long long wgs_atoi(char * str)
{
    long long ret=0;
    while((*str)!='\0')
    {
        ret=((*str)-'0') + ret*10;
        str++;  
    }
    return ret;
}

int main(int argc, char** argv)
{
	//mytestgps -o/-g num stop
    	int err,i,flag;
	long long swaptime;
	long long tmptime;
	long long starttime;
	//long long endtime;
	FILE * fp1;
	time_t timer,timer1;

	hw_module_t* module;

    	err = hw_get_module(GPS_HARDWARE_MODULE_ID, (hw_module_t const**)&module);//get the hardware api
    	if (err == 0) {
		hw_device_t* device;
        	err = module->methods->open(module, GPS_HARDWARE_MODULE_ID, &device);
        	if (err == 0) {
            		gps_device_t* gps_device = (gps_device_t *)device;
            		sGpsInterface = gps_device->get_gps_interface(gps_device); //open the gps
        	}
    	}
    	//printf("just-get_gps_interface-wgs\n");
    	//sleep(60);

    	if (!sGpsInterface || sGpsInterface->init(&sGpsCallbacks) != 0) {
        	printf("fail if the main interface fails to initialize\n");
        	return false;
    	}
    	//printf("just gpsinterfate->init-wgs\n");
    	//printf("wgs-look GpsLocation data:\nlatitude->%lf\n",sGpsLocation.latitude);
    	//printf("this is nmea data->%s\n",sNmeaString);
    	//sleep(60);

    	if (sGpsInterface) {
        	//printf("gps will start!\n");
        	if(sGpsInterface->start() < 0) {
           		printf("gps not start,flase\n");
           		return false;
        	}
    	}
   	else
	{
		return false;
	}
    	//printf("just gpsinterface->start-wgs\n");

    	int mode = sGpsInterface->set_position_mode(0, 0, 0, 1, 1 );//set the gps mode
   	if(mode!=0) {
       		printf("set_position_mode false\n");
       		return false;
    	}
    	//sleep(60);   
	swaptime=sGpsLocation.timestamp;
	//printf("swaptime=%lld\n",swaptime);
	tmptime=100;
	//printf("tmptime=%lld\n",tmptime);
	starttime=9999999999999;
	
	fp1=fopen("/data/gpssensordata.txt","a+");
	i=0;
	flag=0;
	//sleep(1);
	printf("GPS start!\n");
	while(1)
	{
		//printf("sGpslocation.timestamp=%lld\n",sGpsLocation.timestamp);
		usleep(100000);
		if((sGpsLocation.timestamp-swaptime) >= tmptime)
		{
			//printf("start fprintf data\n");
			/*if(flag==0)
			{
				starttime=sGpsLocation.timestamp;
				flag=1;
			}*/
			fprintf(fp1,"num=%d, time=%lld, type=%d, sensor=GPS, value=<%lf,%lf,%lf>, accuracy=%f, status=%d\n",i,sGpsLocation.timestamp,gpstype,sGpsLocation.longitude, sGpsLocation.latitude, sGpsLocation.altitude,sGpsLocation.accuracy,sGpsStatus.status);
			
			i++;
			swaptime=sGpsLocation.timestamp;
		}//end if((sGpsLocation.timestamp-swaptime) >= tmptime)
		//if(i== outputnum) break;
		//if(sGpsLocation.timestamp >= endtime) break;
		//if((sGpsLocation.timestamp-starttime) >= (hour*offset)) break;
	}


		fprintf(fp1,"stop, the gps endtime=%lld\n",sGpsLocation.timestamp);
		
	printf("GPS Messages Sent,terminating, the gps endtime=%lld\n",sGpsLocation.timestamp);
    	
//if(strcmp("continue",argv[3])==0) return 0;
	fclose(fp1);
    	sGpsInterface->stop();
    	printf("stop the gps\n");
    	printf("look the locationflags:Gpslocation->flags=%d\n",sGpsLocation.flags);
    	printf("look the status:GpsStatus->status=%d\n",sGpsStatus.status);
    	sGpsInterface->cleanup();
    	printf("cleanup the gps\n");

	if(execl("mytestgps","mytestgps",/*"-g",argv[2],"stop",*/NULL)<0)
		{
			fprintf(stderr,"execl failed: %s\n",strerror(errno));
			exit(0);
		}

    	return 0;
}

