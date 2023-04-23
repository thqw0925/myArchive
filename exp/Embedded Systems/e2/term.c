#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <pthread.h>

#define BAUDRATE B115200
#define COM1 "/dev/ttySAC0"
#define COM2 "/dev/ttySAC3"
#define ENDMINITERM 27 /* ESC to quit miniterm */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;
volatile int fd;

void child_handler(int s) {
	printf("stop!!!\n");
	STOP=TRUE;
}

void* keyboard(void * data) {
	int c;
	for (;;){
		c=getchar();
		if( c== ENDMINITERM){
			STOP=TRUE;
			break ;
		}
	}
	return NULL;
}

void* receive(void * data) {
	int c;
	printf("read modem\n");
	while (STOP==FALSE) {
		read(fd,&c,1); /* com port */
		write(1,&c,1); /* stdout */
	}
	printf("exit from reading modem\n");
	return NULL;
}

void* send(void * data) {
	int c='0';
	printf("send data\n");
	while (STOP==FALSE) {
		c++;
		c %= 255;
		write(fd,&c,1); /* stdout */
		usleep(100000);
	}
	return NULL; /* wait for child to die or it will become a zombie */
}

int main(int argc,char** argv) {
	struct termios oldtio,newtio,oldstdtio,newstdtio;
	struct sigaction sa;
	int ok;
	pthread_t th_a, th_b, th_c;
	void * retval;

	if( argc > 1)
		fd = open(COM2, O_RDWR );
	else
		fd = open(COM1, O_RDWR ); //| O_NOCTTY |O_NONBLOCK);
	if (fd <0) {
		error(COM1);
		exit(-1);
	}
	tcgetattr(0,&oldstdtio);
	tcgetattr(fd,&oldtio);
	tcgetattr(fd,&newstdtio);
	newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VMIN]=1;
	newtio.c_cc[VTIME]=0;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio); /*set attrib*/
	
	sa.sa_handler = child_handler;
	sa.sa_flags = 0;
	sigaction(SIGCHLD,&sa,NULL);
	pthread_create(&th_a, NULL, keyboard, 0);
	pthread_create(&th_b, NULL, receive, 0);
	pthread_create(&th_c, NULL, send, 0);
	pthread_join(th_a, &retval);
	pthread_join(th_b, &retval);
	pthread_join(th_c, &retval);

	tcsetattr(fd,TCSANOW,&oldtio); /* restore old modem setings */
	tcsetattr(0,TCSANOW,&oldstdtio); /* restore old tty setings */
	close(fd);
	exit(0);
}
