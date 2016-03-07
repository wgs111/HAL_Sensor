#include <stdint.h>
#include <string.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <cutils/log.h>
#include <hardware/sensors.h>
#include <utils/Timers.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#define acctype 2
#define gyrtype 3
#define bartype 4
#define lacctype 5
#define port 6789
#define maxlen 1024
#define hour 7200

char const* getSensorName(int type) {
    switch(type) {
        case SENSOR_TYPE_ACCELEROMETER:
            return "Acc";//handle=0
        case SENSOR_TYPE_MAGNETIC_FIELD:
            return "Mag";
        case SENSOR_TYPE_ORIENTATION:
            return "Ori";
        case SENSOR_TYPE_GYROSCOPE:
            return "Gyr";//handle=2
        case SENSOR_TYPE_LIGHT:
            return "Lux";
        case SENSOR_TYPE_PRESSURE:
            return "Bar";//handle=3
        case SENSOR_TYPE_TEMPERATURE:
            return "Tmp";
        case SENSOR_TYPE_PROXIMITY:
            return "Prx";
        case SENSOR_TYPE_GRAVITY:
            return "Grv";
        case SENSOR_TYPE_LINEAR_ACCELERATION:
            return "Lac";//handle=13
        case SENSOR_TYPE_ROTATION_VECTOR:
            return "Rot";
        case SENSOR_TYPE_RELATIVE_HUMIDITY:
            return "Hum";
        case SENSOR_TYPE_AMBIENT_TEMPERATURE:
            return "Tam";
    }
    return "ukn";
}

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
	//mytestsensor -o/-g waittime outputnum
    int err,i;
    struct sensors_poll_device_1* device1;
    struct sensors_poll_device_t* device;
    struct sensors_module_t* module;
    FILE *fp1;
    time_t timer,timer1;

	//device = (sensors_poll_device_t *)malloc(sizeof(sensors_poll_device_t));	

    err = hw_get_module(SENSORS_HARDWARE_MODULE_ID, (hw_module_t const**)&module);//get sensor api
    if (err != 0) {
        printf("hw_get_module() failed (%s)\n", strerror(-err));
        return 0;
    }

    err = sensors_open_1(&module->common, &device1);//open the sensor device
    if (err != 0) {
        printf("sensors_open() failed (%s)\n", strerror(-err));
        return 0;
    }


    struct sensor_t const* list;
    int count = module->get_sensors_list(module, &list); //get sensor information
    printf("%d sensors found:\n", count);
    for (int i=0 ; i<count ; i++) {
        printf("%s\n"
                "\tvendor: %s\n"
                "\tversion: %d\n"
                "\thandle: %d\n"
                "\ttype: %d\n"
                "\tmaxRange: %f\n"
                "\tresolution: %f\n"
                "\tpower: %f mA\n"
		"\tminDelay: %d\n"
		"\tfifoReservedEventCount: %u\n"
		"\tfifoMaxEventCount: %u\n",
                list[i].name,
                list[i].vendor,
                list[i].version,
                list[i].handle,
                list[i].type,
                list[i].maxRange,
                list[i].resolution,
                list[i].power,
		list[i].minDelay,
		list[i].fifoReservedEventCount,
		list[i].fifoMaxEventCount);
    }

	if(count == 0)
	{
		if(execl("mytestsensor","mytestsensor",/*"-g",argv[4],argv[5],*/NULL)<0)
		{
			fprintf(stderr,"execl failed: %s\n",strerror(errno));
			exit(0);
		}
	}//if we can't get the sensor list, restart

	int hal_version = device1->common.version;

    static const size_t numEvents = 30;
    sensors_event_t buffer[numEvents];

    for (int i=0 ; i<count ; i++) {
        err = device1->activate((struct sensors_poll_device_t*)device1, list[i].handle, 0);
        if (err != 0) {
            printf("deactivate() for '%s'failed (%s)\n",
                    list[i].name, strerror(-err));
            return 0;
        }
    }//deactivate the sensor

    /*for (int i=0 ; i<count ; i++) {
        err = device->activate(device, list[i].handle, 1);
        if (err != 0) {
            printf("activate() for '%s'failed (%s)\n",
                    list[i].name, strerror(-err));
            return 0;
        }
        device->setDelay(device, list[i].handle, ms2ns(60));
    }*/
    

	for (int i=0 ; i<count ; i++)// activate the sensor we needed
	{
		switch(list[i].handle)
		{
			case 0:
			case 2:
			case 3:
			case 13:
				err = device1->activate((struct sensors_poll_device_t*)device1, list[i].handle, 1);
        			if (err != 0) {
            				printf("activate() for '%s'failed (%s)\n",
                    			list[i].name, strerror(-err));
            			return 0;
				}//end of if (err != 0)
				if (hal_version >= SENSORS_DEVICE_API_VERSION_1_1)
				{
					printf("batch road\n");
						device1->batch(device1, list[i].handle, SENSORS_BATCH_WAKE_UPON_FIFO_FULL, ms2ns(50), s2ns(2));
				}
				else
				{
					printf("delay road\n");
					device1->setDelay((struct sensors_poll_device_t*)device1, list[i].handle, ms2ns(60));
				}
				break;
		}//end of switch(list[i].handle)
	}//end of for (int i=0 ; i<count ; i++)


    int j=0;
	int n;
	timer=time(NULL);

	    fp1=fopen("/data/wgssensordata.txt","a+");//the file we store the data
    if(NULL==fp1) {
        return -1;
    }


    do {
        	int n = device1->poll((struct sensors_poll_device_t*)device1, buffer, numEvents);//poll the data
        	if (n < 0) {
            		printf("poll() failed (%s)\n", strerror(-err));
            		break;
        	}
		
		timer1=time(NULL);

        		for (i=0 ; i<n ; i++) {
            			//const sensors_event_t& data = buffer[i];

            			if (buffer[i].version != sizeof(sensors_event_t)) {
                			printf("incorrect event version (version=%d, expected=%d",
                        		buffer[i].version, sizeof(sensors_event_t));
                			break;
            			}//end if(buffer[i].version != sizeof(sensors_event_t))
		
				switch(buffer[i].type) {
				case SENSOR_TYPE_ACCELEROMETER:

					fprintf(fp1,"num=%d, time=%lld, type=%d, sensor=%s, value=<%5.1f,%5.1f,%5.1f>, localtime=%ld\n",
					j,
					buffer[i].timestamp,
					acctype,                            	
					getSensorName(buffer[i].type),
                            		buffer[i].data[0],
                            		buffer[i].data[1],
                            		buffer[i].data[2],
					timer1);

					break;

				case SENSOR_TYPE_GYROSCOPE:
					fprintf(fp1,"num=%d, time=%lld, type=%d, sensor=%s, value=<%5.1f,%5.1f,%5.1f>, localtime=%ld\n",
                            		j,
					buffer[i].timestamp,
					gyrtype,
					getSensorName(buffer[i].type),
                            		buffer[i].data[0],
                            		buffer[i].data[1],
                            		buffer[i].data[2],
					timer1);
					
					break;
				
				case SENSOR_TYPE_LINEAR_ACCELERATION:
					fprintf(fp1,"num=%d, time=%lld, type=%d, sensor=%s, value=<%5.1f,%5.1f,%5.1f>, localtime=%ld\n",
                            		j,
					buffer[i].timestamp,
					lacctype,
					getSensorName(buffer[i].type),
                            		buffer[i].data[0],
                            		buffer[i].data[1],
                            		buffer[i].data[2],
					timer1);
					
					break;
				
				case SENSOR_TYPE_PRESSURE:
					fprintf(fp1,"num=%d, time=%lld, type=%d, sensor=%s, value=%f, localtime=%ld\n",
					j,
					buffer[i].timestamp,				
					bartype,
                            		getSensorName(buffer[i].type),
                            		buffer[i].data[0],
					timer1);
					
					break;
				
				}//end switch
				
        		}//end for (int i=0 ; i<n ; i++)
			
			j++;
			
			//if((timer1-timer) >= hour && (buffer[0].timestamp/1000000000)==timer1) break;
			
		
    } while (1); // fix that

	 fprintf(fp1,"stop, the sensors endtime=%lld\n",buffer[0].timestamp);
    	
    	printf("sensor Messages Sent,terminating, the sensors endtime=%lld\n",buffer[0].timestamp);

    //exit(0);
    fclose(fp1);

    for (int i=0 ; i<count ; i++) {
        err = device1->activate((struct sensors_poll_device_t*)device1, list[i].handle, 0);
        if (err != 0) {
            printf("deactivate() for '%s'failed (%s)\n",
                    list[i].name, strerror(-err));
            return 0;
        }
    }

    err = sensors_close_1(device1);
    if (err != 0) {
        printf("sensors_close() failed (%s)\n", strerror(-err));
    }


    return 0;
}
