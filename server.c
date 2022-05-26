#include <stdio.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>

#ifndef MAX_EVENTS
#  define MAX_EVENTS 16
#endif

#define MAX_TCP_PACKET_SIZE (65535)
#ifndef BUFFER_SIZE
#  define BUFFER_SIZE (MAX_TCP_PACKET_SIZE)
#endif

#ifndef PORT
#  define PORT (1234)
#endif

int work_flag = 1;

void stop_work(int none) {
    work_flag = 0;
}

int main(int argc, char *argv[])
{
	int sd;      // Северный сокет
	int cd;      // Клиентский сокет
	int epollfd; // Дескриптор пула сообщений
	int nfds;    // Количество полученных событий
	int ret;     // Временная переменная... на всякий случай.
	unsigned int addrlen, messages_count;		
	struct sockaddr_in addr; // Структура для задания (получения) адреса    
	struct epoll_event ev, events[MAX_EVENTS]; // Струкрутра для сохранения событий
	char buffer[BUFFER_SIZE]; // Сюда будем данные читать.
  
	signal(SIGINT, stop_work);
  
	// Создаем сокет для соединения по протоколу TCP
	if ( (-1) == (sd = socket(AF_INET, SOCK_STREAM, 0))) {
		//В случае ошибки выводим сообщение и завершаем работу
		perror("socket");
		return __LINE__;
	}
	
	// Задаем адрес для запуска сервера
	addr.sin_family =  AF_INET;  // Задаем семейство сокета
	addr.sin_port = htons(PORT); // Задаем порт сервера 
	addr.sin_addr.s_addr = INADDR_ANY; //Устанавливаем все адреса для прослушивания.
	
	// Пытаемся запустить сервер с заданными настройками.
	if ( (-1) == bind(sd, (struct sockaddr*)&addr, sizeof(addr)) ) {
	// При ошибке выводим сообщение
		perror("bind");
		// Закрываем сервер
		close(sd);
		// Завершаем работу
		return __LINE__;
	}
	
	// Начинаем прослушивание сокета. Длина очереди - 0
	if ( (-1) == listen(sd, 0) ) {
		// При ошибке выводим сообщение
		perror("listen");
		// Закрываем сервер
		close(sd);
		// Завершаем работу
		return __LINE__;
	}        
	
	// Создаем пул для отслеживания событий
	if ( (-1) == (epollfd = epoll_create1(EPOLL_CLOEXEC)) ) {
		// При ошибке выводим сообщение
		perror("epoll_create1");
		// Закрываем сервер
		close(sd);
		// Завершаем работу
		return __LINE__;
	}
	
	// Добавляем сокет, события которого планируем отслеживать
	ev.events = EPOLLIN; // Сокет доступен для чтения
	ev.data.fd = sd;     // Данные, которые хотим получать
	// при возникновении события.
	// В данном примере это сам сокет.
  
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sd, &ev) == -1) {
		// При ошибке выводим сообщение
		perror("epoll_ctl: sd");
		// Закрываем сервер
		close(sd);
		// Завершаем работу
		return __LINE__;
	}
	
		messages_count = 0;
    // Начинаем слушать сокеты
    while( work_flag )	{
			// Ожидаем события 
			nfds = epoll_wait(
												epollfd, 
												events, // Куда сохранять
												MAX_EVENTS, // Максимальное количество допустимых событий
												2 * 1000// Ожидать не более 5 сек.
												);
			
			
			if (nfds == -1) { // Что-то случилось..
				//Получен сигнал
				if ( errno == EINTR ) {
					//Протолжаем работу
					continue;
				}
				// При ошибке выводим сообщение
				perror("epoll_wait");
				close(epollfd);
				// Закрываем сервер
				close(sd);
				// Завершаем работу
				return __LINE__;
			}
			
			// Если сработал таймер, то сообщаем об этом и продолжаем
			if ( nfds == 0 ) {
				//printf("Time is over\n");
				continue;
			}
			
			// Если что-то случилось, то начинаем обработку)))
			for (int n = 0; n < nfds; n++) {
				// Если данных в данных сохранен серверный сокет
				if (events[n].data.fd == sd) {
					// Принимаем новое соединение
					cd = accept(sd, (struct sockaddr *) &addr, &addrlen);
					if (cd == -1) {
						perror("accept");
						close(epollfd);
						close(sd);
						//continue;
						return __LINE__;
					}
					// Получаем текущие настройки для сокета		
					int flags = fcntl(cd, F_GETFL);
					
					// Делаем сокет неблокируемым
					fcntl(cd,flags|O_NONBLOCK);
					
					//Добавляем сокет на слежку
					ev.events = EPOLLIN | EPOLLET;
					ev.data.fd = cd;
					if (epoll_ctl(epollfd, EPOLL_CTL_ADD, cd, &ev) == -1) {
						perror("epoll_ctl: conn_sock");
						exit(EXIT_FAILURE);
					}
					// Сообщаем, что соединение произошло
					printf("Connect with %s\n", inet_ntoa(addr.sin_addr));
				} else {		
					ret = read(events[n].data.fd, buffer, BUFFER_SIZE);
					if ( ret == 0 ) {
						// Если при первом чтении 0,
						// то это означет закрытие клиента
						epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd, NULL);
						close(events[n].data.fd);
						printf("Connection is close\n");
						continue;
					}			
					messages_count++;			
					write(1, buffer, ret);
				}
			}
    }
    close(epollfd);
    close(sd);
    printf("The end! (recv %d messages)\n", messages_count);
    return 0;
}
